/////////////////////////////////////////////////// ////////////////////////////////
//This program is for learning and use only, and cannot be used for any other purpose without the author's permission
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
#include "sys.h"

#ifndef _SPI_H_
#define _SPI_H_




//This test program uses simulated SPI interface driver
//You can freely change the interface IO configuration, use any minimum 4 IO to complete this LCD drive display
/******************************************************************************
The interface definition is defined in lcd.h, please modify and modify the corresponding IO according to the wiring to initialize LCD_GPIO_Init()
#define LCD_CTRL   	  	GPIOB		//Define TFT data port
#define LCD_LED        	GPIO_Pin_9  //PB9 Connect to TFT -LED
#define LCD_RS         	GPIO_Pin_10	//PB10 connected to TFT --RS
#define LCD_CS        	GPIO_Pin_11 //PB11 Connect to TFT --CS
#define LCD_RST     	GPIO_Pin_12	//PB12 connected to TFT --RST
#define LCD_SCL        	GPIO_Pin_13	//PB13 connected to TFT -- CLK
#define LCD_SDA        	GPIO_Pin_15	//PB15 connected to TFT - SDI
*******************************************************************************/

//PB13--->>TFT --SCL/SCK	
//PB15 MOSI--->>TFT --SDA/DIN
#define SPI_MOSI(n)  (n?HAL_GPIO_WritePin(GPIOE,GPIO_PIN_14,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOE,GPIO_PIN_14,GPIO_PIN_RESET))
#define SPI_SCLK(n)  (n?HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_RESET))
#define SPI_MISO  HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_13)

//#define LCD_CS_SET(x) LCD_CTRL->ODR=(LCD_CTRL->ODR&~LCD_CS)|(x ? LCD_CS:0)


#define	SPI_MOSI_SET  	SPI_MOSI(1) //LCD_CTRL->BSRR=SPI_MOSI    
#define	SPI_SCLK_SET  	SPI_SCLK(1) //LCD_CTRL->BSRR=SPI_SCLK    


//LCD control port set 0 operation sentence macro definition

#define	SPI_MOSI_CLR  	SPI_MOSI(0) //LCD_CTRL->BRR=SPI_MOSI    
#define	SPI_SCLK_CLR  	SPI_SCLK(0) //LCD_CTRL->BRR=SPI_SCLK    

void  SPIv_WriteData(u8 Data);
void LCD_Clear_Fast_DMA(uint16_t Color);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
void SPIv_WriteDataBuffer_DMA(uint16_t *pData, uint32_t Size);
void SPIv_DMA_Wait(void);

#endif
