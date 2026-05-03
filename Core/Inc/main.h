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
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

// These HAL device handles are declared and initialized in the various
// CubeMX auto-generated IP init source files (e.g. tim.c, i2c.h, usart.c, etc.)
// and also exported by their corresponding .h files.
// They are exported here in main.h as well as a convenience - the application
// will need to reference these device handles at times, but will likely not
// need to utilize the init functions provided by the IP initialization modules.
// Providing the extern definitions for the device handles here avoids the need
// for the application modules to #include multiple core .h files - it only
// needs to #include "main.h" to gain access to all of the device handles.

//extern TIM_HandleTypeDef htim1;
//extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
//extern TIM_HandleTypeDef htim16;
//extern TIM_HandleTypeDef htim17;
extern RTC_HandleTypeDef hrtc;
//extern IWDG_HandleTypeDef hiwdg;
//extern I2C_HandleTypeDef hi2c1;
//extern SPI_HandleTypeDef hspi2;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart3_tx;
extern DMA_HandleTypeDef hdma_usart4_tx;
extern SPI_HandleTypeDef hspi1;
//extern CRC_HandleTypeDef hcrc;
//extern ADC_HandleTypeDef hadc1;

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

#define DEBUG_UART_HANDLE   huart2

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define NUCLEO_BUTTON_Pin GPIO_PIN_13
#define NUCLEO_BUTTON_GPIO_Port GPIOC
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOF
#define LED_STRIP_3_TX_Pin GPIO_PIN_0
#define LED_STRIP_3_TX_GPIO_Port GPIOA
#define DEBUG_TX_Pin GPIO_PIN_2
#define DEBUG_TX_GPIO_Port GPIOA
#define DEBUG_RX_Pin GPIO_PIN_3
#define DEBUG_RX_GPIO_Port GPIOA
#define NUCLEO_LED_Pin GPIO_PIN_5
#define NUCLEO_LED_GPIO_Port GPIOA
#define LED_STRIP_1_TX_Pin GPIO_PIN_4
#define LED_STRIP_1_TX_GPIO_Port GPIOC
#define LED_STRIP_2_TX_Pin GPIO_PIN_2
#define LED_STRIP_2_TX_GPIO_Port GPIOB
#define SWDATA_Pin GPIO_PIN_13
#define SWDATA_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
