//////////////////////////////////////////////////////////////////////////////////	 
//This program is for learning and use only, and it cannot be used for any other purpose without the author's permission
//Test hardware: single chip microcomputer STM32F743IIT6, punctual atom Apollo STM32F4/F7 development board, main frequency 400MHZ, crystal oscillator 25MHZ
//QDtech-TFT LCD driver for STM32 IO simulation
//xiaofeng@ShenZhen QDtech co.,LTD
//Company website: www.qdtft.com

//wiki technical website: http://www.lcdwiki.com
//Our company provides technical support, any technical questions are welcome to exchange and learn at any time
//Fixed line (fax): +86 0755-23594567
//Phone: 15989313508 (Feng Gong)
//Email: lcdwiki01@gmail.com support@lcdwiki.com goodtft@163.com
//Technical support QQ: 3002773612 3002778157
//Technical exchange QQ group: 324828016
//Creation Date: 2018/08/22
//Version: V1.0
//Copyright, piracy must be investigated.
//Copyright(C) Shenzhen Quandong Electronic Technology Co., Ltd. 2018-2028
//All rights reserved
/****************************************************************************************************
//=========================================Power wiring================================================//
//     LCD module                STM32 microcontroller
//      VCC          Pick up        DC5V/3.3V      //power supply
//      GND          Pick up          GND          //Power ground
//=======================================LCD screen data line wiring==========================================//
//The default data bus type of this module is SPI bus
//     LCD module                STM32 microcontroller    
//    SDI(MOSI)      Pick up          PB15         //LCD screen SPI bus data write signal
//    SDO(MISO)      Pick up          PB14         //LCD screen SPI bus data reading signal, if you don¡¯t need to read, you don¡¯t need to wire
//=======================================LCD screen control line wiring==========================================//
//     LCD module 					      STM32 microcontroller 
//       LED         Pick up          PD6          //LCD backlight control signal, if you don¡¯t need to control, connect to 5V or 3.3V
//       SCK         Pick up          PB13         //LCD screen SPI bus clock signal
//      DC/RS        Pick up          PD5          //LCD screen data/command control signal
//       RST         Pick up          PD12         //LCD reset control signal
//       CS          Pick up          PD11         //LCD chip selection control signal
//=========================================Touch screen touch wire=========================================//
//If the module does not have touch function or has touch function, but does not need touch function, there is no need to wire the touch screen
//	   LCD module                STM32 microcontroller 
//      T_IRQ        Pick up          PH11         //Touch screen touch interrupt signal
//      T_DO         Pick up          PG3          //Touch screen SPI bus read signal
//      T_DIN        Pick up          PI3          //Touch screen SPI bus write signal
//      T_CS         Pick up          PI8          //Touch screen chip select control signal
//      T_CLK        Pick up          PH6          //Touch screen SPI bus clock signal
**************************************************************************************************/	
/* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, QD electronic SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
**************************************************************************************************/

#include "lcd.h"
#include "stdlib.h"
#include "delay.h"	 
#include "spi.h"


//Manage important LCD parameters
//The default is vertical screen
_lcd_dev lcddev;

//Pen color, background color
u16 POINT_COLOR = 0x0000,BACK_COLOR = 0xFFFF;  
u16 DeviceCode;	 

/*****************************************************************************
* @name       :void LCD_WR_REG(u8 data)
* @date       :2018-08-09 
* @function   :Write an 8-bit command to the LCD screen
* @parameters :data:Command value to be written
* @retvalue   :None
******************************************************************************/
void LCD_WR_REG(u8 data)
{ 
  LCD_CS_CLR;     
  LCD_RS_CLR;	  
  SPIv_WriteData(data);
  LCD_CS_SET;	
}

/*****************************************************************************
* @name       :void LCD_WR_DATA(u8 data)
* @date       :2018-08-09 
* @function   :Write an 8-bit data to the LCD screen
* @parameters :data:data value to be written
* @retvalue   :None
******************************************************************************/
void LCD_WR_DATA(u8 data)
{
  LCD_CS_CLR;
  LCD_RS_SET;
  SPIv_WriteData(data);
  LCD_CS_SET;
}

