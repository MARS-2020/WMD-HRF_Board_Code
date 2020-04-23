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
#include "stm32l0xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define HR_MFIO_Pin GPIO_PIN_15
#define HR_MFIO_GPIO_Port GPIOC
#define dim_Pin GPIO_PIN_0
#define dim_GPIO_Port GPIOA
#define dim_EXTI_IRQn EXTI0_1_IRQn
#define LORA_NSS_Pin GPIO_PIN_4
#define LORA_NSS_GPIO_Port GPIOA
#define LORA_RST_Pin GPIO_PIN_0
#define LORA_RST_GPIO_Port GPIOB
#define LORA_DIO_Pin GPIO_PIN_1
#define LORA_DIO_GPIO_Port GPIOB
#define LORA_DIO_EXTI_IRQn EXTI0_1_IRQn
#define oled_NSS_Pin GPIO_PIN_12
#define oled_NSS_GPIO_Port GPIOB
#define oled_DC_Pin GPIO_PIN_14
#define oled_DC_GPIO_Port GPIOB
#define oled_RES_Pin GPIO_PIN_8
#define oled_RES_GPIO_Port GPIOA
#define HR_RESET_Pin GPIO_PIN_5
#define HR_RESET_GPIO_Port GPIOB
#define DEBUG_Pin GPIO_PIN_8
#define DEBUG_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
