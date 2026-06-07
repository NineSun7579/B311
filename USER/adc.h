#ifndef __ADC_H
#define __ADC_H

#include "stm32f4xx.h"

void     ADC1_Init(void);    // ADC1初始化
uint16_t ADC1_GetMax(void);  // 采集2500点，返回最大值(0~4096)

#endif
