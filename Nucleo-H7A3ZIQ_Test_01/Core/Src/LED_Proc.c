/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "LED_Proc.h"


void LED_Process (void)
{
    HAL_Delay(500);
    HAL_GPIO_TogglePin(GPIOB, LD1_Pin);
    HAL_GPIO_TogglePin(GPIOB, LD3_Pin);
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

    /*Configure GPIO pin Output Level */
    //HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    //HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

}
