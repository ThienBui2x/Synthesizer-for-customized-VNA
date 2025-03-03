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
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ILI9225.h"
#include "adf4351_hmi.h"
#include "adf4351.h"
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
extern volatile hmi_status hmi;
uint16_t volatile v_mag[1000] = {0};
uint16_t volatile v_mag_ave = 1250;
uint16_t volatile v_mag_idx = 0;

uint8_t volatile min_val = 70;	// top of y-axis
uint8_t volatile max_val = 168; // bottom of y-axis
uint8_t volatile mean_val = 0;
uint16_t volatile v_mag_max_val = 0;
uint16_t volatile v_mag_min_val = 0;

uint8_t volatile y_curr_val = 0;
uint8_t volatile x_curr_val = 0;
uint8_t volatile y_prev_val = 0;
uint8_t volatile x_prev_val = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(GPIO_Pin);

  if(GPIO_Pin == BACK_BUT_Pin)
  {
//	  HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin);
	  hmi.button = BACK;

	  if(hmi.curr_state == Plot_VMAG)
	  {
		  /* Make sure the plotting is stopped before exiting the stage */
		  if (hmi.while_break == FALSE) hmi.while_break = TRUE;
	  }
  }

  if (GPIO_Pin == UP_BUT_Pin)
  {
//	  HAL_GPIO_TogglePin(ORANGE_LED_GPIO_Port, ORANGE_LED_Pin);
	  hmi.button = UP;
  }

  if (GPIO_Pin == DOWN_BUT_Pin)
  {
//	  HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
	  hmi.button = DOWN;
  }

  if (GPIO_Pin == OK_BUT_Pin)
  {
//	  HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
	  hmi.button = OK;

	  if(hmi.curr_state == Plot_VMAG)
	  {
		  /* Toggle to Start/Stop plotting */
		  if (hmi.while_break == TRUE) hmi.while_break = FALSE;
		  else hmi.while_break = TRUE;
	  }
  }

  if (GPIO_Pin == FORW_BUT_Pin)
  {
	  hmi.button = FORW;
  }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	/* Prevent unused argument(s) compilation warning */
	UNUSED(hadc);
	if(hadc->Instance == ADC1)
	{
		if(v_mag_idx == 200)
		{
			v_mag_idx = 0;
			Clear_Screen_Custom(11,70,210,168);
		}
		v_mag[v_mag_idx] = HAL_ADC_GetValue(&adf4351_hadc);
//		v_mag_ave += (v_mag[v_mag_idx]-v_mag_ave) / (v_mag_idx+1);
		x_curr_val = 11+v_mag_idx;
		if(v_mag[v_mag_idx] > v_mag_max_val) y_curr_val = min_val;
		else if(v_mag[v_mag_idx] < v_mag_min_val) y_curr_val = max_val;
		else y_curr_val = (v_mag[v_mag_idx] * mean_val) / v_mag_ave;

		if(v_mag_idx != 0) Bresenham_line_algorithm(x_prev_val, y_prev_val, x_curr_val, y_curr_val);

		y_prev_val = y_curr_val;
		x_prev_val = x_curr_val;

		v_mag_idx++;

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
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  mean_val = max_val-min_val+1;
  v_mag_max_val = max_val * v_mag_ave / mean_val;
  v_mag_min_val = min_val * v_mag_ave / mean_val;

//  Initialize the LCD
  lcd_init();
//  HAL_Delay(100);
  HMI_Init();
  Welcome();

  /* Test graph animation */
  /*
   * Width  (x): [2 .. WIDTH-1]
   * Height (y): [1 .. HEIGHT-2]
   * Top of graph (x,y): (10,70)
   * End of graph (x,y): (209, 169)
   */
//  uint8_t min_val = 70;	// top of y-axis
//  uint8_t max_val = 169; // bottom of y-axis
//  uint8_t mean_val = max_val-min_val+1;
//  uint16_t v_mag_max_val = max_val * v_mag_ave / mean_val;
//  uint16_t v_mag_min_val = min_val * v_mag_ave / mean_val;

//  Clear_Screen();
//  draw_string(2, 2+CHAR_HEIGHT_SMALL*0, COLOR_TEXT, 1, "Hello-Hello\0");
//  draw_string(2, 2+CHAR_HEIGHT_SMALL*1, COLOR_TEXT, 1, "Hello-Hello\0");
//  draw_pixel(2, 30, COLOR_TITLE);

//  // Draw y-axis
//  for (int i = 70; i < 170; i++)
//  {
//	  draw_pixel(10, i, COLOR_TITLE);
//  }
//  // Draw x-axis
//  for (int i = 10; i < 210; i++)
//  {
//	  draw_pixel(i, 169, COLOR_TITLE);
//  }
  //Plot v_mag data
//  uint8_t y_curr_val = 0;
//  uint8_t x_curr_val = 0;
//  uint8_t y_prev_val = 0;
//  uint8_t x_prev_val = 0;
//  for (int i = 0; i < 200; i++)
//  {
//	  x_curr_val = 10 + i;
//	  if(v_mag[i] > v_mag_max_val) y_curr_val = min_val;
//	  else if(v_mag[i] < v_mag_min_val) y_curr_val = max_val;
//	  else y_curr_val = (v_mag[i] * mean_val) / v_mag_ave;
//
//	  if(i != 0) Bresenham_line_algorithm(x_prev_val, y_prev_val, x_curr_val, y_curr_val);
//
//	  y_prev_val = y_curr_val;
//	  x_prev_val = x_curr_val;
//  }
//  Bresenham_line_algorithm(10, 100, 20, 150);
//  Bresenham_line_algorithm(20, 150, 40, 160);
//  Bresenham_line_algorithm(40, 160, 50, 169);
//  Bresenham_line_algorithm(50, 169, 60, 125);
//  Bresenham_line_algorithm(60, 125, 80, 85);

//  ADF4351_Initialize();
//
//  ADF4351_Set_Out_Freq(920000000);
//  ADF4351_Set_Out_Freq(50000000);

//  for (int i = 0; i < 400; i++)
//  {
//	  HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
//	  HAL_ADC_Start_IT(&adf4351_hadc);
//	  HAL_Delay(100);
//  }
//  HAL_TIM_Base_Start(&htim1);
//  HAL_TIM_Base_Start(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  while(!hmi.state_hold)
	  {
		  hmi.curr_state();
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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
