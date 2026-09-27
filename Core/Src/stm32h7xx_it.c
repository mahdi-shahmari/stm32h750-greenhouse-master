/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32h7xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32h7xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "BT_RTC_PWM.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HardFault_Handler_C(uint32_t *hardfault_args)
{
    volatile uint32_t stacked_pc = hardfault_args[6];
    volatile uint32_t cfsr = SCB->CFSR;
    (void)stacked_pc; (void)cfsr;
    while(1) { __NOP(); }   // breakpoint here
}
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_spi4_tx;
extern SPI_HandleTypeDef hspi4;
extern UART_HandleTypeDef huart1;
/* USER CODE BEGIN EV */
/* Global variables to store fault info (visible in debugger) */
volatile uint32_t fault_register_cfsr;
volatile uint32_t fault_register_hfsr;
volatile uint32_t fault_register_mmfar;
volatile uint32_t fault_register_bfar;
volatile uint32_t fault_pc_value;
volatile uint32_t fault_lr_value;
volatile uint32_t fault_sp_value;
volatile uint32_t fault_xpsr_value;


/* C handler with detailed analysis */
void MemManage_Handler_C(uint32_t *memmanage_args)
{
    /* Save all stacked registers for analysis */
    volatile uint32_t stacked_r0  = memmanage_args[0];
    volatile uint32_t stacked_r1  = memmanage_args[1];
    volatile uint32_t stacked_r2  = memmanage_args[2];
    volatile uint32_t stacked_r3  = memmanage_args[3];
    volatile uint32_t stacked_r12 = memmanage_args[4];
    volatile uint32_t stacked_lr  = memmanage_args[5];
    volatile uint32_t stacked_pc  = memmanage_args[6];
    volatile uint32_t stacked_xpsr = memmanage_args[7];
    
    /* Get fault registers */
    uint32_t cfsr = SCB->CFSR;
    uint32_t hfsr = SCB->HFSR;
    uint32_t mmfar = SCB->MMFAR;
    uint32_t bfar = SCB->BFAR;
    
    /* Store in global variables for debugger */
    fault_register_cfsr = cfsr;
    fault_register_hfsr = hfsr;
    fault_register_mmfar = mmfar;
    fault_register_bfar = bfar;
    fault_pc_value = stacked_pc;
    fault_lr_value = stacked_lr;
    fault_xpsr_value = stacked_xpsr;
    
    /* Determine current SP based on EXC_RETURN */
    uint32_t sp;
    
    /* Check which stack was in use */
    if (__get_LR() & 0x4) {
        sp = __get_PSP();  /* Using Process Stack */
    } else {
        sp = __get_MSP();  /* Using Main Stack */
    }
    
    /* Analyze and indicate fault reason (using LED) */
    #ifdef FAULT_LED_PORT
    /* Determine fault type and blink pattern */
    uint32_t blink_count = 0;
    uint32_t blink_speed = 200;  /* ms */
    
    if (cfsr & (1 << 0)) {  /* IACCVIOL */
        blink_count = 1;
        blink_speed = 200;
    }
    else if (cfsr & (1 << 1)) {  /* DACCVIOL */
        blink_count = 2;
        blink_speed = 200;
    }
    else if (cfsr & (1 << 2)) {  /* UNALIGNED */
        blink_count = 3;
        blink_speed = 200;
    }
    else if (cfsr & (1 << 3)) {  /* BUSFAULT on data */
        blink_count = 4;
        blink_speed = 200;
    }
    else if (cfsr & (1 << 4)) {  /* MSTKERR - STACK OVERFLOW! */
        /* Fast blinking for stack overflow */
        while(1) {
            HAL_GPIO_TogglePin(FAULT_LED_PORT, FAULT_LED_PIN);
            HAL_Delay(50);
        }
    }
    else if (cfsr & (1 << 5)) {  /* MUNSTKERR - Unstacking error */
        blink_count = 5;
        blink_speed = 200;
    }
    else if (cfsr & (1 << 7)) {  /* MLSPERR - Lazy FPU stacking error */
        blink_count = 6;
        blink_speed = 200;
    }
    else {
        blink_count = 10;  /* Unknown fault */
        blink_speed = 100;
    }
    
    /* Blink the pattern */
    for(int i = 0; i < blink_count; i++) {
        HAL_GPIO_WritePin(FAULT_LED_PORT, FAULT_LED_PIN, GPIO_PIN_SET);
        HAL_Delay(blink_speed);
        HAL_GPIO_WritePin(FAULT_LED_PORT, FAULT_LED_PIN, GPIO_PIN_RESET);
        HAL_Delay(blink_speed);
    }
    /* Long pause before repeating */
    HAL_Delay(2000);
    #endif
    
    /* Infinite loop - place breakpoint here */
    while(1) {
        __NOP();  /* SET BREAKPOINT HERE IN DEBUGGER */
    }
}

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
    __asm volatile
    (
        "TST LR, #4                    \n"
        "ITE EQ                        \n"
        "MRSEQ R0, MSP                 \n"
        "MRSNE R0, PSP                 \n"
        "B HardFault_Handler_C         \n"
    );
  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */
  __asm volatile
    (
     "TST LR, #4                    \n"  /* Check EXC_RETURN bit 2 */
       "ITE EQ                        \n"  /* If EQ (bit 2 = 0), use MSP */
         "MRSEQ R0, MSP                 \n"  /* Move MSP to R0 */
           "MRSNE R0, PSP                 \n"  /* Else use PSP */
             "B MemManage_Handler_C         \n"  /* Branch to C handler */
               );
  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
//  lv_tick_inc(1);
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 stream0 global interrupt.
  */
void DMA1_Stream0_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream0_IRQn 0 */
  /* USER CODE END DMA1_Stream0_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_spi4_tx);
  /* USER CODE BEGIN DMA1_Stream0_IRQn 1 */

  /* USER CODE END DMA1_Stream0_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles SPI4 global interrupt.
  */
void SPI4_IRQHandler(void)
{
  /* USER CODE BEGIN SPI4_IRQn 0 */

  /* USER CODE END SPI4_IRQn 0 */
  HAL_SPI_IRQHandler(&hspi4);
  /* USER CODE BEGIN SPI4_IRQn 1 */

  /* USER CODE END SPI4_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
