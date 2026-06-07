#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
#include "usart.h"
#include "delay.h"
#include "AD9910.h"
#include "usart3.h"
#include "adc.h"
#include <stdio.h>

//ALIENTEK ???????STM32F407????????? ????0
//STM32F4??????????-?????????
//???????????www.openedv.com
//???????????http://eboard.taobao.com
//??????????????????????????????????? 
//????????????????? @ALIENTEK
u16 a=0;
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    uart5_init(115200);
    delay_init(72);
    delay_ms(500);

    ADC1_Init();    // 初始化ADC1，PC3模拟输入，DMA连续采集

    {
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
        NVIC_InitTypeDef NVIC_InitStructure;
        
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
        
        TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
        TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
        TIM_TimeBaseStructure.TIM_ClockDivision = 0;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
        
        TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
        TIM_Cmd(TIM6, ENABLE);
        
        NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }

    Init_AD9910();          // AD9910??????????????????????
    AD9910_FreWrite(100);  // ??????????00MHz
    AD9910_AmpWrite(16383); // ??????????????????????0~16383

    // 初始化串口屏显示值
    uart5_send_string("n0.val=100");
    uart5_send_byte(0xff);
    uart5_send_byte(0xff);
    uart5_send_byte(0xff);
    delay_ms(100);

    uart5_send_string("n1.val=1000");
    uart5_send_byte(0xff);
    uart5_send_byte(0xff);
    uart5_send_byte(0xff);
    delay_ms(100);

    UART5_Reset();
    g_init_done = 1;

    while (1)
    {
		char buf[32];
		uint16_t adc_max;
		uint16_t display_val;

		UART5_ProcessPacket();

		// 采集2500个点，找最大值
		adc_max = ADC1_ GetMax();

		// 转换为电压×10
		display_val = (uint16_t)((uint32_t)adc_max * 33 / 4096);

		// 发送到串口屏n5显示
		sprintf(buf, "n5.val=%u", display_val);
		uart5_send_string(buf);
		uart5_send_byte(0xff);
		uart5_send_byte(0xff);
		uart5_send_byte(0xff);
    }
}

/*
?????????????????15?????????ain.c???????????
#include "stm32f4xx.h"

//ALIENTEK ???????STM32F407????????? ????0
//STM32F4??????????-?????????
//???????????www.openedv.com
//???????????http://eboard.taobao.com
//???????????????????????????????  
//????????????????? @ALIENTEK
  
void Delay(__IO uint32_t nCount);

void Delay(__IO uint32_t nCount)
{
  while(nCount--){}
}

int main(void)
{

  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOF, &GPIO_InitStructure);

  while(1){
		GPIO_SetBits(GPIOF,GPIO_Pin_9|GPIO_Pin_10);
		Delay(0x7FFFFF);
		GPIO_ResetBits(GPIOF,GPIO_Pin_9|GPIO_Pin_10);
		Delay(0x7FFFFF);
	
	}
}
*/


