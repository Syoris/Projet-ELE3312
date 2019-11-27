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
#include <string.h>

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
#define HOR_DIV 5
#define VER_DIV 4
#define MOYENNE (50/HOR_DIV)/(40/VER_DIV)




// Lecture du senseur ultrasonique
volatile unsigned int pulse_width = 0;
volatile unsigned int last_captured = 0;
volatile unsigned int signal_polarity = 0;

// Affichage de l'écran
volatile int local_time = 0;
volatile int local_time_spectre = 0;
volatile int flag_update_lcd = 0;

struct LCD_Data lcd_data;

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

volatile int adc_done_left = 0;  //Lecture de l'adc
volatile int adc_done_right = 0;
volatile int begin_word = 0; //Detection de la voix
volatile int end_word = 0;
volatile int end_count = -1;
volatile int update_spectre = 0; //Dessin du spectre

// Valeurs déterminées
float angle = 0;	//Angle de la personne
float prev_angle = 0;
float distance = 0.0;
struct LCD_Data dataLCD;

// Mels Variables
float mel_values[N_MEL];
int hor_spectre_pos = 60;
int hor_spectre_pos_start = 60;
int vert_spectre_pos = 239;
float frequency_analysis[DIV_SPECT*5];

// Données des mots
int time_word = 0;
char mot[80] = {0};
float un[DIV_SPECT*5 + 1] = {44.0188588888889, 44.0188588888889, 44.0188588888889, 44.0188588888889, 44.0188588888889, 44.0188588888889, 44.0188588888889, 44.0188588888889, 44.0188588888889, 44.0188588888889, 44.444393277777785, 51.809078500000005, 54.044749833333334, 58.39025316666667, 68.58751583333331, 75.48424238888889, 83.3840671111111, 92.44774594444443, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 16.185702277777775, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 17};
float six[DIV_SPECT*5 + 1] = {11.748767882352942, 11.962529294117646, 17.880744529411764, 23.272927941176473, 26.467921705882354, 35.445492411764704, 46.693808294117645, 54.348085235294114, 59.61453541176471, 61.2412421764706, 61.46861388235295, 61.46861388235295, 61.46861388235295, 61.46861388235295, 61.46861388235295, 61.46861388235295, 61.46861388235295, 61.83240929411766, 68.80303800000002, 77.62758864705881, 80.20420523529413, 80.35231241176473, 80.5786253529412, 85.1275199411765, 92.66874747058824, 94.17125129411762, 94.95764211764705, 98.63493747058823, 98.71503311764707, 98.71503311764707, 98.71503311764707, 98.71503311764707, 98.71503311764707, 98.71503311764707, 98.71503311764707, 98.71503311764707, 99.1184364117647, 99.28192588235295, 99.44833152941177, 100.0, 8.664300176470588, 8.664300176470588, 8.664300176470588, 8.664300176470588, 8.664300176470588, 8.664300176470588, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.818593352941177, 8.896729117647059, 8.974944235294117, 8.974944235294117, 8.974944235294117, 9.19269723529412, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 18.647058823529413};
float loup[DIV_SPECT*5 + 1] = {35.28023376470588, 35.28023376470588, 35.28023376470588, 37.10909994117647, 47.620454470588236, 72.16548235294117, 89.14065588235293, 89.35721088235293, 92.7670424117647, 97.62189488235295, 97.95285752941176, 97.95285752941176, 97.95285752941176, 97.95285752941176, 97.95285752941176, 97.95285752941176, 97.95285752941176, 97.95285752941176, 98.69552570588235, 99.12169064705881, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 16.135514588235292, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 15.411764705882353};
float deux[DIV_SPECT*5 + 1] = {53.958265941176464, 53.958265941176464, 53.958265941176464, 53.958265941176464, 54.353776411764706, 55.42206258823529, 55.42206258823529, 55.42206258823529, 76.39982605882354, 98.07143176470588, 98.35469011764705, 98.35469011764705, 98.35469011764705, 98.35469011764705, 98.35469011764705, 98.35469011764705, 98.35469011764705, 98.35469011764705, 98.35469011764705, 98.35469011764705, 98.35469011764705, 98.35469011764705, 98.35469011764705, 99.41500047058824, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 20.62340429411765, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 14.411764705882353};
float gauche[DIV_SPECT*5 + 1] = {7.5775926190476195, 7.9603288095238085, 13.206289666666667, 18.840807238095238, 22.475221142857137, 29.863697904761903, 38.298161761904765, 45.78397580952382, 54.53506723809524, 62.623296285714275, 67.25063947619049, 71.9625051904762, 76.82858785714285, 81.27470723809522, 86.34432914285715, 90.9105200952381, 91.60259761904764, 91.8778221904762, 92.0761570952381, 92.1339922857143, 92.19074295238096, 92.52642461904763, 93.49725823809524, 94.23675257142857, 94.4451349047619, 94.4451349047619, 95.66932790476191, 99.6743541904762, 100.0, 100.0, 100.0, 100.0, 100.0,100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 6.511766761904761, 6.511766761904761, 7.139379476190475, 7.559839095238097, 7.763328857142857, 9.16599780952381, 11.604427619047616, 13.680322285714286, 15.421535380952381, 16.556614285714282, 16.849072571428565, 17.190417095238093, 17.329452476190472, 17.46102128571428, 17.46102128571428, 17.536491142857138, 17.536491142857138, 17.536491142857138, 17.536491142857138, 17.536491142857138, 17.536491142857138, 17.536491142857138, 17.536491142857138, 17.536491142857138, 17.536491142857138, 17.536491142857138, 17.536491142857138, 17.578290619047614, 17.578290619047614, 17.578290619047614, 17.578290619047614, 17.578290619047614, 17.578290619047614, 17.578290619047614, 17.578290619047614, 17.578290619047614, 17.578290619047614, 17.578290619047614, 17.578290619047614, 17.578290619047614, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 2.231074523809524, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 23.523809523809526};
float droite[DIV_SPECT*5 + 1] = {17.968766470588236, 17.968766470588236, 18.62057429411765, 18.910732705882353, 19.01588329411765, 25.816914235294117, 32.731193235294114, 39.835899294117645, 52.56455988235294, 62.17215258823529, 72.237549, 81.05434464705881, 84.38078847058823, 86.21694623529412, 87.57825458823528, 89.12372405882354, 90.1651692352941, 90.1651692352941, 90.1651692352941, 90.1651692352941, 90.1651692352941, 90.1651692352941, 90.1651692352941, 90.1651692352941, 92.80112147058823, 94.7256267647059, 94.7256267647059, 94.88655852941176, 99.4142765882353, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 14.101876588235294, 14.101876588235294, 14.101876588235294, 14.101876588235294, 14.101876588235294, 14.101876588235294, 14.326338647058824, 14.626875529411764, 15.902864470588236, 18.581137588235293, 21.96544547058823, 25.31823676470588, 26.97437664705882, 27.285867823529404, 28.05527564705882, 29.066089, 29.331061176470588, 29.331061176470588, 29.331061176470588, 29.331061176470588, 29.331061176470588, 29.331061176470588, 29.331061176470588, 29.331061176470588, 31.148308705882346, 31.957723, 31.957723, 31.957723, 32.862682529411764, 33.36282247058824, 33.44791494117648, 33.44791494117648, 33.44791494117648, 33.44791494117648, 33.44791494117648, 33.44791494117648, 33.44791494117648, 33.44791494117648, 33.44791494117648, 33.44791494117648, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 2.961686294117647, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 20.764705882352942};
float riz[DIV_SPECT*5 + 1] = {22.935959736842104, 22.935959736842104, 23.365992789473683, 39.86956757894737, 43.112110052631586, 44.780713210526315, 50.09750321052633, 51.40035768421053, 51.927828526315785, 51.927828526315785, 51.927828526315785, 51.927828526315785, 51.927828526315785, 51.927828526315785, 51.927828526315785, 51.927828526315785, 51.927828526315785, 51.927828526315785, 51.927828526315785, 52.045826947368425, 52.17188963157895, 52.70981652631579, 54.274255526315805, 57.73781563157895, 60.22837494736842, 66.47023468421052, 75.2786652631579, 78.57869100000002, 85.98152, 95.06169489473685, 98.720049, 99.44113805263157, 99.44113805263157, 99.44113805263157, 99.44113805263157, 99.44113805263157, 99.44113805263157, 99.5550392631579, 100.0, 100.0, 18.397724736842108, 18.397724736842108, 18.571616736842106, 20.34111868421052, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 20.541929578947364, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 1.3066115789473687, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 20.105263157894736};
	//float gateau[3] = {3,3,37};
