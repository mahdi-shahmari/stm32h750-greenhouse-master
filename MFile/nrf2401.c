#include <stdint.h>
#include "nrf2401.h"

uint8_t RadioAddress[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; // Unique 5-byte target address
// Low-level SPI byte swap

#define RX_ADDR_P0_REG    0x0A   // already defined in your TX file, shown here for clarity
#define CMD_R_RX_PAYLOAD  0x61
#define CMD_FLUSH_RX      0xE2

void NRF24_Init_RX(SPI_HandleTypeDef *hspi, uint8_t *rx_address, uint8_t channel)
{
    NRF_CE_LOW();

    uint8_t cmd = CMD_FLUSH_RX;
    NRF_CS_LOW();
    HAL_SPI_Transmit(hspi , &cmd, 1, HAL_MAX_DELAY);
    NRF_CS_HIGH();

    NRF24_WriteRegister(hspi, STATUS_REG, 0x70);   // clear old flags

    NRF24_WriteRegister(hspi, RF_CH_REG, channel);
    NRF24_WriteRegister(hspi, SETUP_AW_REG, 0x03);                       // 5-byte address, must match TX
    NRF24_WriteBuffer(hspi, CMD_W_REGISTER | RX_ADDR_P0_REG, rx_address, 5);
    NRF24_WriteRegister(hspi, EN_AA_REG, 0x01);                          // Auto-Ack ON - match TX side
    NRF24_WriteRegister(hspi, RF_SETUP_REG, 0x08);                       // same power/datarate as TX
    NRF24_WriteRegister(hspi, RX_PW_P0_REG, 32);                         // same 32-byte payload as TX
    NRF24_WriteRegister(hspi, 0x02, 0x01);                                // EN_RXADDR - enable pipe 0

    // Power UP in RX Mode: PWR_UP(bit1) + PRIM_RX(bit0) + CRC bits
    NRF24_WriteRegister(hspi, CONFIG_REG, 0x0F);   // 0b00001111: EN_CRC+CRCO+PWR_UP+PRIM_RX

    HAL_Delay(5);
    NRF_CE_HIGH();   // CE stays HIGH continuously in RX mode - this is the key difference from TX
}

uint8_t NRF24_SPI_TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t data)
{
  uint8_t result = 0;
  // 10ms timeout is plenty for 1 byte
  HAL_SPI_TransmitReceive(hspi, &data, &result, 1, 10); 
  return result;
}

// Read a single register from nRF24
uint8_t NRF24_ReadRegister(SPI_HandleTypeDef *hspi, uint8_t reg)
{
  uint8_t value = 0;
  
  NRF_CS_LOW(); // Select nRF24
  
  // Send read command + register address
  NRF24_SPI_TransmitReceive(hspi, CMD_R_REGISTER | reg); 
  // Send dummy byte to clock out the register contents
  value = NRF24_SPI_TransmitReceive(hspi, 0xFF); 
  
  NRF_CS_HIGH(); // Deselect nRF24
  
  return value;
}

// Send Command + Single Byte Value in ONE burst
void NRF24_WriteRegister(SPI_HandleTypeDef *hspi,uint8_t reg, uint8_t value)
{
  uint8_t buf[2] = { CMD_W_REGISTER | reg, value };
  
  NRF_CS_LOW();
  HAL_SPI_Transmit(hspi, buf, 2, HAL_MAX_DELAY);
  NRF_CS_HIGH();
}

