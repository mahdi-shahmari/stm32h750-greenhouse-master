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
#include "spi.h"
#include "lcd.h"
#include "stm32h7xx_hal.h"

extern SPI_HandleTypeDef hspi4;
static volatile uint8_t spi_dma_busy = 0;

#define FILL_LINE_PIXELS   480
#pragma location = "DMA_RAM"
static uint16_t fill_line_buf[FILL_LINE_PIXELS];

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI4)
    {
        LCD_CS_SET;
        spi_dma_busy = 0;
    }
}
// 1. Keep your standard byte writer exactly as it is for commands/registers
void SPIv_WriteData(u8 Data)
{
    HAL_SPI_Transmit(&hspi4, &Data, 1, HAL_MAX_DELAY);
}

/*****************************************************************************
 * ADD THIS to your existing SPI.c (append below SPIv_WriteData, don't
 * replace anything - your blocking SPIv_WriteData() stays exactly as-is
 * for command/register writes, this is only for bulk pixel data).
 *****************************************************************************/



/* Small reusable line buffer for solid-color DMA fills - avoids needing a
 * full 320*480*2 = 307200 byte framebuffer just to clear the screen. */


/*****************************************************************************
 * @name   :void SPIv_WriteDataBuffer_DMA(uint16_t *pData, uint32_t Size)
 * @brief  :Bulk pixel push via DMA. Size = number of 16-bit pixels.
 *          Handles CS/DC itself - caller just needs LCD_SetWindows() done
 *          first (which internally sends the RAMWR command via the normal
 *          blocking path, that's fine/expected).
 * @note   :Blocks until any PREVIOUS transfer finishes before starting a
 *          new one, but returns as soon as the NEW transfer has started
 *          (async) - call SPIv_DMA_Wait() if you need to know it's done
 *          before touching pData again.
******************************************************************************/

void SPIv_WriteDataBuffer_DMA(uint16_t *pData, uint32_t Size)
{
    /* Wait for any previous transfer to finish */
    while (HAL_SPI_GetState(&hspi4) != HAL_SPI_STATE_READY) { }

    LCD_CS_CLR;
    LCD_RS_SET;

    HAL_StatusTypeDef status = HAL_SPI_Transmit_DMA(&hspi4, (uint8_t*)pData, Size * 2);
    if (status != HAL_OK)
    {
        /* Transfer didn't actually start - release CS so we don't leave
           the bus held low forever, and bail. Put a breakpoint here if
           you ever hit it - tells us HAL_SPI_Transmit_DMA is refusing
           to start (e.g. wrong state at call time). */
        LCD_CS_SET;
        return;
    }
    /* NOTE: CS is released in SPIv_DMA_Wait() below once the transfer is
       confirmed complete via state polling - NOT here, transfer is async. */
}

/*****************************************************************************
 * @name   :void SPIv_DMA_Wait(void)
 * @brief  :Blocks until hspi4 returns to READY state (transfer complete),
 *          then releases CS.
******************************************************************************/
void SPIv_DMA_Wait(void)
{
    while (HAL_SPI_GetState(&hspi4) != HAL_SPI_STATE_READY) { }
    LCD_CS_SET;
}



/*****************************************************************************
 * @name   :void LCD_Clear_Fast_DMA(uint16_t Color)
 * @brief  :Full-screen fill via DMA - replaces the slow byte-by-byte
 *          LCD_Clear() loop. Call LCD_SetWindows() first (or this calls it
 *          itself for a full-screen fill, see below).
******************************************************************************/
void LCD_Clear_Fast_DMA(uint16_t Color)
{
    uint32_t i;
    uint32_t num_pixels = (uint32_t)lcddev.width * (uint32_t)lcddev.height;
    /* ST7796 expects MSB first per pixel over SPI - swap here once, so the
     * raw byte transmit in SPIv_WriteDataBuffer_DMA sends correct order. */
    uint16_t swapped = (Color >> 8) | (Color << 8);

    for (i = 0; i < FILL_LINE_PIXELS; i++)
        fill_line_buf[i] = swapped;

    LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1);

    while (num_pixels > 0)
    {
        uint32_t chunk = (num_pixels > FILL_LINE_PIXELS) ? FILL_LINE_PIXELS : num_pixels;
        SPIv_WriteDataBuffer_DMA(fill_line_buf, chunk);
        SPIv_DMA_Wait();   // simple/safe: wait each chunk before reusing the buffer
        num_pixels -= chunk;
    }
}

/*****************************************************************************
 * @name   :void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
 * @brief  :HAL DMA transfer-complete callback (weak in HAL, overridden here).
 *          Releases LCD_CS and clears the busy flag once DMA actually
 *          finishes clocking data out.
 * @note   :If SPI4 is ever used for anything besides the LCD, add checks
 *          here - for now it's LCD-only so no ambiguity.
******************************************************************************/

//void  SPIv_WriteData(u8 Data)
//{
//  HAL_SPI_Transmit(&hspi4, &Data, 1, HAL_MAX_DELAY);
//  unsigned char i=0;
//  for(i=8;i>0;i--)
//  {
//    if(Data&0x80)	
//      SPI_MOSI_SET; //Output Data
//    else 
//      SPI_MOSI_CLR;
//    
//    SPI_SCLK_CLR;       
//    SPI_SCLK_SET;
//    Data<<=1; 
//  }
//}
