/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#define ARM_MATH_CM4
#include "arm_math.h"
#include "arm_const_structs.h"
#include "stdio.h"
#include "stdlib.h"
#include "MCUFRIEND_kbv.h"

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
#define TABLE_LENGTH 160
#define C 15

// Lecture du senseur ultrasonique
volatile unsigned int pulse_width = 0;
volatile unsigned int last_captured = 0;
volatile unsigned int signal_polarity = 0;

// Affichage de l'écran
volatile int local_time = 0;
volatile int flag_update_lcd = 0;

//Tableau des données en entrée
uint32_t tab_left_1[TABLE_LENGTH];
uint32_t tab_left_2[TABLE_LENGTH];
uint32_t tab_right_1[TABLE_LENGTH];
uint32_t tab_right_2[TABLE_LENGTH];

uint32_t *input_tab_left = tab_left_1;
uint32_t *input_tab_right = tab_right_1;
uint32_t *input_tab_left_inv = tab_left_2;
uint32_t *input_tab_right_inv = tab_right_2;

float input_tab_left_f[TABLE_LENGTH];
float input_tab_right_f[TABLE_LENGTH];
float correlate_tab[(2*TABLE_LENGTH) - 1];	//Tableau des valeurs correlées

volatile int adc_done = 0;  //Lecture de l'adc

// Valeurs déterminées
float angle = 0;	//Angle de la personne
float distance = 0.0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

// Trouve l'angle de la personne en fonction de l'index max
float findAngle(float max_index) {
	float d = ((float)((int)max_index - 159)/16000.0f);
	float a = d*34000.0f/2.0f;
	float r = distance;
	float y = sqrt((pow(r,2) - pow(a,2))/(1+(pow(a,2)/(pow(C,2)-pow(a,2)))));
	float x = sqrt(pow(r,2)-pow(y,2));
	float angle = atan(y/x)*180.0f/PI;
	
	if (max_index < 159) return angle;
	else return (180.0f-angle);
}
//

// Find index of maximum value in correlate tab
float maxDisplace(float* input_float) {
	float max_value;
	uint32_t max_index;
	arm_max_f32(input_float, (2*TABLE_LENGTH)-1, &max_value, &max_index);
	//float return_value = ((float)((int)max_index - 159)/16000.0);
	return max_index;
}
//

// Convertit les tableaux d'entrées (int) en float
void convIntFloat(uint32_t* input_int, float* input_float) {
	for (int i = 0; i < TABLE_LENGTH; i++) {
		input_float[i] = ((float) input_int[i]);
	}
}
//

// Enlève la moyenne aux données
void normalize(float* input_float) {
	float average;
	arm_mean_f32(input_float, TABLE_LENGTH, &average);
	for (int i = 0; i < TABLE_LENGTH; i++) {
		input_float[i] = input_float[i] - average;
	}
}
//

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
  MX_USART2_UART_Init();
  MX_TIM4_Init();
  MX_TIM8_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	LCD_Begin();
	HAL_Delay(20);
	LCD_SetRotation(0);
	LCD_FillScreen(BLACK);
	
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_3);
	
	HAL_TIM_Base_Start(&htim2);
	HAL_ADC_Start_DMA(&hadc1, input_tab_left, TABLE_LENGTH);
	HAL_ADC_Start_DMA(&hadc2, input_tab_right, TABLE_LENGTH);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
		if (flag_update_lcd == 1) {
			distance = pulse_width/58.0f;
			LCD_setCursor(0,0);
			char buf[80];
			sprintf(buf, "%4.2f\r\n %4.2f\r\n", distance, angle);
			printf(buf);
			LCD_printf(buf);
			flag_update_lcd = 0;
		}
		if (adc_done == 1) {
			convIntFloat(input_tab_left_inv, input_tab_left_f);
			convIntFloat(input_tab_right_inv, input_tab_right_f);
			
			normalize(input_tab_left_f);
			normalize(input_tab_right_f);
			arm_correlate_f32(input_tab_left_f, TABLE_LENGTH, input_tab_right_f, TABLE_LENGTH, correlate_tab);
			float d = maxDisplace(correlate_tab);
			angle = findAngle(d);
			
			adc_done = 0;
		}
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
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
}

/* USER CODE BEGIN 4 */
/**  * @brief Retargets the C library printf function to the USART.  
* @param None  
* @retval None  */ 
PUTCHAR_PROTOTYPE {  
	/* Place your implementation of fputc here */  
	/* e.g. write a character to the USART2 and Loop until the end of transmission */  
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF); 
	return ch; 
}

void HAL_SYSTICK_Callback() {
	if (local_time == 500) {
		local_time = 0;
		flag_update_lcd = 1;
	}
	local_time++;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM8) {
		int current_captured = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
		int temp_captured = current_captured;
		signal_polarity = 1 - signal_polarity;
		
		if (signal_polarity == 0) {
			if (current_captured <= last_captured) current_captured += 65535;
			pulse_width = current_captured - last_captured;
		}
		
		last_captured = temp_captured;	
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	if (hadc == &hadc1) {
		if (input_tab_left == tab_left_1) {
			input_tab_left = tab_left_2;
			input_tab_left_inv = tab_left_1;
		} 
		else {
			input_tab_left = tab_left_1;
			input_tab_left_inv = tab_left_2;
		}
		
		if (input_tab_right == tab_right_1) {
			input_tab_right = tab_right_2;
			input_tab_right_inv = tab_right_1;
		} 
		else {
			input_tab_right = tab_right_1;
			input_tab_right_inv = tab_right_2;
		}
		
		HAL_ADC_Start_DMA(&hadc1, input_tab_left, TABLE_LENGTH);
		HAL_ADC_Start_DMA(&hadc2, input_tab_right, TABLE_LENGTH);
		
		adc_done = 1;
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
