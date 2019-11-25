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
#include "LCD_Controle.h"
#include "spectrogram.h"

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
#define LEVEL_1 7
#define LEVEL_2 8
#define LEVEL_3 9
#define LEVEL_4 10
#define VOICE_LEVEL 7.5f
#define DIV_SPECT 40



// Lecture du senseur ultrasonique
volatile unsigned int pulse_width = 0;
volatile unsigned int last_captured = 0;
volatile unsigned int signal_polarity = 0;

// Affichage de l'�cran
volatile int local_time = 0;
volatile int local_time_spectre = 0;
volatile int flag_update_lcd = 0;

struct LCD_Data lcd_data;

//Tableau des donn�es en entr�e
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
float correlate_tab[(2*TABLE_LENGTH) - 1];	//Tableau des valeurs correl�es

volatile int adc_done_left = 0;  //Lecture de l'adc
volatile int adc_done_right = 0;
volatile int begin_word = 0; //Detection de la voix
volatile int end_word = 0;
volatile int end_count = -1;
volatile int update_spectre = 0; //Dessin du spectre

// Valeurs d�termin�es
float angle = 0;	//Angle de la personne
float prev_angle = 0;
float distance = 0.0;
struct LCD_Data dataLCD;

// Mels Variables
float mel_values[N_MEL];
int hor_spectre_pos = 60;
int hor_spectre_pos_start = 60;
int vert_spectre_pos = 239;
float frequency_analysis[DIV_SPECT];

// Donn�es des mots
int time_word = 0;
char mot[80] = {0};
float six[DIV_SPECT + 1] = {1.5127020937499995, 2.2168797187500004, 3.10779271875, 3.8853669374999997, 4.46051140625, 5.448689437500001, 6.49694996875, 7.044134906249999, 7.5921431875, 8.016221218749997, 8.387597625000001, 8.754221468750002, 9.094792656249998, 9.389559499999999, 9.693969374999998, 10.003269718750001, 10.308712812499996, 10.623953937500001, 11.012703187500001, 11.449483937499998, 11.83894146875, 12.166017531249999, 12.498095374999998, 12.912473125000002, 13.47130403125, 13.901282156250002, 14.271057593750001, 14.685990875, 15.302507999999998, 16.063498843749997, 17.14578515625, 17.954113437500002, 18.665002625, 19.55965634375, 20.616072281250002, 21.742268531249998, 22.89044375, 24.062729562499992, 25.229444281250004, 26.523663749999994, 27.34375};
float un[DIV_SPECT + 1] = {1.2001918333333335, 1.929320875, 2.749723166666666, 3.5230936666666666, 4.085839, 4.773302333333333, 5.468789791666667, 6.069352166666666, 6.720225791666667, 7.384090041666666, 8.04394475, 8.66216525, 9.233863000000001, 9.781386666666664, 10.354950083333332, 10.982444000000001, 11.600222708333334, 12.155137625, 12.692513666666665, 13.216339958333336, 13.674436333333334, 14.043144166666666, 14.404176416666665, 14.737287833333331, 15.096232375, 15.488047416666669, 15.932346833333332, 16.363946374999998, 16.756767624999995, 17.19934816666667, 17.961794083333334, 18.334240833333332, 18.676871333333334, 19.021702208333334, 19.331684250000002, 19.625170833333332, 19.969847875000003, 20.349309333333327, 20.748274041666665, 21.658474583333334, 23.708333333333332};
float loup[DIV_SPECT + 1] = {200,114,33,46,64};
float rhino[DIV_SPECT + 1] = {1.7755769583333334, 2.68404475, 3.6986032916666667, 4.62251625, 5.300537041666667, 6.107805750000001, 6.983207749999999, 7.655885624999997, 8.441144375, 9.137874583333334, 9.661186291666667, 10.172986916666668, 10.42269620833333, 10.61716879166667, 10.836847375, 11.073874708333333, 11.289989791666665, 11.463533458333332, 11.733283583333332, 12.077774916666668, 12.261115041666669, 12.397528000000001, 12.547400708333333, 12.749292750000002, 13.002304666666669, 13.232533666666669, 13.434371916666668, 13.670991416666668, 13.941641041666669, 14.651230875000001, 16.13379758333333, 16.941965833333335, 17.539330166666662, 18.403289791666666, 19.358835625000005, 20.19387766666667, 21.3245005, 22.644485208333332, 24.069168, 25.745784583333336,60};
//float test[4000] = {0};
//int index_test = 0;
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
	float angle = atanf(y/x)*180.0f/PI;
	
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

// Convertit les tableaux d'entr�es (int) en float
void convIntFloat(uint32_t* input_int, float* input_float) {
	for (int i = 0; i < TABLE_LENGTH; i++) {
		input_float[i] = ((float) input_int[i]);
	}
}
//

