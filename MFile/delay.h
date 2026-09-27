#ifndef _DELAY_H
#define _DELAY_H

#include "stm32h7xx_hal.h"
#include "sys.h"
#include <stdint.h>
// Define missing legacy types natively 

void delay_init(u16 SYSCLK);
void delay_ms(u16 nms);
void delay_us(u32 nus);

#endif