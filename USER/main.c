#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
#include "usart.h"
#include "delay.h"
#include "AD9910.h"
#include "usart3.h"

//ALIENTEK ̽����STM32F407������ ʵ��0
//STM32F4����ģ��-�⺯���汾
//����֧�֣�www.openedv.com
//�Ա����̣�http://eboard.taobao.com
//���������	������ӿƼ����޹�˾  
//���ߣ�����ԭ�� @ALIENTEK
u16 a=0;
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    uart5_init(115200);
    delay_init(72);
    delay_ms(500);

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

    Init_AD9910();          // AD9910���ƽż��Ĵ�����ʼ��
    AD9910_FreWrite(100);  // д���Ƶ��100MHz
    AD9910_AmpWrite(16383); // д���������󡣷�Χ��0~16383

    // 发送初始化指令给串口屏，更新n0(频率)和n1(VPP)到初始值
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
		UART5_ProcessPacket();
    }
}

/*
�ֲ��н��⵽����15��ʱ���main.cԴ�����£�
#include "stm32f4xx.h"

//ALIENTEK ̽����STM32F407������ ʵ��0
//STM32F4����ģ��-�⺯���汾
//����֧�֣�www.openedv.com
//�Ա����̣�http://eboard.taobao.com
//�������������ӿƼ����޹�˾  
//���ߣ�����ԭ�� @ALIENTEK
  
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