//float un[3] = {1,3,20};
//float rhino[3] = {4,3, 60};
//float ballon[3] = {2,3,30};

//// TEST1
//float signal_analysis[HOR_DIV][VER_DIV];
//float signal_detect[HOR_DIV][VER_DIV];
//int signal_index = 0;

//// TEST2
//float un2[100][40] = {0};

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

// Convertit les tableaux d'entrées (int) en float
void convIntFloat(uint32_t* input_int, float* input_float) {
	for (int i = 0; i < TABLE_LENGTH; i++) {
		input_float[i] = ((float) input_int[i]);
	}
}
//

void printSignal(void) {
//	for (int i = 0; i < VER_DIV; i++) {
//		for (int j = 0; j < HOR_DIV; j++) {
//			printf("%3.2f ", signal_analysis[j][i]);
//		}
//		printf("\r\n");
//	}
}

void printDetect(void) {
//	for (int i = 0; i < VER_DIV; i++) {
//		for (int j = 0; j < HOR_DIV; j++) {
//			printf("%3.2f ", signal_detect[j][i]);
//		}
//		printf("\r\n");
//	}
}

// Enlève la moyenne aux données
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
	int tab_stamp = (int) time_word/10;
	for (int i = 0; i < DIV_SPECT; i++) {
		for (int j = 0; j < N_MEL/DIV_SPECT; j++) {
			if (mel_values[j+i*N_MEL/DIV_SPECT] > LEVEL_3) temp += mel_values[j + i*N_MEL/DIV_SPECT];
		}
		temp /= (N_MEL/DIV_SPECT);
		if (tab_stamp < 5) { 
			frequency_analysis[i + 40*tab_stamp] += temp;}
	}
}
//

