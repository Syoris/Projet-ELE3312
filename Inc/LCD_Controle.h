#ifndef LCD_CONTROLE_H
#define LCD_CONTROLE_H

#include "MCUFRIEND_kbv.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265359f

struct LCD_Data{
	char mot[80];
	float distance;
	float angle;
};

void init_screen(void);
void draw_cadran(float angle);
void draw_screen(struct LCD_Data *printData);

#endif
