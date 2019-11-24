#include "LCD_Controle.h"

 void init_screen(void){
 LCD_Begin();
 LCD_SetRotation(0);
 LCD_FillScreen(BLACK);
 LCD_SetTextSize(3);
}
//
 
void draw_cadran(float angle){
	LCD_FillRect(0, 100, 500, 160, BLACK);
	int x_centre = 155;
	int y_centre = 240;
	int radius = 100;
	
	for (int i = 0; i<5; i++){
		LCD_DrawCircle(x_centre, 240, 98+i, RED);
		LCD_DrawFastHLine(x_centre-100, 240-i, 200, RED);
	}
	
	int n_coche = 5;
	float angle_per_coche = 180.0f/n_coche;
	float longueur_coche = 10.0;
	
	for (int i = 1; i <= n_coche; i++){
		float x_start = x_centre - cos(i*angle_per_coche*PI/180.0f-angle_per_coche*PI/360.0f) * (longueur_coche/2.0f + radius);
		float y_start = y_centre - sin(i*angle_per_coche*PI/180.0f-angle_per_coche*PI/360.0f) * (longueur_coche/2.0f + radius);
		float x_fin = x_centre - cos(i*angle_per_coche*PI/180.0f-angle_per_coche*PI/360.0f) * (-longueur_coche/2.0f + radius);
		float y_fin = y_centre - sin(i*angle_per_coche*PI/180.0f-angle_per_coche*PI/360.0f) * (-longueur_coche/2.0f + radius);
		
		LCD_DrawLine(x_start, y_start, x_fin, y_fin, BLUE);
	}
	float longueur_angle = 135;
	float x_angle = x_centre - cos(angle*PI/180.0f) * longueur_angle;
	float y_angle = y_centre - sin(angle*PI/180.0f) * longueur_angle;
	LCD_DrawLine(x_angle, y_angle, x_centre, y_centre, BLUE);
}
//
 
void draw_screen(struct LCD_Data *printData){
	char buffer[20] = {0};
	
	//LCD_FillScreen(BLACK);
	LCD_setCursor(0,0);
	LCD_printf(buffer);
	
	sprintf(buffer, "Mot:");
	LCD_printf(buffer);
	LCD_printf(printData->mot);
	sprintf(buffer, "\r\n");
	LCD_printf(buffer);
 
	sprintf(buffer, "Angle: %f \r\n", printData->angle);
	LCD_printf(buffer);

	
	sprintf(buffer, "Distance: %f \r\n", printData->distance);
	LCD_printf(buffer);
 
	draw_cadran(printData->angle);
}
//
