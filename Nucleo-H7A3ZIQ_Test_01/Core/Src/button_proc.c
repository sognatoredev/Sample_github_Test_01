/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    button_proc.c
  * @brief   This file provides code for the configuration
  *          of all used button pins.
  ******************************************************************************
  * @attention
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "button_proc.h"


#if 0
void LED_Process (void)
{
    HAL_Delay(500);
    HAL_GPIO_TogglePin(GPIOB, LD1_Pin);
    HAL_GPIO_TogglePin(GPIOB, LD3_Pin);
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}
#else
void Button_Process (void)
{
    if (GPIO_PIN_SET == HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin))
    {
        sprintf(uart3_tx_buf, "Button State : Pushed.\r\n");
        HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);
    }
}
#endif