float correlation(float* resultat, float* mot) {
	  float sum_X = 0; 
	  float sum_Y = 0; 
		float	sum_XY = 0; 
    float squareSum_X = 0;
		float squareSum_Y = 0; 
		int i = 0;
    for (i = 0; i < DIV_SPECT*5; i++) 
    { 
				if (resultat[i] == 0.0 && mot[i] == 0.0) break;
        // sum of elements of array X. 
        sum_X = sum_X + resultat[i]; 
  
        // sum of elements of array Y. 
        sum_Y = sum_Y + mot[i]; 
  
        // sum of X[i] * Y[i]. 
        sum_XY = sum_XY + resultat[i] * mot[i]; 
  
        // sum of square of array elements. 
        squareSum_X = squareSum_X + resultat[i] * resultat[i]; 
        squareSum_Y = squareSum_Y + mot[i] * mot[i]; 
    } 
  
    // use formula for calculating correlation coefficient. 
    float corr = (float)(i*sum_XY-sum_X*sum_Y)/sqrt((i*squareSum_X-sum_X*sum_X)*(i*squareSum_Y-sum_Y*sum_Y)); 
  
    return corr; 
}

int compareWord(float* resultat, float* max_value) {
	float erreur_un = correlation(resultat, un);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_un += (abs((int) (resultat[i] - un[i]))/un[i]);
//		//printf("un\r\n%i:%f\r\n", i, (abs((int) (resultat[i] - un[i]))/un[i]));
//	}
	if ((abs((int) (time_word - un[200]))/un[200]) > 0.15)
		erreur_un -= 1;
//	erreur_un += (abs((int) (time_word - un[40]))/un[40]);
//	erreur_un /= (DIV_SPECT + 1);
	printf("un\r\n%4.2f\r\n", erreur_un);
	printf("%4.2f\r\n", abs((int) (time_word - un[200]))/un[200]);
	

	float erreur_six = correlation(resultat, six);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_six += (abs((int) (resultat[i] - six[i]))/six[i]);
//		//printf("%i:%f\r\n", i, (abs((int) (resultat[i] - six[i]))/six[i]));
//	}
	if ((abs((int) (time_word - six[200]))/six[200]) > 0.15)
		erreur_six -= 1;
//	erreur_six += (abs((int) (time_word - six[40]))/six[40]);
//	erreur_six /= (DIV_SPECT + 1);
	printf("six\r\n%4.2f\r\n", erreur_six);
	printf("%4.2f\r\n", abs((int) (time_word - six[200]))/six[200]);
	

	float erreur_loup = correlation(resultat, loup);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_loup += (abs((int) (resultat[i] - loup[i]))/loup[i]);
//		//printf("loup\r\n%i:%f\r\n", i, (abs((int) (resultat[i] - loup[i]))/loup[i]));
//	}
	if ((abs((int) (time_word - loup[200]))/loup[200]) > 0.15)
		erreur_loup -= 1;
//	erreur_loup += (abs((int) (time_word - loup[40]))/loup[40]);
//	erreur_loup /= (DIV_SPECT + 1);
	printf("loup\r\n%4.2f\r\n", erreur_loup);
	printf("%4.2f\r\n", abs((int) (time_word - loup[200]))/loup[200]);
	

	float erreur_deux = correlation(resultat, deux);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_deux += (abs((int) (resultat[i] - deux[i]))/deux[i]);
//	}
	if ((abs((int) (time_word - deux[200]))/deux[200]) > 0.15)
		erreur_deux -= 1;
//	erreur_deux += (abs((int) (time_word - deux[40]))/deux[40]);
//	erreur_deux /= (DIV_SPECT + 1);
	printf("deux\r\n%4.2f\r\n", erreur_deux);
	printf("%4.2f\r\n", abs((int) (time_word - deux[200]))/deux[200]);
	

	float erreur_gauche = correlation(resultat, gauche);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_deux += (abs((int) (resultat[i] - deux[i]))/deux[i]);
//	}
	if ((abs((int) (time_word - gauche[200]))/gauche[200]) > 0.15)
		erreur_gauche -= 1;
//	erreur_deux += (abs((int) (time_word - deux[40]))/deux[40]);
//	erreur_deux /= (DIV_SPECT + 1);
	printf("gauche\r\n%4.2f\r\n", erreur_gauche);
	printf("%4.2f\r\n", abs((int) (time_word - gauche[200]))/gauche[200]);
	

	float erreur_droite = correlation(resultat, droite);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_deux += (abs((int) (resultat[i] - deux[i]))/deux[i]);
//	}
	if ((abs((int) (time_word - droite[200]))/droite[200]) > 0.15)
		erreur_droite -= 1;
//	erreur_deux += (abs((int) (time_word - deux[40]))/deux[40]);
//	erreur_deux /= (DIV_SPECT + 1);
	printf("droite\r\n%4.2f\r\n", erreur_droite);
	printf("%4.2f\r\n", abs((int) (time_word - droite[200]))/droite[200]);
	
	float erreur_riz = correlation(resultat, riz);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_deux += (abs((int) (resultat[i] - deux[i]))/deux[i]);
//	}
	if ((abs((int) (time_word - riz[200]))/riz[200]) > 0.15)
		erreur_riz -= 1;
//	erreur_deux += (abs((int) (time_word - deux[40]))/deux[40]);
//	erreur_deux /= (DIV_SPECT + 1);
	printf("droite\r\n%4.2f\r\n", erreur_riz);
	printf("%4.2f\r\n", abs((int) (time_word - riz[200]))/riz[200]);
	

	float erreur[5] = {erreur_un, erreur_six, erreur_loup, erreur_deux, erreur_riz};
	uint32_t max_index;
	arm_max_f32(erreur, 5, max_value, &max_index);
	return max_index;
}
//s

