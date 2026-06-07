#include "adc.h"

#define ADC_BUF_SIZE 2500
static uint16_t adc_buf[ADC_BUF_SIZE];

/************************************************************
** 函数名称: void ADC1_Init(void)
** 函数功能: 初始化ADC1，配置PC3为模拟输入，单次转换模式
**************************************************************/
void ADC1_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    ADC_InitTypeDef   ADC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1, ADC_SampleTime_15Cycles);
    ADC_Cmd(ADC1, ENABLE);
}

/************************************************************
** 函数名称: uint16_t ADC1_GetMax(void)
** 函数功能: 采集2500个点，返回最大ADC值(0~4096)
**************************************************************/
uint16_t ADC1_GetMax(void)
{
    uint16_t i;
    uint16_t adc_max = 0;

    // 采集2500个点存入缓冲区
    for (i = 0; i < ADC_BUF_SIZE; i++)
    {
        ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
        ADC_SoftwareStartConv(ADC1);
        while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
        adc_buf[i] = ADC_GetConversionValue(ADC1);
    }

    // 遍历缓冲区找最大值
    for (i = 0; i < ADC_BUF_SIZE; i++)
    {
        if (adc_buf[i] > adc_max) adc_max = adc_buf[i];
    }

    return adc_max;
}

/************************************************************
** 函数名称: uint16_t ADC1_ReadSingle(void)
** 函数功能: 单次读取PC3 ADC值
** 返回值:   12位值(0~4095)
**************************************************************/
uint16_t ADC1_ReadSingle(void)
{
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
    ADC_SoftwareStartConv(ADC1);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    return ADC_GetConversionValue(ADC1);  // 返回原始值
}