/*****************************************************************************
* @name       :void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
* @date       :2018-08-09 
* @function   :Write data into registers
* @parameters :LCD_Reg:Register address
LCD_RegValue:Data to be written
* @retvalue   :None
******************************************************************************/
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
{	
  LCD_WR_REG(LCD_Reg);  
  LCD_WR_DATA(LCD_RegValue);	    		 
}	   

/*****************************************************************************
* @name       :void LCD_WriteRAM_Prepare(void)
* @date       :2018-08-09 
* @function   :Write GRAM
* @parameters :None
* @retvalue   :None
******************************************************************************/	 
void LCD_WriteRAM_Prepare(void)
{
  LCD_WR_REG(lcddev.wramcmd);
}	 

/*****************************************************************************
* @name       :void Lcd_WriteData_16Bit(u16 Data)
* @date       :2018-08-09 
* @function   :Write an 16-bit command to the LCD screen
* @parameters :Data:Data to be written
* @retvalue   :None
******************************************************************************/	 
void Lcd_WriteData_16Bit(u16 Data)
{	
  LCD_CS_CLR;
  LCD_RS_SET;  
  SPIv_WriteData(Data>>8);
  SPIv_WriteData(Data);
  LCD_CS_SET;
}

/*****************************************************************************
* @name       :void LCD_DrawPoint(u16 x,u16 y)
* @date       :2018-08-09 
* @function   :Write a pixel data at a specified location
* @parameters :x:the x coordinate of the pixel
y:the y coordinate of the pixel
* @retvalue   :None
******************************************************************************/	
void LCD_DrawPoint(u16 x,u16 y)
{
  LCD_SetCursor(x,y);//Set cursor position 
  Lcd_WriteData_16Bit(POINT_COLOR); 
}

/*****************************************************************************
* @name       :void LCD_Clear(u16 Color)
* @date       :2018-08-09 
* @function   :Full screen filled LCD screen
* @parameters :color:Filled color
* @retvalue   :None
******************************************************************************/	
void LCD_Clear(u16 Color)
{
  unsigned int i,m;  
  LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);   
  LCD_CS_CLR;
  LCD_RS_SET;
  for(i=0;i<lcddev.height;i++)
  {
    for(m=0;m<lcddev.width;m++)
    {	
      SPIv_WriteData(Color>>8);
      SPIv_WriteData(Color);
    }
  }
  LCD_CS_SET;
} 

/*****************************************************************************
* @name       :void LCD_Clear(u16 Color)
* @date       :2018-08-09 
* @function   :Initialization LCD screen GPIO
* @parameters :None
* @retvalue   :None
******************************************************************************/	
void LCD_GPIOInit(void)
{
  GPIO_InitTypeDef GPIO_Initure;
  __HAL_RCC_GPIOB_CLK_ENABLE();					
  __HAL_RCC_GPIOE_CLK_ENABLE();			
  __HAL_RCC_GPIOA_CLK_ENABLE();					
  
  
  //-------BACKLIGH-------------
  GPIO_Initure.Pin=GPIO_PIN_0;		
  GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  		
  GPIO_Initure.Pull=GPIO_PULLUP;         			
  GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH; 
  HAL_GPIO_Init(GPIOB,&GPIO_Initure);     		
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0 , GPIO_PIN_SET );
  
  //------SCK MOSI DC RST  -----------------------
  GPIO_Initure.Pin=GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_14| GPIO_PIN_15|GPIO_PIN_11;		
  GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  		
  GPIO_Initure.Pull=GPIO_PULLUP;         			
  GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;  	
  HAL_GPIO_Init(GPIOE,&GPIO_Initure);     		
  HAL_GPIO_WritePin(GPIOE,GPIO_PIN_15 | GPIO_PIN_14, GPIO_PIN_SET );
  HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11, GPIO_PIN_RESET );//Cs to low
  //-------------Miso---------------
  GPIO_Initure.Pin=GPIO_PIN_13;		
  GPIO_Initure.Mode=GPIO_MODE_INPUT;  		
  GPIO_Initure.Pull=GPIO_PULLUP;         			
  GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;  	
  HAL_GPIO_Init(GPIOE,&GPIO_Initure); 
}