// Enl�ve la moyenne aux donn�es
void normalize(float* input_float) {
	float average;
	arm_mean_f32(input_float, TABLE_LENGTH, &average);
	for (int i = 0; i < TABLE_LENGTH; i++) {
		input_float[i] = input_float[i] - average;
	}
}
//

// Dessiner une ligne de test
void drawAngle() {
	LCD_drawLine(120, 100, 120 + 60*cos(prev_angle*PI/180), 100 + 60*sin(prev_angle*PI/180), BLACK);
	LCD_drawLine(120, 100, 120 + 60*cos(angle*PI/180), 100 + 60*sin(angle*PI/180), WHITE);
}
//

// Dessiner le spectrogramme de test
void drawSpectre() {
	for (int i = 0; i < N_MEL; i++) {
		uint16_t color = BLACK;
		if (mel_values[i] < (LEVEL_1)) color = BLACK;
		else if (mel_values[i] < (LEVEL_2) && mel_values[i] >= (LEVEL_1)) color = BLUE;
		else if (mel_values[i] < (LEVEL_3) && mel_values[i] >= (LEVEL_2)) color = YELLOW;
		else color = RED;
		LCD_fillRect(hor_spectre_pos, (vert_spectre_pos - 2*i),2,2, color);
	}
	
	hor_spectre_pos += 2;
	if (hor_spectre_pos == hor_spectre_pos_start + 100*2) { 
		hor_spectre_pos = hor_spectre_pos_start;
		LCD_fillRect(hor_spectre_pos_start, (vert_spectre_pos - 79), 200, 80, BLACK);
	}
}
//

void detectVoice(void) {
	float average;
	arm_mean_f32(mel_values, N_MEL, &average);
	if (average >= VOICE_LEVEL && begin_word == 0) {begin_word = 1; end_count = -1;}
	else if (average < VOICE_LEVEL && begin_word == 1) {end_count = 100; begin_word = 0;}
}
//

void analyzeFrequencies(void) {
	float temp;
	for (int i = 0; i < DIV_SPECT; i++) {
		for (int j = 0; j < N_MEL/DIV_SPECT; j++) {
			if (mel_values[j+i*N_MEL/DIV_SPECT] > VOICE_LEVEL) temp += mel_values[j + i*N_MEL/DIV_SPECT];
		}
		temp /= (N_MEL/DIV_SPECT);
		frequency_analysis[i] += temp;
	}
}
//

int compareWordFreq(float* mot, float* resultat, float sensibilite) {
	int compare_resultat = 0;
	for (int i = 0; i < DIV_SPECT; i++) {
		if ((resultat[i] < (mot[i]*(1+sensibilite)) && (resultat[i] > (mot[i]*(1-sensibilite))))) compare_resultat++;
	}
	if (compare_resultat == DIV_SPECT) return 1;
	else return 0;
}
//


int compareWord(float* resultat, float* min_value) {
	float erreur_un = 0.0;
	for (int i = 0; i < DIV_SPECT; i++) {
		erreur_un += (abs((int) (resultat[i] - un[i]))/un[i]);
		//printf("un\r\n%i:%f\r\n", i, (abs((int) (resultat[i] - un[i]))/un[i]));
	}
	if ((abs((int) (time_word - un[40]))/un[40]) > 0.5)
		erreur_un += (DIV_SPECT + 1);
	erreur_un /= (DIV_SPECT + 1);
	printf("%4.2f\r\n", erreur_un);
	
	float erreur_six = 0;
	for (int i = 0; i < DIV_SPECT; i++) {
		erreur_six += (abs((int) (resultat[i] - six[i]))/six[i]);
		//printf("%i:%f\r\n", i, (abs((int) (resultat[i] - six[i]))/six[i]));
	}
	if ((abs((int) (time_word - six[40]))/six[40]) > 0.5)
		erreur_six += (DIV_SPECT + 1);
	erreur_six /= (DIV_SPECT + 1);
	printf("%4.2f\r\n", erreur_six);
	
	float erreur_loup = 1.0;
	for (int i = 0; i < DIV_SPECT; i++) {
		erreur_loup += (abs((int) (resultat[i] - loup[i]))/loup[i]);
		//printf("loup\r\n%i:%f\r\n", i, (abs((int) (resultat[i] - loup[i]))/loup[i]));
	}
	if ((abs((int) (time_word - loup[40]))/loup[40]) > 0.5)
		erreur_loup += (DIV_SPECT + 1);
	erreur_loup /= (DIV_SPECT + 1);
	//printf("%4.2f\r\n", erreur_loup);
	
	float erreur_rhino = 0;
	for (int i = 0; i < DIV_SPECT; i++) {
		erreur_rhino += (abs((int) (resultat[i] - rhino[i]))/rhino[i]);
	}
	if ((abs((int) (time_word - rhino[40]))/rhino[40]) > 0.5)
		erreur_rhino += (DIV_SPECT + 1);
	erreur_rhino /= (DIV_SPECT + 1);
	printf("%4.2f\r\n", erreur_rhino);
	
	float erreur[3] = {erreur_un, erreur_six, erreur_rhino};
	uint32_t min_index;
	arm_min_f32(erreur, 3, min_value, &min_index);
	return min_index;
}
//s

