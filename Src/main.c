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
float frequency_analysis[DIV_SPECT];

// Données des mots
int time_word = 0;
char mot[80] = {0};
float un[DIV_SPECT + 1] = {6.135052407407407, 6.135052407407407, 6.135052407407407, 6.169463518518518, 6.169463518518518, 6.169463518518518, 6.169463518518518, 6.169463518518518, 6.169463518518518, 6.169463518518518, 7.183795962962964, 9.16950640740741, 9.295256703703705, 9.317788, 9.347195481481483, 11.11354, 12.741551518518518, 14.678248555555557, 15.61856037037037, 16.13336274074074, 16.17060533333333, 16.17060533333333, 16.17060533333333, 16.17060533333333, 16.17060533333333, 16.19711714814815, 16.19711714814815, 16.19711714814815, 16.19711714814815, 16.19711714814815, 16.19711714814815, 16.19711714814815, 16.19711714814815, 16.19711714814815, 16.19711714814815, 16.19711714814815, 16.19711714814815, 16.19711714814815, 16.19711714814815, 16.197112592592596,  17.85185185185185};
float un2[DIV_SPECT + 1] = {4.478464105263159, 4.478464105263159, 4.478464105263159, 4.478464105263159, 4.756528578947369, 4.843710947368422, 4.843710947368422, 4.843710947368422, 4.843710947368422, 4.857131105263158, 4.87545352631579, 6.214986684210526, 7.202530157894737, 7.538606000000001, 8.499658842105262, 9.878003210526316, 11.951752789473684, 13.98242010526316, 16.196683, 17.36399268421053, 17.36399268421053, 17.36399268421053, 17.42685668421053, 17.42685668421053, 17.42685668421053, 17.458924789473684, 17.472665421052636, 17.472665421052636, 17.486074789473687, 17.499511789473686, 17.499511789473686, 17.499511789473686, 17.499511789473686, 17.499511789473686, 17.499511789473686, 17.499511789473686, 17.499511789473686, 17.499511789473686, 17.499511789473686, 17.499505263157896, 20.157894736842106};
float six[DIV_SPECT + 1] = {3.626421681818182, 3.626421681818182, 4.062622772727273, 4.740602045454545, 4.959002909090909, 5.918038681818181, 8.233945909090908, 9.587762454545453, 9.804754045454546, 9.820065181818181, 9.820065181818181, 9.820065181818181, 9.820065181818181, 9.820065181818181, 9.820065181818181, 9.820065181818181, 9.820065181818181, 9.820065181818181, 9.902749863636364, 11.20101990909091, 12.33207240909091, 12.648842136363639, 12.800371227272723, 13.904630499999998, 15.489148136363639, 15.926258409090908, 16.067483818181817, 17.092281272727273, 18.27875240909091, 18.31056931818182, 18.31056931818182, 18.31056931818182, 18.31056931818182, 18.31056931818182, 18.31056931818182, 18.31056931818182, 18.31056931818182, 18.31056931818182, 18.31056931818182, 18.310564090909093, 20.227272727272727};
float six2[DIV_SPECT + 1] = {5.216588947368422, 5.216588947368422, 5.333706842105264, 5.480627263157896, 5.558521684210527, 7.811034894736842, 11.183943947368421, 11.748633157894734, 12.306631421052629, 12.362080947368419, 12.362080947368419, 12.362080947368419, 12.362080947368419, 12.362080947368419, 12.362080947368419, 12.362080947368419, 12.362080947368419, 12.362080947368419, 12.58561284210526, 13.353554684210522, 13.410075999999998, 13.410075999999998, 13.410075999999998, 13.504398842105262, 14.423252315789473, 14.548320157894736, 14.548320157894736, 14.885346736842106, 15.067163263157896, 15.067163263157896, 15.067163263157896, 15.067163263157896, 15.067163263157896, 15.067163263157896, 15.067163263157896, 15.067163263157896, 15.067163263157896, 15.067163263157896, 15.067163263157896, 15.06715894736842, 20.105263157894736};
float loup[DIV_SPECT + 1] = {5.9845999090909086, 5.9845999090909086, 5.9845999090909086, 6.54432559090909, 7.944152090909092, 10.95615109090909, 13.417794363636368, 13.417794363636368, 13.417794363636368, 13.417794363636368, 13.417794363636368, 13.417794363636368, 13.417794363636368, 13.417794363636368, 13.417794363636368, 13.417794363636368, 13.417794363636368, 13.417794363636368, 13.417794363636368, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436678863636367, 13.436673636363635, 15.045454545454545};
float loup2[DIV_SPECT + 1] = {5.438439466666666, 5.438439466666666, 5.465095133333333, 6.660954399999999, 8.892506666666668, 11.533411866666665, 13.301203866666667, 13.327628666666667, 13.391701133333333, 13.391701133333333, 13.391701133333333, 13.391701133333333, 13.391701133333333, 13.391701133333333, 13.391701133333333, 13.391701133333333, 13.391701133333333, 13.413679733333334, 13.413679733333334, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.45664986666667, 13.456644666666667, 15.733333333333333};
float kangooroo[DIV_SPECT + 1] = {2.2597199090909093, 2.282991590909091, 3.4309165909090917, 4.598482090909091, 5.147789636363638, 5.700865409090909, 6.740501909090908, 6.981367500000001, 7.645394454545456, 8.419520500000003, 9.309272227272725, 10.08114568181818, 10.82750318181818, 11.567539045454543, 12.3986145, 13.31123468181818, 13.944175363636361, 14.223583863636367, 14.33438081818182, 14.386776545454547, 14.416295000000002, 14.439535181818183, 14.541843090909094, 14.686905772727272, 14.810176272727274, 14.94341736363636, 15.12230618181818, 15.788084318181818, 16.532028954545456, 16.965086318181818, 16.980076045454545, 16.99022418181818, 16.99706259090909, 17.003314863636362, 17.00982940909091, 17.01641759090909, 17.033066363636365, 17.036900409090908, 17.040743636363636, 17.054946363636365, 29.90909090909091};
float deux[DIV_SPECT + 1] = {6.557953904761906, 6.557953904761906, 6.557953904761906, 6.557953904761906, 7.963479904761904, 8.089067428571427, 8.14259619047619, 8.578872476190476, 12.210054476190479, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.83174652380952, 13.831742380952381, 14.523809523809524};
float deux2[DIV_SPECT + 1] = {6.457002052631578, 6.457002052631578, 6.457002052631578, 6.562733421052631, 8.897272789473684, 10.089427526315788, 10.298710315789473, 10.298710315789473, 12.399929684210527, 13.547755, 13.547755, 13.547755, 13.547755, 13.547755, 13.547755, 13.547755, 13.547755, 13.547755, 13.567141947368423, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646919315789477, 13.646915263157894, 15.0};
float mere[DIV_SPECT + 1] = {2.921701833333334, 2.921701833333334, 2.921701833333334, 3.0020660000000006, 3.798970388888889, 4.2988764999999995, 4.581831333333334, 4.6682522777777775, 5.4440025, 6.401391499999999, 7.464198611111112, 9.001845000000001, 10.415070166666666, 11.389116555555557, 12.508963444444442, 13.485229722222222, 14.241808222222225, 14.780498666666668, 15.544176833333333, 16.255472500000003, 16.565880111111117, 16.66401605555556, 16.69342261111111, 16.699955111111112, 16.699955111111112, 16.708660055555555, 16.708660055555555, 16.708660055555555, 16.708660055555555, 16.708660055555555, 16.708660055555555, 16.708660055555555, 16.708660055555555, 16.708660055555555, 16.708660055555555, 16.708660055555555, 16.708660055555555, 16.708660055555555, 16.708660055555555, 16.708656666666666, 23.72222222222222};
float grand[DIV_SPECT + 1] = {5.096619038461539, 5.096619038461539, 5.096619038461539, 5.096619038461539, 5.56973096153846, 5.755340461538462, 5.777421153846154, 5.777421153846154, 6.125655846153844, 6.285621615384613, 6.555489884615384, 8.46250473076923, 9.527386923076925, 10.134115884615385, 10.771955384615383, 11.29488792307692, 11.594631384615385, 11.679710923076922, 12.51403469230769, 13.868824076923078, 14.604635961538463, 14.849635115384615, 14.983814884615386, 15.054481653846157, 15.167210923076926, 15.680642692307693, 16.59267373076923, 17.38823526923077, 17.67627115384615, 17.73708923076923, 17.79062353846154, 17.821496884615385, 17.821496884615385, 17.821496884615385, 17.821496884615385, 17.821496884615385, 17.821496884615385, 17.821496884615385, 17.821496884615385, 17.821493076923076, 19.115384615384617};
float gauche[DIV_SPECT + 1] = {5.74199884, 5.74199884, 5.77219608, 5.80016324, 5.80016324, 5.994239759999999, 7.961148359999999, 10.287088919999999, 13.628982520000001, 13.899143280000002, 13.899143280000002, 13.899143280000002, 13.899143280000002, 13.899143280000002, 13.899143280000002, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.99508716, 13.995082000000002, 20};
float gauche2[DIV_SPECT + 1] = {3.2187824210526315, 3.2348633684210526, 3.680200631578947, 4.361393789473683, 4.626939421052632, 5.604615157894737, 7.920398631578947, 9.638421368421051, 11.24091689473684, 12.376457736842104, 12.484293736842105, 12.528141842105263, 12.610180368421053, 13.525570578947368, 14.33126836842105, 14.717727631578947, 14.724541315789473, 14.724541315789473, 14.724541315789473, 14.724541315789473, 14.724541315789473, 14.724541315789473, 14.724541315789473, 14.724541315789473, 14.724541315789473, 14.724541315789473, 14.738045894736842, 14.762798105263157, 14.762798105263157, 14.762798105263157, 14.762798105263157, 14.762798105263157, 14.762798105263157, 14.762798105263157, 14.762798105263157, 14.762798105263157, 14.762798105263157, 14.762798105263157, 14.762798105263157, 14.762794210526316, 23.263157894736842};
float droite[DIV_SPECT + 1] = {7.248772823529413, 7.248772823529413, 7.248772823529413, 7.248772823529413, 7.248772823529413, 7.248772823529413, 7.78372176470588, 7.78372176470588, 8.697911823529411, 9.188022705882352, 10.790580823529414, 12.623291823529412, 13.224491882352943, 13.318507294117648, 13.34942811764706, 13.676734705882351, 14.058097294117646, 14.14254188235294, 14.14254188235294, 14.14254188235294, 14.14254188235294, 14.14254188235294, 14.14254188235294, 14.14254188235294, 14.14254188235294, 14.289626764705883, 14.352249529411763, 14.352249529411763, 14.513449117647058, 14.66954282352941, 14.66954282352941, 14.66954282352941, 14.66954282352941, 14.66954282352941, 14.66954282352941, 14.66954282352941, 14.66954282352941, 14.66954282352941, 14.66954282352941, 14.669538823529413, 16.0};
float droite2[DIV_SPECT + 1] = {7.434386130434783, 7.434386130434783, 7.434386130434783, 7.434386130434783, 7.434386130434783, 7.446879608695652, 8.133469782608698, 8.35135886956522, 9.568365260869564, 10.05588308695652, 11.306013, 12.819654652173915, 13.42105895652174, 13.448839391304348, 13.548600260869565, 13.74084204347826, 13.977418999999998, 14.107308217391301, 14.127764521739126, 14.127764521739126, 14.127764521739126, 14.127764521739126, 14.127764521739126, 14.127764521739126, 14.127764521739126, 14.148116608695648, 14.148116608695648, 14.148116608695648, 14.169893652173908, 14.3032902173913, 14.3032902173913, 14.3032902173913, 14.3032902173913, 14.3032902173913, 14.3032902173913, 14.3032902173913, 14.3032902173913, 14.3032902173913, 14.3032902173913, 14.360866086956523, 16.043478260869566};
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
	for (int i = 0; i < DIV_SPECT; i++) {
		for (int j = 0; j < N_MEL/DIV_SPECT; j++) {
			if (mel_values[j+i*N_MEL/DIV_SPECT] > LEVEL_3) temp += mel_values[j + i*N_MEL/DIV_SPECT];
		}
		temp /= (N_MEL/DIV_SPECT);
		frequency_analysis[i] += temp;
	}
	
	//Testing new
