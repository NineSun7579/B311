#include "adc.h"
#include <math.h>

#define ADC_TIMEOUT_MS 10
#define ADC_VREF       3.3f
#define ADC_RESOLUTION 65535.0f
#define VPP_CALIB      2.2f

static uint16_t adc_buf[FFT_LENGTH];
static float fft_inputbuf[FFT_LENGTH * 2];
static float fft_outputbuf[FFT_LENGTH];
static arm_cfft_radix4_instance_f32 scfft;

uint16_t ADC1_GetVpp(void)
{
    uint16_t i;
    uint16_t adc_max = 0;
    uint16_t adc_min = 65535;

    HAL_ADC_Start(&hadc1);

    for (i = 0; i < ADC_BUF_SIZE; i++)
    {
        if (HAL_ADC_PollForConversion(&hadc1, ADC_TIMEOUT_MS) != HAL_OK)
        {
            HAL_ADC_Stop(&hadc1);
            return 0;
        }
        adc_buf[i] = (uint16_t)HAL_ADC_GetValue(&hadc1);
    }

    HAL_ADC_Stop(&hadc1);

    for (i = 0; i < ADC_BUF_SIZE; i++)
    {
        if (adc_buf[i] > adc_max) adc_max = adc_buf[i];
        if (adc_buf[i] < adc_min) adc_min = adc_buf[i];
    }

    return adc_max - adc_min;
}

float ADC1_GetVpp_Voltage(void)
{
    uint16_t i;
    uint16_t adc_max = 0;
    uint16_t adc_min = 65535;
    uint32_t adc_sum = 0;

    HAL_ADC_Start(&hadc1);

    for (i = 0; i < ADC_BUF_SIZE; i++)
    {
        if (HAL_ADC_PollForConversion(&hadc1, ADC_TIMEOUT_MS) != HAL_OK)
        {
            HAL_ADC_Stop(&hadc1);
            return 0.0f;
        }
        adc_buf[i] = (uint16_t)HAL_ADC_GetValue(&hadc1);
    }

    HAL_ADC_Stop(&hadc1);

    for (i = 0; i < ADC_BUF_SIZE; i++)
    {
        if (adc_buf[i] > adc_max) adc_max = adc_buf[i];
        if (adc_buf[i] < adc_min) adc_min = adc_buf[i];
        adc_sum += adc_buf[i];
    }

    float vpp = (float)(adc_max - adc_min) * ADC_VREF / ADC_RESOLUTION;

    return vpp;
}

float ADC1_GetVpp_FFT(void)
{
    uint16_t i;
    uint32_t max_index;
    float max_mag;
    float vpp;

    arm_cfft_radix4_init_f32(&scfft, FFT_LENGTH, 0, 1);

    HAL_ADC_Start(&hadc1);

    for (i = 0; i < FFT_LENGTH; i++)
    {
        if (HAL_ADC_PollForConversion(&hadc1, ADC_TIMEOUT_MS) != HAL_OK)
        {
            HAL_ADC_Stop(&hadc1);
            return 0.0f;
        }
        adc_buf[i] = (uint16_t)HAL_ADC_GetValue(&hadc1);
    }

    HAL_ADC_Stop(&hadc1);

    for (i = 0; i < FFT_LENGTH; i++)
    {
        fft_inputbuf[2 * i] = (float)adc_buf[i] * ADC_VREF / ADC_RESOLUTION;
        fft_inputbuf[2 * i + 1] = 0.0f;
    }

    arm_cfft_radix4_f32(&scfft, fft_inputbuf);
    arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH);

    fft_outputbuf[0] = 0.0f;
    arm_max_f32(fft_outputbuf + 1, FFT_LENGTH / 2 - 1, &max_mag, &max_index);

    vpp = 2.0f * max_mag / FFT_LENGTH * VPP_CALIB;

    return vpp;
}

float ADC1_GetDCVoltage(void)
{
    uint16_t i;
    uint32_t adc_sum = 0;
    float voltage;

    HAL_ADC_Start(&hadc1);

    for (i = 0; i < ADC_BUF_SIZE; i++)
    {
        if (HAL_ADC_PollForConversion(&hadc1, ADC_TIMEOUT_MS) != HAL_OK)
        {
            HAL_ADC_Stop(&hadc1);
            return 0.0f;
        }
        adc_sum += (uint16_t)HAL_ADC_GetValue(&hadc1);
    }

    HAL_ADC_Stop(&hadc1);

    float adc_avg = (float)adc_sum / ADC_BUF_SIZE;
    voltage = adc_avg * ADC_VREF / ADC_RESOLUTION;

    return voltage;
}

float ADC1_GetFrequency(void)
{
    uint16_t i;
    uint16_t sample_count = ADC_BUF_SIZE;
    uint16_t adc_max = 0, adc_min = 65535;

    HAL_ADC_Start(&hadc1);

    for (i = 0; i < sample_count; i++)
    {
        if (HAL_ADC_PollForConversion(&hadc1, ADC_TIMEOUT_MS) != HAL_OK)
        {
            HAL_ADC_Stop(&hadc1);
            return 0.0f;
        }
        adc_buf[i] = (uint16_t)HAL_ADC_GetValue(&hadc1);
    }

    HAL_ADC_Stop(&hadc1);

    for (i = 0; i < sample_count; i++)
    {
        if (adc_buf[i] > adc_max) adc_max = adc_buf[i];
        if (adc_buf[i] < adc_min) adc_min = adc_buf[i];
    }

    uint16_t amplitude = adc_max - adc_min;
    if (amplitude < 1000) return 0.0f;

    float threshold = (float)(adc_max + adc_min) / 2.0f;

    uint32_t rising_cross_count = 0;
    uint8_t above = (adc_buf[0] >= threshold) ? 1 : 0;

    for (i = 1; i < sample_count; i++)
    {
        uint8_t now_above = (adc_buf[i] >= threshold) ? 1 : 0;
        if (above == 0 && now_above == 1)
        {
            rising_cross_count++;
        }
        above = now_above;
    }

    if (rising_cross_count < 2) return 0.0f;

    float frequency = (float)rising_cross_count * 111.1f;

    if (frequency > 500000.0f || frequency < 10.0f) return 0.0f;

    return frequency;
}
