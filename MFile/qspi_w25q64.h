#ifndef QSPI_w25q64_H
#define QSPI_w25q64_H

#include "stm32h7xx_hal.h"

/*----------------------------------------------- named parameter macro -------------------------------------------*/

#define QSPI_W25Qxx_OK           		0		// W25Qxx communication is normal
#define W25Qxx_ERROR_INIT         		-1		// initialization error
#define W25Qxx_ERROR_WriteEnable       -2		// write enable error
#define W25Qxx_ERROR_AUTOPOLLING       -3		// Polling waiting error, no response
#define W25Qxx_ERROR_Erase         		-4		// Erase error
#define W25Qxx_ERROR_TRANSMIT         	-5		// transmission error
#define W25Qxx_ERROR_MemoryMapped		-6    // memory map mode error

#define W25Qxx_CMD_EnableReset  		0x66		// enable reset
#define W25Qxx_CMD_ResetDevice   	0x99		// reset device
#define W25Qxx_CMD_JedecID 			0x9F		// JEDEC ID  
#define W25Qxx_CMD_WriteEnable		0X06		// write enable

#define W25Qxx_CMD_SectorErase 		0x20		// Sector erase, 4K bytes, refer to erase time 45ms
#define W25Qxx_CMD_BlockErase_32K 	0x52		// Block erase, 32K bytes, refer to erase time 120ms
#define W25Qxx_CMD_BlockErase_64K 	0xD8		// Block erase, 64K bytes, refer to erase time 150ms
#define W25Qxx_CMD_ChipErase 			0xC7		// Whole chip erase, refer to erase time 20S
#define W25Qxx_CMD_QuadInputPageProgram  	0x32  		// 1-1-4In mode (1-line command, 1-line address, 4-line data), page programming command, reference write time 0.4ms
#define W25Qxx_CMD_FastReadQuad_IO       	0xEB  		// 1-4-4In the mode (1-line command, 4-line address, 4-line data), fast read command
#define W25Qxx_CMD_ReadStatus_REG1			0X05			// Read Status Register 1
#define W25Qxx_Status_REG1_BUSY  			0x01			// Read bit 0 of status register 1 (read only), the Busy flag, will be set to 1 when an erase/write data/write command is in progress
#define W25Qxx_Status_REG1_WEL  				0x02			// Read the first bit of the status register 1 (read-only), the WEL write enable flag bit, when the flag bit is 1, it means that the write operation can be performed
#define W25Qxx_PageSize       				256			// Page size, 256 bytes
#define W25Qxx_FlashSize       				0x800000		// W25Q64 size, 8M bytes
#define W25Qxx_FLASH_ID           			0Xef4017    // W25Q64 JEDEC ID
#define W25Qxx_ChipErase_TIMEOUT_MAX		100000U		// Timeout waiting time, the maximum time required for W25Q64 chip erase is 100S
#define W25Qxx_Mem_Addr							0x90000000 	// address in memory-mapped mode


/*----------------------------------------------- Pin Configuration Macro ------------------------------------------*/

#define QUADSPI_CLK_PIN							GPIO_PIN_2				// QUADSPI_CLK pin
#define	QUADSPI_CLK_PORT						GPIOB					// QUADSPI_CLK pin port
#define	QUADSPI_CLK_AF							GPIO_AF9_QUADSPI			// QUADSPI_CLK IO port multiplexing
#define GPIO_QUADSPI_CLK_ENABLE      			                __HAL_RCC_GPIOB_CLK_ENABLE()	 	// QUADSPI_CLK pin clock
  
#define QUADSPI_BK1_NCS_PIN						GPIO_PIN_6				// QUADSPI_BK1_NCS pin
#define	QUADSPI_BK1_NCS_PORT						GPIOB					// QUADSPI_BK1_NCS pin port
#define	QUADSPI_BK1_NCS_AF						GPIO_AF10_QUADSPI			// QUADSPI_BK1_NCS IO port multiplexing
#define GPIO_QUADSPI_BK1_NCS_ENABLE        	                        __HAL_RCC_GPIOB_CLK_ENABLE()	 	// QUADSPI_BK1_NCS pin clock