void wordAnalysis(void) {
	float max_value_norm = 0.0;
	uint32_t max_index_norm = 0;
	arm_max_f32(frequency_analysis, DIV_SPECT*5, &max_value_norm, &max_index_norm);
	printf("#");
	for (int i = (DIV_SPECT*5 - 1); i >= 0; i--) {
		frequency_analysis[i] = (float) frequency_analysis[i]/max_value_norm*100;
		char buf[80]; 
		sprintf(buf, "%f, ", frequency_analysis[i]); 
		printf(buf);
	}
	printf("%i, ", time_word);
	printf("\r\n");
//	if (compareWordFreq(loup, frequency_analysis, 0.4) == 1) sprintf(mot, "loup");
//	else if (compareWordFreq(un, frequency_analysis, 0.4) == 1) sprintf(mot, "un");
//	else if (compareWordFreq(six, frequency_analysis, 0.4) == 1) sprintf(mot, "six");
	float max_value = 0;
	int max_index = compareWord(frequency_analysis, &max_value);
//	printf("%i\r\n", max_index);
	sprintf(mot, "               ");
	if (max_value >= 0.96) {
		if (max_index == 0) sprintf(mot, "un      ");
		else if (max_index == 1) sprintf(mot, "six      ");
		else if (max_index == 2) sprintf(mot, "loup    ");
		else if (max_index == 3) sprintf(mot, "deux    ");
		//else if (max_index == 4) sprintf(mot, "gauche    ");
		else if (max_index == 4) sprintf(mot, "riz      ");
		else sprintf(mot, "                ");
		printf(mot);
		printf("\r\n\r\n");
	}
	
	// Reinitialize word
	for (int i = 0; i < (DIV_SPECT*5); i++) {
		frequency_analysis[i] = 0.0;
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
	lcd_data.angle = 0.0;
	lcd_data.distance = 0.0;
	
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
				if (strncmp(mot,"                ", 1) != 0) {
					sprintf(lcd_data.mot, mot);
					draw_screen(&lcd_data);
					HAL_Delay(1000);
				}
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
			if (begin_word == 1 || end_count != -1) {analyzeFrequencies(); time_word++;}
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
			
			// Calcul de la corrélation
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

// Pour update LCD à la bonne fréquence
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
