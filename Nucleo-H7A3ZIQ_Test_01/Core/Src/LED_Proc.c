/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    led_proc.c
  * @brief   This file provides code for the configuration
  *          of all used led pins.
  ******************************************************************************
  * @attention

  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "led_proc.h"

#if 0
void LED_Process (void)
{
    HAL_Delay(500);
    HAL_GPIO_TogglePin(GPIOB, LD1_Pin);
    HAL_GPIO_TogglePin(GPIOB, LD3_Pin);
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}
#else
void LED_Process (void)
{
  if (TIM1_CNT > 499) // 1ms period Timer2 Counter.
  {
    TIM1_CNT = 0;

    HAL_GPIO_TogglePin(GPIOB, LD1_Pin);
    // HAL_GPIO_TogglePin(GPIOB, LD3_Pin);
    //HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  }
  else if (TIM2_CNT > 9) // 100ms * 9 period Timer2 Counter.
  {
    TIM2_CNT = 0;
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  }
  else if (TIM8_CNT > 99999) // 10us * 99999 period Timer8 Counter.
  {
    TIM8_CNT = 0;
    HAL_GPIO_TogglePin(GPIOB, LD3_Pin);
  }
}
#endif