void wordAnalysis(void) {
	float total = 0;
	for (int i = 0; i < DIV_SPECT; i++) { total += frequency_analysis[i]; }
	total /= 5;
	printf("#");
	for (int i = (DIV_SPECT - 1); i >= 0; i--) {
		frequency_analysis[i] = (float) frequency_analysis[i]/total*100;
		char buf[80]; 
		sprintf(buf, "%f, ", frequency_analysis[i]); 
		printf(buf);
	}
	printf("%i, ", time_word);
	printf("\r\n");
	sprintf(mot, "rip");
//	if (compareWordFreq(loup, frequency_analysis, 0.4) == 1) sprintf(mot, "loup");
//	else if (compareWordFreq(un, frequency_analysis, 0.4) == 1) sprintf(mot, "un");
//	else if (compareWordFreq(six, frequency_analysis, 0.4) == 1) sprintf(mot, "six");
	float min_value = 0;
	int min_index = compareWord(frequency_analysis, &min_value);
	//printf("%i\r\n", min_index);
	if (min_value <= 0.1) {
		if (min_index == 0) sprintf(mot, "un      ");
		else if (min_index == 1) sprintf(mot, "six      ");
		else if (min_index == 4) sprintf(mot, "loup    ");
		else if (min_index == 2) sprintf(mot, "rhino    ");
		printf(mot);
		printf("\r\n\r\n");
	}
	
	// Reinitialize word
	for (int i = (DIV_SPECT - 1); i >= 0; i--) {
		frequency_analysis[i] = 0;
	}
	time_word = 0;
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
	init_screen();
	
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
		if (end_word == 1) {
				wordAnalysis();
				end_word = 0;
				HAL_Delay(1000);
		}
		//
		
		// To update LCD
		if (flag_update_lcd == 1 && end_count == -1) {
			distance = pulse_width/58.0;
			lcd_data.angle = angle;
			lcd_data.distance = distance;
			sprintf(lcd_data.mot, mot);
			draw_screen(&lcd_data);
			
			prev_angle = angle;
			flag_update_lcd = 0;
		}
		//
		
		if (update_spectre == 1) {
			drawSpectre();
			detectVoice();
			if (begin_word == 1 || end_count != -1) {analyzeFrequencies(); time_word++; }
			update_spectre = 0;
		}
		//
		
		// Once ADC is done
		if (adc_done_left == 1 && adc_done_right == 1) {
			// Convert the output
			convIntFloat(input_tab_left_inv, input_tab_left_f);
			convIntFloat(input_tab_right_inv, input_tab_right_f);
			
			// Remove the average
			normalize(input_tab_left_f);
			normalize(input_tab_right_f);
			
			// Calcul de la corr�lation
			arm_correlate_f32(input_tab_left_f, TABLE_LENGTH, input_tab_right_f, TABLE_LENGTH, correlate_tab);
			
			// Find index of maximum
			float d = maxDisplace(correlate_tab);
			
			// Compute angle
			angle = findAngle(d);
			
			// Find Mel Coefficients
			compute_log_mel_coefficients(input_tab_left_f, mel_values);
			
			adc_done_left = 0;
			adc_done_right = 0;
		}
		//
		
	/* USER CODE END WHILE */
	/* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
//

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
//

// Pour update LCD � la bonne fr�quence
void HAL_SYSTICK_Callback() {
	
	if (local_time >= 100) {
		local_time = 0;
		flag_update_lcd = 1;
	}
	//
	
	if (local_time_spectre >= 10) {
		update_spectre = 1;
		local_time_spectre = 0;
	}
	//
	
	if (end_count == 0) end_word = 1;
	if (end_count > -1) end_count--;
	
	local_time++;
	local_time_spectre++;
}
//

// Quand la capture du ECHO est finie
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
//

// Quand la lecture des ADC est finie
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
		HAL_ADC_Start_DMA(&hadc1, input_tab_left, TABLE_LENGTH);
		
		adc_done_left = 1;
	}
	if (hadc == &hadc2) {
		if (input_tab_right == tab_right_1) {
			input_tab_right = tab_right_2;
			input_tab_right_inv = tab_right_1;
		} 
		else {
			input_tab_right = tab_right_1;
			input_tab_right_inv = tab_right_2;
		}
		HAL_ADC_Start_DMA(&hadc2, input_tab_right, TABLE_LENGTH);
		
		adc_done_right = 1;
	}
}
//

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
