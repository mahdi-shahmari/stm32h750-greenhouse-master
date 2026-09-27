//////////////////////////////////////////////////////////////////////////////////	 
//This program is for learning only, and it cannot be used for any other purposes without the author's permission
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
//       SCK         Pick up          PB13         //LCD SPI bus clock signal
//      DC/RS        Pick up          PD5          //LCD screen data/command control signal
//       RST         Pick up          PD12         //LCD reset control signal
//       CS          Pick up          PD11         //LCD chip selection control signal
//=========================================Touch screen touch wire=========================================//
//If the module does not have touch function or has touch function, but does not need touch function, there is no need for touch screen wiring
//	   LCD module                STM32 microcontroller 
//      T_IRQ        Pick up          PH11         //Touch screen touch interrupt signal
//      T_DO         Pick up          PG3          //Touch screen SPI bus read signal
//      T_DIN        Pick up          PI3          //Touch screen SPI bus write signal
//      T_CS         Pick up          PI8          //Touch screen chip selection control signal
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

#ifndef _MYIIC_H
#define _MYIIC_H
#include "sys.h"
	
//IO direction setting
#define SDA_IN()  {GPIOB->MODER&=~(3<<(11*2));GPIOB->MODER|=0<<11*2;}	//PH5 input mode
#define SDA_OUT() {GPIOB->MODER&=~(3<<(11*2));GPIOB->MODER|=1<<11*2;} //PH5 output mode
//IO operation
#define IIC_SCL(n)  (n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET)) //SCL
#define IIC_SDA(n)  (n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET)) //SDA
#define READ_SDA    HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_7)  //Enter SDA

//IIC all operation functions
void IIC_Init(void);                //Initialize the IO port of IIC				 
void IIC_Start(void);				//Send IIC start signal
void IIC_Stop(void);	  			//Send IIC stop signal
void IIC_Send_Byte(u8 txd);			//IIC sends a byte
u8 IIC_Read_Byte(unsigned char ack);//IIC read one byte
u8 IIC_Wait_Ack(void); 				//IIC waits for ACK signal
void IIC_Ack(void);					//IIC sends ACK signal
void IIC_NAck(void);				//IIC does not send ACK signal

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	 
#endif

