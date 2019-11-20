/**
  ******************************************************************************
  * File Name          : spectrogram.h
  * Description        : Computes the log-Mel coefficients for a new frame.
  * Author             : Alex Riviello
  * Last modified      : 2019/11/06
  ******************************************************************************
**/

#ifndef __SPECTROGRAM_H
#define __SPECTROGRAM_H


/* Defines */
#define N_SHIFT 160
#define N_FRAME 480
#define N_FFT   512
#define N_MEL   40
#define RFFT 		0

/* Includes */
#ifndef ARM_MATH_CM4
#define ARM_MATH_CM4
#include "arm_math.h"
#include "arm_const_structs.h"
#endif 

/* Function Prototypes */
void compute_log_mel_coefficients(float *pSrc, float* pDst);

/* Externs */
extern const int FILTER_INDICES[40];
extern const float FILTERBANK[1280];
extern const float HAMMING_WINDOW[480];

#endif
