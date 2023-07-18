/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#define BUTTON_CONFIGURATION_Pin GPIO_PIN_0
#define BUTTON_CONFIGURATION_GPIO_Port GPIOC
#define BUTTON_CONFIGURATION_EXTI_IRQn EXTI0_IRQn
#define BUTTON_SLIDING_GATE_FRONT_Pin GPIO_PIN_1
#define BUTTON_SLIDING_GATE_FRONT_GPIO_Port GPIOC
#define BUTTON_SLIDING_GATE_FRONT_EXTI_IRQn EXTI1_IRQn
#define BUTTON_SLIDING_GATE_RETRO_Pin GPIO_PIN_2
#define BUTTON_SLIDING_GATE_RETRO_GPIO_Port GPIOC
#define BUTTON_SLIDING_GATE_RETRO_EXTI_IRQn EXTI2_IRQn
#define BUTTON_DOOR_FRONT_Pin GPIO_PIN_3
#define BUTTON_DOOR_FRONT_GPIO_Port GPIOC
#define BUTTON_DOOR_FRONT_EXTI_IRQn EXTI3_IRQn
#define PIR_INTERNAL_LIGHT_Pin GPIO_PIN_1
#define PIR_INTERNAL_LIGHT_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define BUTTON_DOOR_RETRO_Pin GPIO_PIN_4
#define BUTTON_DOOR_RETRO_GPIO_Port GPIOA
#define BUTTON_DOOR_RETRO_EXTI_IRQn EXTI4_IRQn
#define NUMPAD_C3_Pin GPIO_PIN_6
#define NUMPAD_C3_GPIO_Port GPIOA
#define SG90_Pin GPIO_PIN_7
#define SG90_GPIO_Port GPIOA
#define NUMPAD_R2_Pin GPIO_PIN_5
#define NUMPAD_R2_GPIO_Port GPIOC
#define ACTIVE_BUZZER_Pin GPIO_PIN_0
#define ACTIVE_BUZZER_GPIO_Port GPIOB
#define STEPPER_MOTOR_IN4_Pin GPIO_PIN_1
#define STEPPER_MOTOR_IN4_GPIO_Port GPIOB
#define PHOTO_INTERRUPTER_Pin GPIO_PIN_10
#define PHOTO_INTERRUPTER_GPIO_Port GPIOB
#define PHOTO_INTERRUPTER_EXTI_IRQn EXTI15_10_IRQn
#define STEPPER_MOTOR_IN1_Pin GPIO_PIN_13
#define STEPPER_MOTOR_IN1_GPIO_Port GPIOB
#define STEPPER_MOTOR_IN2_Pin GPIO_PIN_14
#define STEPPER_MOTOR_IN2_GPIO_Port GPIOB
#define STEPPER_MOTOR_IN3_Pin GPIO_PIN_15
#define STEPPER_MOTOR_IN3_GPIO_Port GPIOB
#define NUMPAD_R3_Pin GPIO_PIN_6
#define NUMPAD_R3_GPIO_Port GPIOC
#define HC_SR04_OPEN_POSITION_TRIGGER_Pin GPIO_PIN_7
#define HC_SR04_OPEN_POSITION_TRIGGER_GPIO_Port GPIOC
#define NUMPAD_C0_Pin GPIO_PIN_8
#define NUMPAD_C0_GPIO_Port GPIOC
#define NUMPAD_C1_Pin GPIO_PIN_9
#define NUMPAD_C1_GPIO_Port GPIOC
#define HC_SR04_OPEN_POSITION_ECO_Pin GPIO_PIN_8
#define HC_SR04_OPEN_POSITION_ECO_GPIO_Port GPIOA
#define NUMPAD_R0_Pin GPIO_PIN_11
#define NUMPAD_R0_GPIO_Port GPIOA
#define NUMPAD_R1_Pin GPIO_PIN_12
#define NUMPAD_R1_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define LED_INTERNAL_LIGHT_Pin GPIO_PIN_10
#define LED_INTERNAL_LIGHT_GPIO_Port GPIOC
#define LED_ALARM_Pin GPIO_PIN_11
#define LED_ALARM_GPIO_Port GPIOC
#define LED_SLIDING_GATE_Pin GPIO_PIN_12
#define LED_SLIDING_GATE_GPIO_Port GPIOC
#define LED_CONFIGURATION_Pin GPIO_PIN_2
#define LED_CONFIGURATION_GPIO_Port GPIOD
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define HC_SR04_CLOSE_POSITION_TRIGGER_Pin GPIO_PIN_5
#define HC_SR04_CLOSE_POSITION_TRIGGER_GPIO_Port GPIOB
#define NUMPAD_C2_Pin GPIO_PIN_8
#define NUMPAD_C2_GPIO_Port GPIOB
#define HC_SR04_CLOSE_POSITION_ECO_Pin GPIO_PIN_9
#define HC_SR04_CLOSE_POSITION_ECO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
