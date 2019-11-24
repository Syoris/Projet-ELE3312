#include "LCD_Controle.h"

 void init_screen(void){
 LCD_Begin();
 LCD_SetRotation(1);
 LCD_FillScreen(BLACK);
 LCD_SetTextSize(3);
 draw_coches();
}
//
 
void draw_cadran(float angle){
//	LCD_FillRect(0, 100, 500, 160, BLACK);
	int x_centre = 155;
	int y_centre = 150;
	int radius = 75;
	static float prev_x;
	static float prev_y;
	LCD_DrawLine(prev_x, prev_y, x_centre, y_centre - 5, BLACK); //Erase previous line

	float longueur_angle = radius - 5;
	float x_angle = x_centre - cos(angle*PI/180.0f) * longueur_angle;
	float y_angle = y_centre - sin(angle*PI/180.0f) * longueur_angle;
	LCD_DrawLine(x_angle, y_angle, x_centre, y_centre - 5, BLUE); //Draw current angle
	prev_x = x_angle;
	prev_y = y_angle;
}
//

void draw_screen(struct LCD_Data *printData){
	char buffer[80] = {0};
	LCD_setCursor(0,0);
	
	sprintf(buffer, "Mot:");
	LCD_printf(buffer);
	LCD_printf(printData->mot);
 
	sprintf(buffer, "\r\nAngle: %f \r\n", printData->angle);
	LCD_printf(buffer);

	
	sprintf(buffer, "Distance: %f \r\n", printData->distance);
	LCD_printf(buffer);
 
	draw_cadran(printData->angle);
}
//

void draw_coches(void){
	int x_centre = 155;
	int y_centre = 150;
	int radius = 75;

	int n_coche = 5;
	float angle_per_coche = 180.0f/n_coche;
	float longueur_coche = 10.0;

	
	for (int i = 0; i<5; i++){
		LCD_DrawCircle(x_centre, y_centre, radius-2+i, RED);
		LCD_DrawFastHLine(x_centre-radius, y_centre-i, 2*radius, RED);
	}
		
	for (int i = 1; i <= n_coche; i++){
		float x_start = x_centre - cos(i*angle_per_coche*PI/180.0f-angle_per_coche*PI/360.0f) * (longueur_coche/2.0f + radius);
		float y_start = y_centre - sin(i*angle_per_coche*PI/180.0f-angle_per_coche*PI/360.0f) * (longueur_coche/2.0f + radius);
		float x_fin = x_centre - cos(i*angle_per_coche*PI/180.0f-angle_per_coche*PI/360.0f) * (-longueur_coche/2.0f + radius);
		float y_fin = y_centre - sin(i*angle_per_coche*PI/180.0f-angle_per_coche*PI/360.0f) * (-longueur_coche/2.0f + radius);

		LCD_DrawLine(x_start, y_start, x_fin, y_fin, BLUE);
	}
	LCD_fillRect(0, y_centre, 400, 120, BLACK);
}
//
