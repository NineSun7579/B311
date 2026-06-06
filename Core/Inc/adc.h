#ifndef __ADC_H
#define __ADC_H

#include "stm32h7xx_hal.h"
#include <stdint.h>
#include "arm_math.h"

#define ADC_BUF_SIZE   2500
#define FFT_LENGTH     1024

extern ADC_HandleTypeDef hadc1;

uint16_t ADC1_GetVpp(void);
float ADC1_GetVpp_FFT(void);
float ADC1_GetDCVoltage(void);

#endif
