#include "delay.h"

static u32 fac_us = 0;

// Initializes the delay driver parameters
void delay_init(u16 SYSCLK)
{
  // For H7, SysTick clock is typically HCLK (400MHz)
  fac_us = SYSCLK; 
}

// Microsecond delay loop using SysTick counter value tracking
void delay_us(u32 nus)
{        
  uint32_t ticks;
  uint32_t told, tnow, tcnt = 0;
  uint32_t reload = SysTick->LOAD;                 
  
  ticks = nus * fac_us;                     
  told = SysTick->VAL;                        
  while(1)
  {
    tnow = SysTick->VAL;    
    if(tnow != told)
    {        
      if(tnow < told) {
        tcnt += told - tnow;    
      } else {
        tcnt += reload - tnow + told;        
      }
      told = tnow;
      if(tcnt >= ticks) break;            
    }  
  };
}

// Millisecond delay wrapper routing down to your hardware microsecond loop
void delay_ms(u16 nms)
{    
  delay_us((uint32_t)(nms * 1000));            
}