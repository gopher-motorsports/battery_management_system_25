/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "telemetryTask.h"
#include "statusUpdateTask.h"

#define TELEMETRY_TASK_PERIOD_MS      20
#define PRINT_TASK_PERIOD_MS          1000
#define STATUS_UPDATE_TASK_PERIOD_MS  10

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define AMS_FAULT_N_Pin GPIO_PIN_14
#define AMS_FAULT_N_GPIO_Port GPIOC
#define MCU_GSENSE_Pin GPIO_PIN_0
#define MCU_GSENSE_GPIO_Port GPIOC
#define MCU_FAULT_Pin GPIO_PIN_1
#define MCU_FAULT_GPIO_Port GPIOC
#define MCU_HEARTBEAT_Pin GPIO_PIN_2
#define MCU_HEARTBEAT_GPIO_Port GPIOC
#define PORTA_CS_Pin GPIO_PIN_4
#define PORTA_CS_GPIO_Port GPIOA
#define ISO_SPI_CLK_Pin GPIO_PIN_5
#define ISO_SPI_CLK_GPIO_Port GPIOA
#define ISO_SPI_MISO_Pin GPIO_PIN_6
#define ISO_SPI_MISO_GPIO_Port GPIOA
#define ISO_SPI_MOSI_Pin GPIO_PIN_7
#define ISO_SPI_MOSI_GPIO_Port GPIOA
#define PORTB_CS_Pin GPIO_PIN_4
#define PORTB_CS_GPIO_Port GPIOC
#define AMS_INB_N_Pin GPIO_PIN_7
#define AMS_INB_N_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

extern telemetryTaskData_S telemetryTaskData;
extern statusUpdateTaskData_S statusUpdateTaskData;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
