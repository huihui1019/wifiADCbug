/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "key.h"
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

/* USER CODE BEGIN PV */
u32 tim2utick = 0;
u32 tim3utick = 0;
u32 level = 0;
u32 distance_time = 0;
u32 rainfall = 0;
u32 levelmax = 1000;
u32 levelmin = 0;
u8 float_str[7] = {0};

u32 ADC_Value[3] = {0};
u32 ADC_Value_all = 0;

u32 key[4] = {0};

u8 wifistr[10] = {0};
u8 wifidata[10] = 0;
u8 wifi = 10;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    if(htim->Instance == TIM2){//1ms tick interrupt handler
    KEY_LOOP(key)
    if(((key[0] == 10)) && (levelmax < 2000)){
      levelmax += 10;
      HAL_UART_Transmit_DMA(&huart2,(u8 *)&levelmax,2);
    }
    else if((key[1] == 10) && (levelmax > levelmin)){
      levelmax -= 10;
      HAL_UART_Transmit_DMA(&huart2,(u8 *)&levelmax,2);
    }
    else if((key[2] == 10) && (levelmin < levelmax)){
      levelmin += 10;
      HAL_UART_Transmit_DMA(&huart2,(u8 *)&levelmin,2);
    }
    else if((key[3] == 10) && (levelmin > 0)){
      levelmin -= 10;
      HAL_UART_Transmit_DMA(&huart2,(u8 *)&levelmin,2);
    }
    if (level < levelmin || level > levelmax){
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    }
    else HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

  }
  else if(htim->Instance == TIM3){//10us tick interrupt handler
    ++tim3utick;
    tim3utick>3502 ? GPIO_OFF(GPIOA,GPIO_PIN_9) : (tim3utick<3500?GPIO_OFF(GPIOA,GPIO_PIN_9):GPIO_ON(GPIOA,GPIO_PIN_9));
    if (GPIO_STATUS(GPIOA,GPIO_PIN_8)){
      distance_time++;
      tim3utick = 0;
    }
    else if((!GPIO_STATUS(GPIOA,GPIO_PIN_8))){
      if(distance_time>10 && distance_time<6000){
        level = distance_time*3400/2/10000;
        // HAL_UART_Transmit_DMA(&huart2,(u8 *)&level,2);
      }
    distance_time = 0;
    }
  }
  else if(htim->Instance == TIM3){//1ms tick interrupt handler
  }
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
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim3);
  HAL_TIM_Base_Start_IT(&htim4);
  HAL_ADCEx_Calibration_Start(&hadc1);
  OLED_Init();
  OLED_ColorTurn(0);
  OLED_DisplayTurn(0);
  OLED_Refresh();
  WiFi_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    // HAL_UART_Transmit_DMA(&huart2,(u8 *)&key[1],2);
      u8 string_level[20]="WATER_LEVEL(mm):";
      // snprintf(float_str,8, "%f", level);
      snprintf(float_str,8, "%04d", level);
      strcat(string_level, float_str);
      OLED_ShowString(0,0,string_level,12);
      
      u8 string_rain[20]="RAINFALL(%):";
      for(int i=0; i<3; i++){
      HAL_ADC_Start(&hadc1);
      HAL_ADC_PollForConversion(&hadc1, 50);
        if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC)){
          ADC_Value[i] = HAL_ADC_GetValue(&hadc1)*100/4096;  
          ADC_Value_all += ADC_Value[i];
        }
      }
      rainfall = ADC_Value_all/3;
      ADC_Value_all = 0;
      snprintf(float_str,8, "%03d", 100-rainfall);
      strcat(string_rain, float_str);
      OLED_ShowString(0,16,string_rain,12);  
      
      u8 string_rainmax[20]="MAX(mm):";
      snprintf(float_str,8, "%04d", levelmax);
      strcat(string_rainmax, float_str);
      OLED_ShowString(0,32,string_rainmax,12);
      u8 string_rainmin[20]="MIN(mm):";
      snprintf(float_str,8, "%04d", levelmin);
      strcat(string_rainmin, float_str);
      OLED_ShowString(0,48,string_rainmin,12);
      OLED_Refresh();

    switch (wifi){
      case 0:
      GPIO_OFF(GPIOB, GPIO_PIN_13);
      GPIO_OFF(GPIOB, GPIO_PIN_14);
      break;
      case 1:
      GPIO_ON(GPIOB, GPIO_PIN_13);
      GPIO_OFF(GPIOB, GPIO_PIN_14);
      break ;
      case 2:
      GPIO_ON(GPIOB, GPIO_PIN_14);
      GPIO_OFF(GPIOB, GPIO_PIN_13);
      break ;
      case 3:
      snprintf(wifistr,8, "%04d", level);
      HAL_UART_Transmit_DMA(&huart3,(u8 *)&wifistr,4);
      wifi = 10;
      break;
      case 4:
      snprintf(wifistr,8, "%03d", 100-rainfall);
      HAL_UART_Transmit_DMA(&huart3,(u8 *)&wifistr,10);
      wifi = 10;
      break;
      case 5:
      snprintf(wifistr,8, "%04d", levelmax);
      HAL_UART_Transmit_DMA(&huart3,(u8 *)&wifistr,4);
      wifi = 10;
      break;
      case 6:
      snprintf(wifistr,8, "%04d", levelmin);
      HAL_UART_Transmit_DMA(&huart3,(u8 *)&wifistr,4);
      wifi = 10;
      break;
      default:break;
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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

#ifdef  USE_FULL_ASSERT
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