#define QUADSPI_BK1_IO0_PIN						GPIO_PIN_11				// QUADSPI_BK1_IO0 pin
#define	QUADSPI_BK1_IO0_PORT						GPIOD					// QUADSPI_BK1_IO0 pin port
#define	QUADSPI_BK1_IO0_AF						GPIO_AF9_QUADSPI			// QUADSPI_BK1_IO0 IO port multiplexing
#define GPIO_QUADSPI_BK1_IO0_ENABLE        	                        __HAL_RCC_GPIOD_CLK_ENABLE()	 	// QUADSPI_BK1_IO0 pin clock

#define QUADSPI_BK1_IO1_PIN						GPIO_PIN_12				// QUADSPI_BK1_IO1 pin
#define	QUADSPI_BK1_IO1_PORT						GPIOD					// QUADSPI_BK1_IO1 pin port
#define	QUADSPI_BK1_IO1_AF						GPIO_AF9_QUADSPI			// QUADSPI_BK1_IO1 IO port multiplexing
#define GPIO_QUADSPI_BK1_IO1_ENABLE        	                        __HAL_RCC_GPIOD_CLK_ENABLE()	 	// QUADSPI_BK1_IO1 pin clock

#define QUADSPI_BK1_IO2_PIN						GPIO_PIN_2				// QUADSPI_BK1_IO2 pin
#define	QUADSPI_BK1_IO2_PORT						GPIOE					// QUADSPI_BK1_IO2 pin port
#define	QUADSPI_BK1_IO2_AF						GPIO_AF9_QUADSPI			// QUADSPI_BK1_IO2 IO port multiplexing
#define GPIO_QUADSPI_BK1_IO2_ENABLE        	                        __HAL_RCC_GPIOE_CLK_ENABLE()	 	// QUADSPI_BK1_IO2 pin clock

#define QUADSPI_BK1_IO3_PIN						GPIO_PIN_13				// QUADSPI_BK1_IO3 pin
#define	QUADSPI_BK1_IO3_PORT						GPIOD					// QUADSPI_BK1_IO3 pin port
#define	QUADSPI_BK1_IO3_AF						GPIO_AF9_QUADSPI			// QUADSPI_BK1_IO3 IO port multiplexing
#define GPIO_QUADSPI_BK1_IO3_ENABLE      	                        __HAL_RCC_GPIOD_CLK_ENABLE()	 	// QUADSPI_BK1_IO3 pin clock


/*----------------------------------------------- function declaration ---------------------------------------------------*/

int8_t	QSPI_W25Qxx_Init(void);						// W25Qxx initialization
int8_t 	QSPI_W25Qxx_Reset(void);					// reset device
uint32_t QSPI_W25Qxx_ReadID(void);					// Read Device ID
int8_t 	QSPI_W25Qxx_MemoryMappedMode(void);		                // Enter memory mapped mode
	
int8_t 	QSPI_W25Qxx_SectorErase(uint32_t SectorAddress);		// sector erase£¬4K bytes, refer to erase time 45ms
int8_t 	QSPI_W25Qxx_BlockErase_32K (uint32_t SectorAddress);	        // block erase£¬  32K bytes, refer to erase time120ms
int8_t 	QSPI_W25Qxx_BlockErase_64K (uint32_t SectorAddress);	        // block erase£¬  64KByte, the reference erasing time is 150ms, the actual use is recommended to use 64K erasing, the erasing time is the fastest
int8_t 	QSPI_W25Qxx_ChipErase (void);                                   // Chip erase, refer to erase time 20S

int8_t	QSPI_W25Qxx_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);	//Write in pages, up to 256 bytes
int8_t	QSPI_W25Qxx_WriteBuffer(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);		// Write data, the maximum cannot exceed the size of the flash chip
int8_t 	QSPI_W25Qxx_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);	// Read data, the maximum cannot exceed the size of the flash chip



#endif // QSPI_w25q64_H 





