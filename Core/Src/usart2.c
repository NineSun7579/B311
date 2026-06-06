#include "usart2.h"
#include "AD9910.h"
#include <string.h>
#include <stdio.h>

extern UART_HandleTypeDef huart2;

uint8_t  USART2_RX_BUF[USART2_REC_LEN];
uint16_t USART2_RX_STA = 0;
uint32_t freq;
uint32_t vpp;

float sweep_vpp_data[SWEEP_POINT_COUNT];
volatile uint8_t sweep_start_flag = 0;
volatile uint8_t sweep_running = 0;

static uint8_t  rx_buf[16];
static uint8_t  rx_cnt = 0;
static uint8_t  frame_started = 0;
static uint8_t  got_tail1 = 0;
static uint32_t last_rx_tick = 0;
volatile uint8_t rx_frame_ready = 0;
volatile uint8_t g_init_done = 0;

static uint8_t rx_byte;

void USART2_StartReceive(void)
{
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
}

void usart2_send_byte(uint8_t data)
{
    HAL_UART_Transmit(&huart2, &data, 1, HAL_MAX_DELAY);
}

void usart2_send_string(char *str)
{
    while (*str)
    {
        usart2_send_byte(*str++);
    }
}

void usart2_send_buf(uint8_t *buf, uint16_t len)
{
    HAL_UART_Transmit(&huart2, buf, len, HAL_MAX_DELAY);
}

uint8_t usart2_recv_byte(void)
{
    uint8_t data;
    HAL_UART_Receive(&huart2, &data, 1, HAL_MAX_DELAY);
    return data;
}

uint32_t usart2_recv_value(void)
{
    uint8_t buf[4];
    buf[0] = usart2_recv_byte();
    buf[1] = usart2_recv_byte();
    buf[2] = usart2_recv_byte();
    buf[3] = usart2_recv_byte();
    return (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
}

static void send_val_to_screen(char *label, uint32_t value)
{
    char buf[32];
    sprintf(buf, "%s.val=%lu", label, value);
    usart2_send_string(buf);
    usart2_send_byte(0xff);
    usart2_send_byte(0xff);
    usart2_send_byte(0xff);
}

void USART2_Reset(void)
{
    rx_cnt = 0;
    frame_started = 0;
    got_tail1 = 0;
    rx_frame_ready = 0;
}

void USART2_ProcessPacket(void)
{
    if (!g_init_done)
        return;

    if (rx_frame_ready)
    {
        rx_frame_ready = 0;

        if (rx_cnt == 2 && rx_buf[0] == 0xAA && rx_buf[1] == 0xCC)
        {
            sweep_start_flag = 1;
        }
        else if (rx_cnt == 4)
        {
            freq = (uint32_t)rx_buf[0] | ((uint32_t)rx_buf[1] << 8) | ((uint32_t)rx_buf[2] << 16) | ((uint32_t)rx_buf[3] << 24);
            AD9910_FreWrite(freq);
            send_val_to_screen("n0", freq);
        }
        else if (rx_cnt == 9 && rx_buf[4] == 0xA2)
        {
            freq = (uint32_t)rx_buf[0] | ((uint32_t)rx_buf[1] << 8) | ((uint32_t)rx_buf[2] << 16) | ((uint32_t)rx_buf[3] << 24);
            vpp  = (uint32_t)rx_buf[5] | ((uint32_t)rx_buf[6] << 8) | ((uint32_t)rx_buf[7] << 16) | ((uint32_t)rx_buf[8] << 24);
            AD9910_FreWrite(freq);
            AD9910_AmpWrite(vpp);
            send_val_to_screen("n0", freq);
            send_val_to_screen("n1", vpp);
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        if (!frame_started)
        {
            if (rx_cnt == 0 && rx_byte == FRAME_HEAD1)
            {
                rx_buf[rx_cnt++] = rx_byte;
            }
            else if (rx_cnt == 1 && rx_byte == FRAME_HEAD2)
            {
                rx_cnt = 0;
                frame_started = 1;
                got_tail1 = 0;
            }
            else
            {
                rx_cnt = 0;
            }
        }
        else
        {
            if (got_tail1)
            {
                if (rx_byte == FRAME_TAIL2)
                {
                    rx_frame_ready = 1;
                    frame_started = 0;
                }
                got_tail1 = 0;
            }
            else if (rx_byte == FRAME_TAIL1)
            {
                got_tail1 = 1;
            }
            else if (rx_cnt < 14)
            {
                rx_buf[rx_cnt++] = rx_byte;
            }
            else
            {
                rx_cnt = 0;
                frame_started = 0;
            }
        }

        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}
