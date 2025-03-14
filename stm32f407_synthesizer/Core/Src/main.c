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
extern uint16_t volatile vmag[1000];
extern uint16_t volatile vmag_val;
extern uint16_t volatile vmag_min;
extern uint16_t volatile vmag_ave;
extern uint16_t volatile vmag_idx;
extern uint8_t  volatile vmag_offset;
extern uint16_t volatile vmag_upper_bound;
extern uint16_t volatile vmag_lower_bound;

extern uint8_t  volatile y_min_val;
extern uint8_t  volatile y_max_val;
extern uint8_t  volatile y_mean_val;
extern uint8_t  volatile y_curr_val;
extern uint8_t  volatile x_curr_val;
extern uint8_t  volatile y_prev_val;
extern uint8_t  volatile x_prev_val;

uint16_t volatile idx = 0;
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

  if(hmi.button == NONE)
  {
	  if(GPIO_Pin == BACK_BUT_Pin)
	  {
		  hmi.button = BACK;

		  if(hmi.curr_state == Plot_VMAG)
		  {
			  /* Make sure the plotting is stopped before exiting the stage */
			  if (hmi.while_break == FALSE) hmi.while_break = TRUE;
		  }
	  }

	  if (GPIO_Pin == UP_BUT_Pin)
	  {
		  hmi.button = UP;
	  }

	  if (GPIO_Pin == DOWN_BUT_Pin)
	  {
		  hmi.button = DOWN;
	  }

	  if (GPIO_Pin == OK_BUT_Pin)
	  {
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
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	/* Prevent unused argument(s) compilation warning */
	UNUSED(hadc);
	if(hadc->Instance == ADC1)
	{
		/* Reset the screen condition */
		if(vmag_idx == SAMPLES+OFFSET)
		{
			vmag_idx = 0;
			vmag_ave = 0;
			Clear_Screen_Custom(X_AXIS_LEFT+1,Y_AXIS_TOP,X_AXIS_RIGHT,Y_AXIS_BOT-1);
		}

		/* Read VMAG values from ADC */
		vmag_val = HAL_ADC_GetValue(&adf4351_hadc);
		if( vmag_val < vmag_min)
		{
			vmag_min = vmag_val;
			adf4351.scan_freq = adf4351.calc_freq;
		}
		vmag[idx] = vmag_val;
		idx++;

		/* The first OFFSET values to find VMAG average */
		if(vmag_idx < OFFSET)
		{
			vmag_ave += vmag_val;
		}
		else
		{
			if(vmag_idx == OFFSET)
			{
				vmag_ave /= OFFSET;
				y_mean_val = y_max_val-y_min_val+1;
				vmag_upper_bound = y_max_val * vmag_ave / y_mean_val;
				vmag_lower_bound = y_min_val * vmag_ave / y_mean_val;
			}

			/* Draw the pattern */
			x_curr_val = (vmag_idx-OFFSET) + (X_AXIS_LEFT+1);
			if(vmag_val > vmag_upper_bound) y_curr_val = y_min_val;
			else if(vmag_val < vmag_lower_bound) y_curr_val = y_max_val;
			else y_curr_val = (vmag_val * y_mean_val) / vmag_ave;

			/* The higher y_value, the lower the y-plot */
			y_curr_val = y_curr_val + 2*(y_mean_val-y_curr_val);

			if(vmag_idx > OFFSET)
			{
				Bresenham_line_algorithm(x_prev_val, y_prev_val, x_curr_val, y_curr_val);
			}

			y_prev_val = y_curr_val;
			x_prev_val = x_curr_val;
		}
		vmag_idx++;
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
  lcd_init();
  HMI_Init();
  Welcome();
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
