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
//Creation Date: 2018/08/09
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

#ifndef _24CXX_H
#define _24CXX_H
#include "sys.h"
#include "myiic.h"
	
#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767  
//STM32 F746 development board uses 24c02, so define EE_TYPE as AT24C02
#define EE_TYPE AT24C02
					  
u8 AT24CXX_ReadOneByte(u16 ReadAddr);							//Read a byte at the specified address
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite);		//Write a byte to the specified address
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len);//The specified address begins to write the specified length of data
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len);					//The specified address starts to read the specified length data
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite);	//Write data of the specified length from the specified address
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead);   	//Read the data of the specified length from the specified address

u8 AT24CXX_Check(void);  //Check the device
void AT24CXX_Init(void); //Initialize IIC
#endif
