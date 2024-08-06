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

uint8_t prvbtnstate = 0;

void Button_Process (void)
{
    uint8_t currentlybtnstate = 0;
    
    // Timer 로 처리하는 것을 추천하겠음.
    currentlybtnstate = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);

    if (prvbtnstate != currentlybtnstate)
    {
        prvbtnstate = currentlybtnstate;

        sprintf(uart3_tx_buf, "Button Toggle.\r\n");
        HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);
    }
}




