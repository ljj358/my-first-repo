/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#define TIM_PERIOD_VALUE 50000
#define LORA_IRQ_TX_Pin GPIO_PIN_4
#define LORA_IRQ_TX_GPIO_Port GPIOE
#define MCU_DO1_Pin GPIO_PIN_5
#define MCU_DO1_GPIO_Port GPIOE
#define LORA_BUSY_TX_Pin GPIO_PIN_6
#define LORA_BUSY_TX_GPIO_Port GPIOE
#define RUN_LED_Pin GPIO_PIN_13
#define RUN_LED_GPIO_Port GPIOC
#define MCU_RELAY2_Pin GPIO_PIN_0
#define MCU_RELAY2_GPIO_Port GPIOC
#define MCU_BUZZER_Pin GPIO_PIN_4
#define MCU_BUZZER_GPIO_Port GPIOA
#define MCU_DO4_Pin GPIO_PIN_6
#define MCU_DO4_GPIO_Port GPIOA
#define MCU_DO3_Pin GPIO_PIN_1
#define MCU_DO3_GPIO_Port GPIOB
#define MCU_RELAY1_Pin GPIO_PIN_7
#define MCU_RELAY1_GPIO_Port GPIOE
#define TM_SCL_Pin GPIO_PIN_8
#define TM_SCL_GPIO_Port GPIOE
#define TM_SDA_Pin GPIO_PIN_9
#define TM_SDA_GPIO_Port GPIOE
#define KEY_Pin GPIO_PIN_10
#define KEY_GPIO_Port GPIOE
#define SPI_SCS_Pin GPIO_PIN_15
#define SPI_SCS_GPIO_Port GPIOE
#define WZ_INT_Pin GPIO_PIN_8
#define WZ_INT_GPIO_Port GPIOD
#define WZ_RST_Pin GPIO_PIN_9
#define WZ_RST_GPIO_Port GPIOD
#define VUSB_Pin GPIO_PIN_10
#define VUSB_GPIO_Port GPIOD
#define LORA_IRQ_RX_Pin GPIO_PIN_14
#define LORA_IRQ_RX_GPIO_Port GPIOD
#define LORA_BUSY_RX_Pin GPIO_PIN_15
#define LORA_BUSY_RX_GPIO_Port GPIOD
#define USB_CTRL_Pin GPIO_PIN_9
#define USB_CTRL_GPIO_Port GPIOC
#define LORA_CS_RX_Pin GPIO_PIN_8
#define LORA_CS_RX_GPIO_Port GPIOA
#define UART_CTRL_Pin GPIO_PIN_9
#define UART_CTRL_GPIO_Port GPIOA
#define GNSS_PPS_Pin GPIO_PIN_10
#define GNSS_PPS_GPIO_Port GPIOA
#define LORA_RST_RX_Pin GPIO_PIN_15
#define LORA_RST_RX_GPIO_Port GPIOA
#define LORA_CS_TX_Pin GPIO_PIN_5
#define LORA_CS_TX_GPIO_Port GPIOD
#define M4G_RST_Pin GPIO_PIN_3
#define M4G_RST_GPIO_Port GPIOB
#define M4G_PWR_KEY_Pin GPIO_PIN_4
#define M4G_PWR_KEY_GPIO_Port GPIOB
#define M4G_STATUS_Pin GPIO_PIN_5
#define M4G_STATUS_GPIO_Port GPIOB
#define LORA_RST_TX_Pin GPIO_PIN_8
#define LORA_RST_TX_GPIO_Port GPIOB
#define MCU_DO2_Pin GPIO_PIN_9
#define MCU_DO2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
