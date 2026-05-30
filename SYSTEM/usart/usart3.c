#include "usart3.h"
#include "AD9910.h"
#include "stm32f4xx.h"
#include <string.h>
#include <stdio.h>

extern volatile uint32_t g_tick_ms;

u8  UART5_RX_BUF[UART5_REC_LEN];
u16 UART5_RX_STA = 0;
u32 freq;
u32 vpp;

static u8  rx_buf[8];
static u8  rx_cnt = 0;
static u32 last_rx_tick = 0;
volatile u8 rx_8bytes_ready = 0;
volatile u8 g_init_done = 0;

void uart5_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_UART5);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_UART5);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART5, &USART_InitStructure);

#if EN_UART5_RX
	{
		NVIC_InitTypeDef NVIC_InitStructure;
		USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);

		NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
#endif

	USART_Cmd(UART5, ENABLE);
}

void uart5_send_byte(u8 data)
{
	while ((UART5->SR & 0x80) == 0);
	UART5->DR = data;
}

void uart5_send_string(char *str)
{
	while (*str)
	{
		uart5_send_byte(*str++);
	}
}

void uart5_send_buf(u8 *buf, u16 len)
{
	u16 i;
	for (i = 0; i < len; i++)
	{
		uart5_send_byte(buf[i]);
	}
}

u8 uart5_recv_byte(void)
{
	while ((UART5->SR & 0x20) == 0);
	return (u8)(UART5->DR & 0xFF);
}

u32 uart5_recv_value(void)
{
	u8 buf[4];
	buf[0] = uart5_recv_byte();
	buf[1] = uart5_recv_byte();
	buf[2] = uart5_recv_byte();
	buf[3] = uart5_recv_byte();
	return (u32)buf[0] | ((u32)buf[1] << 8) | ((u32)buf[2] << 16) | ((u32)buf[3] << 24);
}

static void send_val_to_screen(char *label, u32 value)
{
	char buf[32];
	sprintf(buf, "%s.val=%lu", label, value);
	uart5_send_string(buf);
	uart5_send_byte(0xff);
	uart5_send_byte(0xff);
	uart5_send_byte(0xff);
}

void UART5_Reset(void)
{
	rx_cnt = 0;
	rx_8bytes_ready = 0;
}

void UART5_ProcessPacket(void)
{
	if (!g_init_done)
		return;

	if (rx_8bytes_ready)
	{
		rx_8bytes_ready = 0;

		freq = (u32)rx_buf[0] | ((u32)rx_buf[1] << 8) | ((u32)rx_buf[2] << 16) | ((u32)rx_buf[3] << 24);
		vpp  = (u32)rx_buf[4] | ((u32)rx_buf[5] << 8) | ((u32)rx_buf[6] << 16) | ((u32)rx_buf[7] << 24);

		AD9910_FreWrite(freq);
		AD9910_AmpWrite(vpp);

		send_val_to_screen("n0", freq);
		send_val_to_screen("n1", vpp);
	}
	else if (rx_cnt == 4)
	{
		if ((g_tick_ms - last_rx_tick) >= 5)
		{
			rx_cnt = 0;

			freq = (u32)rx_buf[0] | ((u32)rx_buf[1] << 8) | ((u32)rx_buf[2] << 16) | ((u32)rx_buf[3] << 24);

			AD9910_FreWrite(freq);

			send_val_to_screen("n0", freq);
		}
	}
}

#if EN_UART5_RX
void UART5_IRQHandler(void)
{
	u32 sr = UART5->SR;

	if (sr & USART_SR_RXNE)
	{
		if (rx_cnt < 8)
		{
			rx_buf[rx_cnt] = (u8)UART5->DR;
			rx_cnt++;
			last_rx_tick = g_tick_ms;

			if (rx_cnt == 8)
			{
				rx_8bytes_ready = 1;
				rx_cnt = 0;
			}
		}
		else
		{
			(void)UART5->DR;
		}
	}

	if (sr & USART_SR_ORE)
	{
		(void)UART5->DR;
		rx_cnt = 0;
		rx_8bytes_ready = 0;
	}
}
#endif