/*****************************************************************************
* @name       :void LCD_RESET(void)
* @date       :2018-08-09 
* @function   :Reset LCD screen
* @parameters :None
* @retvalue   :None
******************************************************************************/	
void LCD_RESET(void)
{
  LCD_RST_CLR;
  delay_ms(100);	
  LCD_RST_SET;
  delay_ms(50);
}

/*****************************************************************************
* @name       :void LCD_RESET(void)
* @date       :2018-08-09 
* @function   :Initialization LCD screen
* @parameters :None
* @retvalue   :None
******************************************************************************/	 	 
void LCD_Init(void)
{  
  //LCD_GPIOInit();//LCD GPIO initialization										 
  LCD_RESET(); //LCD Reset
  //************* ST7796S initialization**********//	
  LCD_WR_REG(0xF0);
  LCD_WR_DATA(0xC3);
  LCD_WR_REG(0xF0);
  LCD_WR_DATA(0x96);
  LCD_WR_REG(0x36);
  LCD_WR_DATA(0x68);	
  LCD_WR_REG(0x3A);
  LCD_WR_DATA(0x05);	
  LCD_WR_REG(0xB0);
  LCD_WR_DATA(0x80);	
  LCD_WR_REG(0xB6);
  LCD_WR_DATA(0x00);
  LCD_WR_DATA(0x02);	
  LCD_WR_REG(0xB5);
  LCD_WR_DATA(0x02);
  LCD_WR_DATA(0x03);
  LCD_WR_DATA(0x00);
  LCD_WR_DATA(0x04);
  LCD_WR_REG(0xB1);
  LCD_WR_DATA(0x80);	
  LCD_WR_DATA(0x10);	
  LCD_WR_REG(0xB4);
  LCD_WR_DATA(0x00);
  LCD_WR_REG(0xB7);
  LCD_WR_DATA(0xC6);
  LCD_WR_REG(0xC5);
  LCD_WR_DATA(0x24);
  LCD_WR_REG(0xE4);
  LCD_WR_DATA(0x31);
  LCD_WR_REG(0xE8);
  LCD_WR_DATA(0x40);
  LCD_WR_DATA(0x8A);
  LCD_WR_DATA(0x00);
  LCD_WR_DATA(0x00);
  LCD_WR_DATA(0x29);
  LCD_WR_DATA(0x19);
  LCD_WR_DATA(0xA5);
  LCD_WR_DATA(0x33);
  LCD_WR_REG(0xC2);
  LCD_WR_REG(0xA7);
  
  LCD_WR_REG(0xE0);
  LCD_WR_DATA(0xF0);
  LCD_WR_DATA(0x09);
  LCD_WR_DATA(0x13);
  LCD_WR_DATA(0x12);
  LCD_WR_DATA(0x12);
  LCD_WR_DATA(0x2B);
  LCD_WR_DATA(0x3C);
  LCD_WR_DATA(0x44);
  LCD_WR_DATA(0x4B);
  LCD_WR_DATA(0x1B);
  LCD_WR_DATA(0x18);
  LCD_WR_DATA(0x17);
  LCD_WR_DATA(0x1D);
  LCD_WR_DATA(0x21);
  
  LCD_WR_REG(0XE1);
  LCD_WR_DATA(0xF0);
  LCD_WR_DATA(0x09);
  LCD_WR_DATA(0x13);
  LCD_WR_DATA(0x0C);
  LCD_WR_DATA(0x0D);
  LCD_WR_DATA(0x27);
  LCD_WR_DATA(0x3B);
  LCD_WR_DATA(0x44);
  LCD_WR_DATA(0x4D);
  LCD_WR_DATA(0x0B);
  LCD_WR_DATA(0x17);
  LCD_WR_DATA(0x17);
  LCD_WR_DATA(0x1D);
  LCD_WR_DATA(0x21);
  
  LCD_WR_REG(0X36);
  LCD_WR_DATA(0xEC);
  LCD_WR_REG(0xF0);
  LCD_WR_DATA(0xC3);
  LCD_WR_REG(0xF0);
  LCD_WR_DATA(0x69);
  LCD_WR_REG(0X13);
  LCD_WR_REG(0X11);
  LCD_WR_REG(0X29);
  
  LCD_direction(1);//Set LCD display direction
  LCD_LED(1);//Turn on the backlight	 
  LCD_Clear(WHITE);//Clear full screen white
}

