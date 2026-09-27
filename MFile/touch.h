	
#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "sys.h"

#define TP_PRES_DOWN 0x80  //Touch screen is pressed	  
#define TP_CATH_PRES 0x40  //A button is pressed 	  
										    
//Touch screen controller
typedef struct
{
	u8 (*init)(void);			//Initialize the touch screen controller
	u8 (*scan)(u8);				//Scan the touch screen. 0, screen scan; 1, physical coordinates;	 
	void (*adjust)(void);		//Touch screen calibration
	u16 x0;						//Original coordinates (the coordinates when first pressed)
	u16 y0;
	u16 x; 						//Current coordinates (the coordinates of the touch screen during this scan)
	u16 y;						   	    
	u8  sta;					//Pen state
//b7: press 1/release 0;
//b6:0, no button is pressed; 1, there is a button pressed.
////////////////////////Touch screen calibration parameters/////////////////////////								
	float xfac;					
	float yfac;
	short xoff;
	short yoff;	   
// Newly added parameters, which are needed when the left and right sides of the touch screen are completely reversed.
//When touchtype=0, it is suitable for TP where the left and right are X coordinates, and the upper and lower are Y coordinates.
//When touchtype=1, it is suitable for TP with Y coordinates for left and right, and X coordinates for up and down.
	u8 touchtype;
}_m_tp_dev;

extern _m_tp_dev tp_dev;	 	//The touch screen controller is defined in touch.c

//Connect pin to touch screen chip
//Connect to the touch screen chip	  

#define PEN    		HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6)   //T_PEN
#define DOUT      HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)     //T_MISO
#define TDIN(n)   (n?HAL_GPIO_WritePin(GPIOE,GPIO_PIN_6,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOE,GPIO_PIN_6,GPIO_PIN_RESET))     //T_MOSI
#define TCLK(n)   (n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET))     //T_SCK
#define TCS(n)    (n?HAL_GPIO_WritePin(GPIOE,GPIO_PIN_5,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOE,GPIO_PIN_5,GPIO_PIN_RESET))     //T_CS 

void TP_Write_Byte(u8 num);						//Write a data to the control chip
u16 TP_Read_AD(u8 CMD);							//Read AD conversion value
u16 TP_Read_XOY(u8 xy);							//Coordinate reading with filtering(X/Y)
u8 TP_Read_XY(u16 *x,u16 *y);					//Bidirectional reading(X+Y)
u8 TP_Read_XY2(u16 *x,u16 *y);					//Bidirectional coordinate reading with enhanced filtering
void TP_Drow_Touch_Point(u16 x,u16 y,u16 color);//Draw a coordinate calibration point
void TP_Draw_Big_Point(u16 x,u16 y,u16 color);	//Draw a big point
u8 TP_Scan(u8 tp);								//scanning
void TP_Save_Adjdata(void);						//Save calibration parameters
u8 TP_Get_Adjdata(void);						//Read calibration parameters
void TP_Adjust(void);							//Touch screen calibration
u8 TP_Init(void);								//initialization
																 
	  
#endif

















