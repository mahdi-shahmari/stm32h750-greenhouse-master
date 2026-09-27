/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_LED_Pin GPIO_PIN_0
#define LCD_LED_GPIO_Port GPIOB
#define Power_1_Pin GPIO_PIN_7
#define Power_1_GPIO_Port GPIOE
#define Power_2_Pin GPIO_PIN_8
#define Power_2_GPIO_Port GPIOE
#define Power_3_Pin GPIO_PIN_9
#define Power_3_GPIO_Port GPIOE
#define LCD_RST_Pin GPIO_PIN_10
#define LCD_RST_GPIO_Port GPIOE
#define LCD_CS_Pin GPIO_PIN_11
#define LCD_CS_GPIO_Port GPIOE
#define LCD_RS_Pin GPIO_PIN_15
#define LCD_RS_GPIO_Port GPIOE
#define Power_4_Pin GPIO_PIN_14
#define Power_4_GPIO_Port GPIOB
#define Power_5_Pin GPIO_PIN_15
#define Power_5_GPIO_Port GPIOB
#define Power_6_Pin GPIO_PIN_14
#define Power_6_GPIO_Port GPIOD
#define NRF_CE_Pin GPIO_PIN_0
#define NRF_CE_GPIO_Port GPIOE
#define NRF_CSN_Pin GPIO_PIN_1
#define NRF_CSN_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