/*****************************************************************************
* @name       :void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
* @date       :2018-08-09 
* @function   :Setting LCD display window
* @parameters :xStar:the bebinning x coordinate of the LCD display window
yStar:the bebinning y coordinate of the LCD display window
xEnd:the endning x coordinate of the LCD display window
yEnd:the endning y coordinate of the LCD display window
* @retvalue   :None
******************************************************************************/ 
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
{	
  LCD_WR_REG(lcddev.setxcmd);	
  LCD_WR_DATA(xStar>>8);
  LCD_WR_DATA(0x00FF&xStar);		
  LCD_WR_DATA(xEnd>>8);
  LCD_WR_DATA(0x00FF&xEnd);
  
  LCD_WR_REG(lcddev.setycmd);	
  LCD_WR_DATA(yStar>>8);
  LCD_WR_DATA(0x00FF&yStar);		
  LCD_WR_DATA(yEnd>>8);
  LCD_WR_DATA(0x00FF&yEnd);
  
  LCD_WriteRAM_Prepare();	//Start writing to GRAM			
}   

/*****************************************************************************
* @name       :void LCD_SetCursor(u16 Xpos, u16 Ypos)
* @date       :2018-08-09 
* @function   :Set coordinate value
* @parameters :Xpos:the  x coordinate of the pixel
Ypos:the  y coordinate of the pixel
* @retvalue   :None
******************************************************************************/ 
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{	  	    			
  LCD_SetWindows(Xpos,Ypos,Xpos,Ypos);	
} 

void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
  u16 t; 
  int xerr=0,yerr=0,delta_x,delta_y,distance; 
  int incx,incy,uRow,uCol; 
  
  delta_x=x2-x1;  
  delta_y=y2-y1; 
  uRow=x1; 
  uCol=y1; 
  if(delta_x>0)incx=1;  
  else if(delta_x==0)incx=0; 
  else {incx=-1;delta_x=-delta_x;} 
  if(delta_y>0)incy=1; 
  else if(delta_y==0)incy=0; 
  else{incy=-1;delta_y=-delta_y;} 
  if( delta_x>delta_y)distance=delta_x;  
  else distance=delta_y; 
  for(t=0;t<=distance+1;t++ ) 
  {  
    LCD_DrawPoint(uRow,uCol);
    xerr+=delta_x ; 
    yerr+=delta_y ; 
    if(xerr>distance) 
    { 
      xerr-=distance; 
      uRow+=incx; 
    } 
    if(yerr>distance) 
    { 
      yerr-=distance; 
      uCol+=incy; 
    } 
  }  
}

/*****************************************************************************
* @name       :void LCD_direction(u8 direction)
* @date       :2018-08-09 
* @function   :Setting the display direction of LCD screen
* @parameters :direction:0-0 degree
1-90 degree
2-180 degree
3-270 degree
* @retvalue   :None
******************************************************************************/ 
void LCD_direction(u8 direction)
{ 
  lcddev.setxcmd=0x2A;
  lcddev.setycmd=0x2B;
  lcddev.wramcmd=0x2C;
  switch(direction){		  
  case 0:						 	 		
    lcddev.width=LCD_W;
    lcddev.height=LCD_H;		
    LCD_WriteReg(0x36,(1<<3)|(1<<6));
    break;
  case 1:
    lcddev.width=LCD_H;
    lcddev.height=LCD_W;
    LCD_WriteReg(0x36,(1<<3)|(1<<5));
    break;
  case 2:						 	 		
    lcddev.width=LCD_W;
    lcddev.height=LCD_H;	
    LCD_WriteReg(0x36,(1<<3)|(1<<7));
    break;
  case 3:
    lcddev.width=LCD_H;
    lcddev.height=LCD_W;
    LCD_WriteReg(0x36,(1<<3)|(1<<7)|(1<<6)|(1<<5));
    break;	
  default:break;
  }		
}	 
