#ifndef __NRF24_H__
#define __NRF24_H__

#include "main.h"
#include "stm32h7xx_hal.h"

#define NRF_CS_CLR HAL_GPIO_WritePin(GPIOE, NRF_CSN_Pin, GPIO_PIN_RESET)
#define NRF_CS_SET HAL_GPIO_WritePin(GPIOE, NRF_CSN_Pin, GPIO_PIN_SET) 

#define NRF_CE_CLR HAL_GPIO_WritePin(GPIOE, NRF_CE_Pin, GPIO_PIN_RESET)
#define NRF_CE_SET HAL_GPIO_WritePin(GPIOE, NRF_CE_Pin, GPIO_PIN_SET) 

#define NRF_CS_LOW()  NRF_CS_CLR
#define NRF_CS_HIGH() NRF_CS_SET

#define NRF_CE_LOW()  NRF_CE_CLR
#define NRF_CE_HIGH() NRF_CE_SET

#define CMD_W_REGISTER    0x20
#define CMD_W_TX_PAYLOAD  0xA0
#define CMD_FLUSH_TX      0xE1

#define EN_AA_REG         0x01
#define SETUP_AW_REG      0x03
#define SETUP_RETR_REG    0x04
#define RF_CH_REG         0x05
#define RF_SETUP_REG      0x06
#define TX_ADDR_REG       0x10
#define RX_ADDR_P0_REG    0x0A

#define FIFO_STATUS_REG  0x17
#define FEATURE_REG      0x1D
#define DYNDPD_REG       0x1C
// nRF24L01 Register Addresses & Commands
#define CONFIG_REG      0x00
#define STATUS_REG      0x07
#define CMD_R_REGISTER  0x00 // Read command mask

// nRF24L01 Register Addresses & Commands
#define CONFIG_REG      0x00
#define STATUS_REG      0x07
#define CMD_R_REGISTER  0x00 // Read command mask
#define FIFO_STATUS_REG  0x17
#define FEATURE_REG      0x1D
#define DYNDPD_REG       0x1C
#define RX_PW_P0_REG    0x11  // Pipe 0 Payload Width (0 to 32 bytes)
#define RX_PW_P1_REG    0x12  // Pipe 1 Payload Width
#define RX_PW_P2_REG    0x13  // Pipe 2 Payload Width
#define RX_PW_P3_REG    0x14  // Pipe 3 Payload Width
#define RX_PW_P4_REG    0x15  // Pipe 4 Payload Width
#define RX_PW_P5_REG    0x16  // Pipe 5 Payload Width

#define RX_ADDR_P0_REG    0x0A   // already defined in your TX file, shown here for clarity
#define CMD_R_RX_PAYLOAD  0x61
#define CMD_FLUSH_RX      0xE2

extern uint8_t RadioAddress[] ;

uint8_t NRF24_SPI_TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t data);
uint8_t NRF24_ReadRegister(SPI_HandleTypeDef *hspi, uint8_t reg);
void NRF24_WriteRegister(SPI_HandleTypeDef *hspi, uint8_t reg, uint8_t value);
void NRF24_WriteBuffer(SPI_HandleTypeDef *hspi, uint8_t reg, uint8_t *pBuf, uint8_t len);
void NRF24_Init_TX(SPI_HandleTypeDef *hspi, uint8_t *tx_address, uint8_t channel);
uint8_t NRF24_Transmit(SPI_HandleTypeDef *hspi, uint8_t *pBuf, uint8_t len);
void NRF24_Init_RX(SPI_HandleTypeDef *hspi, uint8_t *rx_address, uint8_t channel);
uint8_t NRF24_Receive(SPI_HandleTypeDef *hspi, uint8_t *pBuf);

#endif