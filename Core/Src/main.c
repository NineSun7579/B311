/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "AD9910.h"
#include "usart2.h"
#include "adc.h"
#include <stdio.h>
#include "arm_math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void Analyze_FilterType(void)
{
    char buf[32];

    float first_avg = 0, mid_avg = 0, last_avg = 0;
    float max_vpp = 0;
    uint16_t third = SWEEP_POINT_COUNT / 3;

    for (uint16_t i = 0; i < third; i++)
        first_avg += sweep_vpp_data[i];
    first_avg /= third;

    for (uint16_t i = third; i < third * 2; i++)
        mid_avg += sweep_vpp_data[i];
    mid_avg /= third;

    for (uint16_t i = third * 2; i < SWEEP_POINT_COUNT; i++)
        last_avg += sweep_vpp_data[i];
    last_avg /= third;

    for (uint16_t i = 0; i < SWEEP_POINT_COUNT; i++)
        if (sweep_vpp_data[i] > max_vpp) max_vpp = sweep_vpp_data[i];

    float edge_avg = (first_avg + last_avg) / 2.0f + 0.0001f;
    float ratio = first_avg / (last_avg + 0.0001f);
    float center_ratio = mid_avg / (edge_avg + 0.0001f);

    float lp_score = 0, hp_score = 0, bp_score = 0, bs_score = 0;

    if (ratio >= 1.0f)
        lp_score += (ratio - 1.0f) * 2.0f;
    else
        hp_score += (1.0f - ratio) * 2.0f;

    if (center_ratio >= 1.0f)
        bp_score += (center_ratio - 1.0f) * 1.5f;
    else
        bs_score += (1.0f - center_ratio) * 1.5f;

    float first_ratio = first_avg / (max_vpp + 0.0001f);
    float last_ratio = last_avg / (max_vpp + 0.0001f);
    float mid_ratio = mid_avg / (max_vpp + 0.0001f);

    lp_score += first_ratio * 0.5f;
    hp_score += last_ratio * 0.5f;
    bp_score += mid_ratio * 0.5f;
    bs_score += (1.0f - mid_ratio) * 0.5f;

    sprintf(buf, "t0.bco=%u", COLOR_DEFAULT);
    usart2_send_string(buf);
    usart2_send_byte(0xff);
    usart2_send_byte(0xff);
    usart2_send_byte(0xff);

    sprintf(buf, "t2.bco=%u", COLOR_DEFAULT);
    usart2_send_string(buf);
    usart2_send_byte(0xff);
    usart2_send_byte(0xff);
    usart2_send_byte(0xff);

    sprintf(buf, "t3.bco=%u", COLOR_DEFAULT);
    usart2_send_string(buf);
    usart2_send_byte(0xff);
    usart2_send_byte(0xff);
    usart2_send_byte(0xff);

    sprintf(buf, "t4.bco=%u", COLOR_DEFAULT);
    usart2_send_string(buf);
    usart2_send_byte(0xff);
    usart2_send_byte(0xff);
    usart2_send_byte(0xff);

    float max_score = lp_score;
    uint8_t filter_type = 1;

    if (hp_score > max_score) { max_score = hp_score; filter_type = 2; }
    if (bp_score > max_score) { max_score = bp_score; filter_type = 3; }
    if (bs_score > max_score) { max_score = bs_score; filter_type = 4; }

    switch (filter_type)
    {
        case 1:
            sprintf(buf, "t4.bco=%u", COLOR_YELLOW);
            usart2_send_string(buf);
            usart2_send_byte(0xff);
            usart2_send_byte(0xff);
            usart2_send_byte(0xff);
            break;
        case 2:
            sprintf(buf, "t3.bco=%u", COLOR_YELLOW);
            usart2_send_string(buf);
            usart2_send_byte(0xff);
            usart2_send_byte(0xff);
            usart2_send_byte(0xff);
            break;
        case 3:
            sprintf(buf, "t0.bco=%u", COLOR_YELLOW);
            usart2_send_string(buf);
            usart2_send_byte(0xff);
            usart2_send_byte(0xff);
            usart2_send_byte(0xff);
            break;
        case 4:
            sprintf(buf, "t2.bco=%u", COLOR_YELLOW);
            usart2_send_string(buf);
            usart2_send_byte(0xff);
            usart2_send_byte(0xff);
            usart2_send_byte(0xff);
            break;
    }
}

static void Sweep_Frequency(void)
{
    sweep_running = 1;
    sweep_start_flag = 0;

    uint32_t freq_point = SWEEP_FREQ_MIN;
    uint16_t point_index = 0;
    char buf[32];

    while (point_index < SWEEP_POINT_COUNT)
    {
        AD9910_FreWrite(freq_point);
        HAL_Delay(20);

        float vpp_sum = 0;
        for (uint8_t i = 0; i < 3; i++)
        {
            vpp_sum += ADC1_GetVpp_Voltage();
            HAL_Delay(10);
        }
        float vpp_avg = vpp_sum / 3.0f;

        sweep_vpp_data[point_index] = vpp_avg;

        uint32_t vpp_mv = (uint32_t)(vpp_avg * 1000.0f);
        sprintf(buf, "n5.val=%lu", vpp_mv);
        usart2_send_string(buf);
        usart2_send_byte(0xff);
        usart2_send_byte(0xff);
        usart2_send_byte(0xff);

        sprintf(buf, "n0.val=%lu", freq_point);
        usart2_send_string(buf);
        usart2_send_byte(0xff);
        usart2_send_byte(0xff);
        usart2_send_byte(0xff);

        point_index++;
        freq_point += SWEEP_FREQ_STEP;

        HAL_Delay(400);
    }

    Analyze_FilterType();
    sweep_running = 0;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
/* USER CODE BEGIN 2 */
  Init_AD9910();
  AD9910_FreWrite(100);
  AD9910_AmpWrite(16383);

  usart2_send_string("n0.val=100");
  usart2_send_byte(0xff);
  usart2_send_byte(0xff);
  usart2_send_byte(0xff);
  HAL_Delay(100);

  usart2_send_string("n1.val=1000");
  usart2_send_byte(0xff);
  usart2_send_byte(0xff);
  usart2_send_byte(0xff);
  HAL_Delay(100);

  USART2_Reset();
  g_init_done = 1;
  USART2_StartReceive();
/* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    USART2_ProcessPacket();

    if (sweep_start_flag && !sweep_running)
    {
        Sweep_Frequency();
    }

    static uint32_t last_measure_tick = 0;
    if (HAL_GetTick() - last_measure_tick >= 100)
    {
      last_measure_tick = HAL_GetTick();

      static float vpp_sum = 0;
      static uint8_t measure_count = 0;
      
      vpp_sum += ADC1_GetVpp_Voltage();
      measure_count++;
      
      if (measure_count >= 5)
      {
        float vpp_avg = vpp_sum / 5;
        uint32_t vpp_mv = (uint32_t)(vpp_avg * 1000.0f);
        
        char buf[32];
        sprintf(buf, "n5.val=%lu", vpp_mv);
        usart2_send_string(buf);
        usart2_send_byte(0xff);
        usart2_send_byte(0xff);
        usart2_send_byte(0xff);
        
        vpp_sum = 0;
        measure_count = 0;
      }
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSI);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_16B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.Oversampling.Ratio = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED, ADC_CALIB_OFFSET);
  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
