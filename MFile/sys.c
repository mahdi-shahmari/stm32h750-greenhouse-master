#include "sys.h"

void Cache_Enable(void)
{
    SCB_EnableICache();
    SCB_EnableDCache();
	SCB->CACR|=1<<2;   
}


#ifdef  USE_FULL_ASSERT



void assert_failed(uint8_t* file, uint32_t line)
{ 
	while (1)
	{
	}
}


#endif



u8 Get_ICahceSta(void)
{
    u8 sta;
    sta=((SCB->CCR)>>17)&0X01;
    return sta;
}



u8 Get_DCahceSta(void)
{
    u8 sta;
    sta=((SCB->CCR)>>16)&0X01;
    return sta;
}



void QSPI_Enable_Memmapmode(void)
{
	u32 tempreg=0; 
	vu32 *data_reg=&QUADSPI->DR;
	GPIO_InitTypeDef qspi_gpio;
	
	RCC->AHB4ENR|=1<<1;    					
	RCC->AHB4ENR|=1<<5;    						  
	RCC->AHB3ENR|=1<<14;   						

	qspi_gpio.Pin=GPIO_PIN_6;					
	qspi_gpio.Mode=GPIO_MODE_AF_PP;
	qspi_gpio.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
	qspi_gpio.Pull=GPIO_NOPULL;
	qspi_gpio.Alternate=GPIO_AF10_QUADSPI;
	HAL_GPIO_Init(GPIOB,&qspi_gpio);
	
	qspi_gpio.Pin=GPIO_PIN_2;				
	qspi_gpio.Alternate=GPIO_AF9_QUADSPI;
	HAL_GPIO_Init(GPIOB,&qspi_gpio);
	
	qspi_gpio.Pin=GPIO_PIN_6|GPIO_PIN_7;		
	qspi_gpio.Alternate=GPIO_AF9_QUADSPI;
	HAL_GPIO_Init(GPIOF,&qspi_gpio);
	
	qspi_gpio.Pin=GPIO_PIN_8|GPIO_PIN_9;			
	qspi_gpio.Alternate=GPIO_AF10_QUADSPI;
	HAL_GPIO_Init(GPIOF,&qspi_gpio);
	
	RCC->AHB3RSTR|=1<<14;			
	RCC->AHB3RSTR&=~(1<<14);		
	while(QUADSPI->SR&(1<<5));	
	QUADSPI->CR=0X01000310;			
	QUADSPI->DCR=0X00160401;		
	QUADSPI->CR|=1<<0;				


	while(QUADSPI->SR&(1<<5));		
	QUADSPI->CCR=0X00000138;	
	while((QUADSPI->SR&(1<<1))==0);	
	QUADSPI->FCR|=1<<1;					

	
	while(QUADSPI->SR&(1<<5));	
	QUADSPI->CCR=0X00000106;		
	while((QUADSPI->SR&(1<<1))==0);	
	QUADSPI->FCR|=1<<1;				
	
 	while(QUADSPI->SR&(1<<5));		
	QUADSPI->CCR=0X030003C0;		
	QUADSPI->DLR=0;
	while((QUADSPI->SR&(1<<2))==0);
	*(vu8 *)data_reg=3<<4;			
	QUADSPI->CR|=1<<2;				
	while((QUADSPI->SR&(1<<1))==0);
	QUADSPI->FCR|=1<<1;			
	while(QUADSPI->SR&(1<<5));		 

	while(QUADSPI->SR&(1<<5));		
	QUADSPI->ABR=0;				
	tempreg=0XEB;					
	tempreg|=3<<8;					
	tempreg|=3<<10;				
	tempreg|=2<<12;				
	tempreg|=3<<14;					
	tempreg|=0<<16;					
	tempreg|=6<<18;					
	tempreg|=3<<24;					
	tempreg|=3<<26;					
	QUADSPI->CCR=tempreg;			
	
	SCB->SHCSR&=~(1<<16);			
	MPU->CTRL&=~(1<<0);				
	MPU->RNR=0;						
	MPU->RBAR=0X90000000;			
	MPU->RASR=0X0303002D;			
	MPU->CTRL=(1<<2)|(1<<0);		
	SCB->SHCSR|=1<<16;			
}



#if defined(__clang__) 
void __attribute__((noinline)) WFI_SET(void)
{
    __asm__("wfi");
}

void __attribute__((noinline)) INTX_DISABLE(void)
{
    __asm__("cpsid i \t\n"
            "bx lr");
}

void __attribute__((noinline)) INTX_ENABLE(void)
{
    __asm__("cpsie i \t\n"
            "bx lr");
}

void __attribute__((noinline)) MSR_MSP(u32 addr) 
{
    __asm__("msr msp, r0 \t\n"
            "bx r14");
}
#elif defined (__CC_ARM)    

__asm void WFI_SET(void)
{
	WFI;		  
}
__asm void INTX_DISABLE(void)
{
	CPSID   I
	BX      LR	  
}
__asm void INTX_ENABLE(void)
{
	CPSIE   I
	BX      LR  
}
__asm void MSR_MSP(u32 addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}
#endif
















/********************************************************************************/
/*                             www.kavirElectronic.ir                                    */
/******************************************************************************/