// Send Command + Multi-byte Buffer in ONE burst
void NRF24_WriteBuffer(SPI_HandleTypeDef *hspi, uint8_t reg, uint8_t *pBuf, uint8_t len)
{
  uint8_t cmd = reg;
  
  NRF_CS_LOW();
  HAL_SPI_Transmit(hspi, &cmd, 1, HAL_MAX_DELAY);
  HAL_SPI_Transmit(hspi, pBuf, len, HAL_MAX_DELAY);
  NRF_CS_HIGH();
}
void NRF24_Init_TX(SPI_HandleTypeDef *hspi, uint8_t *tx_address, uint8_t channel)
{
  NRF_CE_LOW(); 
  
  // Reset/Flush FIFOs just in case garbage is left over
  uint8_t cmd = 0xE1; // FLUSH_TX
  NRF_CS_LOW();
  HAL_SPI_Transmit(hspi, &cmd, 1, HAL_MAX_DELAY);
  NRF_CS_HIGH();
  
  // Clear any leftover interrupt flags (MAX_RT, TX_DS, RX_DR)
  NRF24_WriteRegister(hspi, STATUS_REG, 0x70); 
  
  // Configuration sequence
  NRF24_WriteRegister(hspi, RF_CH_REG, channel);
  NRF24_WriteRegister(hspi, SETUP_AW_REG, 0x03);                       // 5-byte address width
  NRF24_WriteBuffer(hspi, CMD_W_REGISTER | TX_ADDR_REG, tx_address, 5);
  NRF24_WriteBuffer(hspi, CMD_W_REGISTER | RX_ADDR_P0_REG, tx_address, 5);
  NRF24_WriteRegister(hspi, EN_AA_REG, 0x01);                          // Enable Auto-Ack Pipe 0
  NRF24_WriteRegister(hspi, SETUP_RETR_REG, 0x1A);                     // 500us delay, 10 retries
  NRF24_WriteRegister(hspi, RF_SETUP_REG, 0x08);                      // -18dBm low power, 2Mbps
  NRF24_WriteRegister(hspi, RX_PW_P0_REG, 32);                        // 32-byte payload size
  
  // Power UP in TX Mode
  NRF24_WriteRegister(hspi, CONFIG_REG, 0x0E); 
  HAL_Delay(100);
}

uint8_t NRF24_Transmit(SPI_HandleTypeDef *hspi, uint8_t *pBuf, uint8_t len)
{
// 1. Clear status flags before starting
    NRF24_WriteRegister(hspi, STATUS_REG, 0x70);

    // 2. Load payload into TX FIFO
    NRF24_WriteBuffer(hspi, CMD_W_TX_PAYLOAD, pBuf, len);

    // 3. Pulse CE High for at least 15 microseconds to trigger transmission
    NRF_CE_HIGH();
    for(volatile int i = 0; i < 500; i++); // Short delay
    NRF_CE_LOW();

    // 4. Wait for transmission result or timeout
    uint32_t timeout = HAL_GetTick();
    while ((HAL_GetTick() - timeout) < 100)
    {
        uint8_t status = NRF24_ReadRegister(hspi, STATUS_REG);

        // TX_DS (Data Sent & ACK received)
        if (status & 0x20) 
        {
            NRF24_WriteRegister(hspi, STATUS_REG, 0x20); // Clear flag
            //printf("Sending OK...\r\n");
            return 1; // SUCCESS
        }
        
        // MAX_RT (Maximum Retries reached, no ACK)
        if (status & 0x10) 
        {
            NRF24_WriteRegister(hspi, STATUS_REG, 0x10); // Clear flag
            // Flush FIFO so it doesn't stay jammed
            uint8_t flush_cmd = 0xE1;
            NRF_CS_LOW();
            HAL_SPI_Transmit(hspi, &flush_cmd, 1, HAL_MAX_DELAY);
            NRF_CS_HIGH();
            //printf("Sending MAX TRY...\r\n");
            return 2; // MAX RETRIES REACHED (RF engine works!)
        }
    }
//printf("Sending Error 0...\r\n");
    return 0; // TIMEOUT (Hardware/CE line issue)
}


uint8_t NRF24_Receive(SPI_HandleTypeDef *hspi, uint8_t *pBuf)
{
    uint8_t status = NRF24_ReadRegister(hspi, STATUS_REG);

    if (status & 0x40)   // RX_DR - data ready flag
    {
        uint8_t cmd = CMD_R_RX_PAYLOAD;
        NRF_CS_LOW();
        HAL_SPI_Transmit(hspi, &cmd, 1, HAL_MAX_DELAY);
        HAL_SPI_Receive(hspi, pBuf, 32, HAL_MAX_DELAY);
        NRF_CS_HIGH();

        NRF24_WriteRegister(hspi, STATUS_REG, 0x40);   // clear RX_DR flag
        return 1;   // new data received
    }
    return 0;   // nothing new
}