//	for (int i = 0; i < N_MEL; i++) {
//		if (mel_values[i]) signal_analysis[(signal_index/(50/HOR_DIV))][(i/(40/VER_DIV))] += mel_values[i];
//		signal_test2[i + signal_index*10] = mel_values[i];
//	}
//	signal_index++;
	
	//Testing new
	
	
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

float correlation(float* resultat, float* mot) {
	  float sum_X = 0; 
	  float sum_Y = 0; 
		float	sum_XY = 0; 
    float squareSum_X = 0;
		float squareSum_Y = 0; 
  
    for (int i = 0; i < DIV_SPECT; i++) 
    { 
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
    float corr = (float)(DIV_SPECT*sum_XY-sum_X*sum_Y)/sqrt((DIV_SPECT*squareSum_X-sum_X*sum_X)*(DIV_SPECT*squareSum_Y-sum_Y*sum_Y)); 
  
    return corr; 
}

int compareWord(float* resultat, float* max_value) {
	float erreur_un = correlation(resultat, un);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_un += (abs((int) (resultat[i] - un[i]))/un[i]);
//		//printf("un\r\n%i:%f\r\n", i, (abs((int) (resultat[i] - un[i]))/un[i]));
//	}
	if ((abs((int) (time_word - un[40]))/un[40]) > 0.15)
		erreur_un -= 1;
//	erreur_un += (abs((int) (time_word - un[40]))/un[40]);
//	erreur_un /= (DIV_SPECT + 1);
//	printf("un\r\n%4.2f\r\n", erreur_un);
//	printf("%4.2f\r\n", abs((int) (time_word - un[40]))/un[40]);
	
		float erreur_un2 = correlation(resultat, un2);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_un += (abs((int) (resultat[i] - un[i]))/un[i]);
//		//printf("un\r\n%i:%f\r\n", i, (abs((int) (resultat[i] - un[i]))/un[i]));
//	}
	if ((abs((int) (time_word - un2[40]))/un2[40]) > 0.15)
		erreur_un2 -= 1;
//	erreur_un += (abs((int) (time_word - un[40]))/un[40]);
//	erreur_un /= (DIV_SPECT + 1);
//	printf("un\r\n%4.2f\r\n", erreur_un);
//	printf("%4.2f\r\n", abs((int) (time_word - un[40]))/un[40]);
	
	float erreur_six = correlation(resultat, six);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_six += (abs((int) (resultat[i] - six[i]))/six[i]);
//		//printf("%i:%f\r\n", i, (abs((int) (resultat[i] - six[i]))/six[i]));
//	}
	if ((abs((int) (time_word - six[40]))/six[40]) > 0.15)
		erreur_six -= 1;
//	erreur_six += (abs((int) (time_word - six[40]))/six[40]);
//	erreur_six /= (DIV_SPECT + 1);
//	printf("six\r\n%4.2f\r\n", erreur_six);
//	printf("%4.2f\r\n", abs((int) (time_word - six[40]))/six[40]);
	
	float erreur_six2 = correlation(resultat, six2);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_six += (abs((int) (resultat[i] - six[i]))/six[i]);
//		//printf("%i:%f\r\n", i, (abs((int) (resultat[i] - six[i]))/six[i]));
//	}
	if ((abs((int) (time_word - six2[40]))/six2[40]) > 0.15)
		erreur_six2 -= 1;
//	erreur_six += (abs((int) (time_word - six[40]))/six[40]);
//	erreur_six /= (DIV_SPECT + 1);
//	printf("six\r\n%4.2f\r\n", erreur_six);
//	printf("%4.2f\r\n", abs((int) (time_word - six[40]))/six[40]);
	
	float erreur_loup = correlation(resultat, loup);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_loup += (abs((int) (resultat[i] - loup[i]))/loup[i]);
//		//printf("loup\r\n%i:%f\r\n", i, (abs((int) (resultat[i] - loup[i]))/loup[i]));
//	}
	if ((abs((int) (time_word - loup[40]))/loup[40]) > 0.15)
		erreur_loup -= 1;
//	erreur_loup += (abs((int) (time_word - loup[40]))/loup[40]);
//	erreur_loup /= (DIV_SPECT + 1);
//	printf("loup\r\n%4.2f\r\n", erreur_loup);
//	printf("%4.2f\r\n", abs((int) (time_word - loup[40]))/loup[40]);
	
	float erreur_loup2 = correlation(resultat, loup2);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_loup += (abs((int) (resultat[i] - loup[i]))/loup[i]);
//		//printf("loup\r\n%i:%f\r\n", i, (abs((int) (resultat[i] - loup[i]))/loup[i]));
//	}
	if ((abs((int) (time_word - loup2[40]))/loup2[40]) > 0.15)
		erreur_loup2 -= 1;
//	erreur_loup += (abs((int) (time_word - loup[40]))/loup[40]);
//	erreur_loup /= (DIV_SPECT + 1);
//	printf("loup\r\n%4.2f\r\n", erreur_loup);
//	printf("%4.2f\r\n", abs((int) (time_word - loup[40]))/loup[40]);
	
	float erreur_kangooroo = correlation(resultat, kangooroo);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_kangooroo += (abs((int) (resultat[i] - kangooroo[i]))/kangooroo[i]);
//	}
	if ((abs((int) (time_word - kangooroo[40]))/kangooroo[40]) > 0.15)
		erreur_kangooroo -= 1;
//	erreur_kangooroo += (abs((int) (time_word - kangooroo[40]))/kangooroo[40]);
//	erreur_kangooroo /= (DIV_SPECT + 1);
//	printf("kangooroo\r\n%4.2f\r\n", erreur_kangooroo);
//	printf("%4.2f\r\n", abs((int) (time_word - kangooroo[40]))/kangooroo[40]);
	
	float erreur_deux = correlation(resultat, deux);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_deux += (abs((int) (resultat[i] - deux[i]))/deux[i]);
//	}
	if ((abs((int) (time_word - deux[40]))/deux[40]) > 0.15)
		erreur_deux -= 1;
//	erreur_deux += (abs((int) (time_word - deux[40]))/deux[40]);
//	erreur_deux /= (DIV_SPECT + 1);
//	printf("deux\r\n%4.2f\r\n", erreur_deux);
//	printf("%4.2f\r\n", abs((int) (time_word - deux[40]))/deux[40]);
	
	float erreur_deux2 = correlation(resultat, deux2);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_deux += (abs((int) (resultat[i] - deux[i]))/deux[i]);
//	}
	if ((abs((int) (time_word - deux2[40]))/deux2[40]) > 0.15)
		erreur_deux2 -= 1;
//	erreur_deux += (abs((int) (time_word - deux[40]))/deux[40]);
//	erreur_deux /= (DIV_SPECT + 1);
//	printf("deux\r\n%4.2f\r\n", erreur_deux);
//	printf("%4.2f\r\n", abs((int) (time_word - deux[40]))/deux[40]);
	
	float erreur_grand = correlation(resultat, grand);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_deux += (abs((int) (resultat[i] - deux[i]))/deux[i]);
//	}
	if ((abs((int) (time_word - grand[40]))/grand[40]) > 0.15)
		erreur_grand -= 1;
//	erreur_deux += (abs((int) (time_word - deux[40]))/deux[40]);
//	erreur_deux /= (DIV_SPECT + 1);
//	printf("grand\r\n%4.2f\r\n", erreur_grand);
//	printf("%4.2f\r\n", abs((int) (time_word - grand[40]))/grand[40]);
	
	float erreur_mere = correlation(resultat, mere);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_deux += (abs((int) (resultat[i] - deux[i]))/deux[i]);
//	}
	if ((abs((int) (time_word - mere[40]))/mere[40]) > 0.15)
		erreur_grand -= 1;
//	erreur_deux += (abs((int) (time_word - deux[40]))/deux[40]);
//	erreur_deux /= (DIV_SPECT + 1);
//	printf("mere\r\n%4.2f\r\n", erreur_mere);
//	printf("%4.2f\r\n", abs((int) (time_word - mere[40]))/mere[40]);
	
	float erreur_gauche = correlation(resultat, gauche);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_deux += (abs((int) (resultat[i] - deux[i]))/deux[i]);
//	}
	if ((abs((int) (time_word - gauche[40]))/gauche[40]) > 0.15)
		erreur_gauche -= 1;
//	erreur_deux += (abs((int) (time_word - deux[40]))/deux[40]);
//	erreur_deux /= (DIV_SPECT + 1);
//	printf("mere\r\n%4.2f\r\n", erreur_gauche);
//	printf("%4.2f\r\n", abs((int) (time_word - gauche[40]))/gauche[40]);
	
	float erreur_gauche2 = correlation(resultat, gauche2);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_deux += (abs((int) (resultat[i] - deux[i]))/deux[i]);
//	}
	if ((abs((int) (time_word - gauche2[40]))/gauche2[40]) > 0.15)
		erreur_gauche2 -= 1;
//	erreur_deux += (abs((int) (time_word - deux[40]))/deux[40]);
//	erreur_deux /= (DIV_SPECT + 1);
//	printf("mere\r\n%4.2f\r\n", erreur_gauche);
//	printf("%4.2f\r\n", abs((int) (time_word - gauche[40]))/gauche[40]);
	
	float erreur_droite = correlation(resultat, droite);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_deux += (abs((int) (resultat[i] - deux[i]))/deux[i]);
//	}
	if ((abs((int) (time_word - droite[40]))/droite[40]) > 0.15)
		erreur_droite -= 1;
//	erreur_deux += (abs((int) (time_word - deux[40]))/deux[40]);
//	erreur_deux /= (DIV_SPECT + 1);
//	printf("mere\r\n%4.2f\r\n", erreur_gauche);
//	printf("%4.2f\r\n", abs((int) (time_word - gauche[40]))/gauche[40]);
	
	float erreur_droite2 = correlation(resultat, droite2);
//	for (int i = 0; i < DIV_SPECT; i++) {
//		erreur_deux += (abs((int) (resultat[i] - deux[i]))/deux[i]);
//	}
	if ((abs((int) (time_word - droite2[40]))/droite2[40]) > 0.15)
		erreur_droite2 -= 1;
//	erreur_deux += (abs((int) (time_word - deux[40]))/deux[40]);
//	erreur_deux /= (DIV_SPECT + 1);
//	printf("mere\r\n%4.2f\r\n", erreur_gauche);
//	printf("%4.2f\r\n", abs((int) (time_word - gauche[40]))/gauche[40]);
	
	float erreur[12] = {erreur_un, erreur_six, erreur_loup, erreur_deux, erreur_gauche, erreur_deux2, erreur_un2, erreur_loup2, erreur_six2, erreur_gauche2, erreur_droite, erreur_droite2};
	uint32_t max_index;
	arm_max_f32(erreur, 12, max_value, &max_index);
	return max_index;
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
	sprintf(mot, "            ");
//	if (compareWordFreq(loup, frequency_analysis, 0.4) == 1) sprintf(mot, "loup");
//	else if (compareWordFreq(un, frequency_analysis, 0.4) == 1) sprintf(mot, "un");
//	else if (compareWordFreq(six, frequency_analysis, 0.4) == 1) sprintf(mot, "six");
	float max_value = 0;
	int max_index = compareWord(frequency_analysis, &max_value);
	printf("%i\r\n", max_index);
	if (max_value >= 0.96) {
		if (max_index == 0) sprintf(mot, "un      ");
		else if (max_index == 1) sprintf(mot, "six      ");
		else if (max_index == 2) sprintf(mot, "loup    ");
		else if (max_index == 3) sprintf(mot, "deux    ");
		else if (max_index == 4) sprintf(mot, "gauche    ");
		else if (max_index == 5) sprintf(mot, "deux    ");
		else if (max_index == 6) sprintf(mot, "un      ");
		else if (max_index == 7) sprintf(mot, "loup    ");
		else if (max_index == 8) sprintf(mot, "six      ");
		else if (max_index == 9) sprintf(mot, "gauche    ");
		else if (max_index == 10) sprintf(mot, "droite      ");
		else if (max_index == 11) sprintf(mot, "droite    ");
		else sprintf(mot, "                ");
		//printf(mot);
		//printf("\r\n\r\n");
	}
	
	// Reinitialize word
	for (int i = (DIV_SPECT - 1); i >= 0; i--) {
		frequency_analysis[i] = 0;
	}
	time_word = 0;
}
//

void wordAnalysisTest(void) {
//	int max_index_x = 0;
//	int max_index_y = 0;
//	float max_value = 0.0;
//	
//	for (int i = 0; i < HOR_DIV; i++) {
//		for (int j = 0; j < VER_DIV; j++) {
//			signal_analysis[i][j] /= MOYENNE;
//			if (signal_analysis[i][j] > max_value) {max_index_x = i; max_index_y = j; max_value = signal_analysis[i][j];}
//		}
//	}
//	
//	printf("max:%f\r\n", max_value);
//	for (int i = 0; i < HOR_DIV; i++) {
//		for (int j = 0; j < VER_DIV; j++) {
//			signal_analysis[i][j] /= max_value;
//			if (signal_analysis[i][j] > 0.8) signal_detect[i][j] = 1;
//			else signal_detect[i][j] = 0;
//		}
//	}
//	
//	printSignal();
//	printf("\r\n");
//	printDetect();
//	
//	printf("(%i,%i,%i)\r\n", max_index_x, max_index_y, signal_index);
//	if (max_index_x == un[0] && max_index_y == un[1] && signal_index <= un[2]*(1+0.1) && signal_index <= un[2]*(1+0.1)) printf("un\r\n");
//	else if (max_index_x == rhino[0] && max_index_y == rhino[1] && signal_index <= rhino[2]*(1+0.1) && signal_index <= rhino[2]*(1+0.1)) printf("rhinoceros\r\n");
//	else if (max_index_x == gateau[0] && max_index_y == gateau[1] && signal_index <= gateau[2]*(1+0.1) && signal_index <= gateau[2]*(1+0.1)) printf("gateau\r\n");
//	else if (max_index_x == ballon[0] && max_index_y == ballon[1] && signal_index <= ballon[2]*(1+0.1) && signal_index <= ballon[2]*(1+0.1)) printf("ballon\r\n");
//	else printf("rip\r\n");
//	signal_index = 0;
//	
//	for (int i = 0; i < HOR_DIV; i++) {
//		for (int j = 0; j < 4; j++) {
//			signal_analysis[i][j] = 0;
//			signal_detect[i][j] = 0;
//		}
//	}
}



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
				sprintf(lcd_data.mot, mot);
				draw_screen(&lcd_data);
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
