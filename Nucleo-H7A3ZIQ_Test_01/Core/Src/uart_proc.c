/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    uart_proc.c
  * @brief   This file provides code for the configuration
  *          of uart3.
  ******************************************************************************
  * @attention
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "uart_proc.h"


void Uart3_Process (void)
{
    if ((uart3_rx_buf[uart3_rx_index - 1] == 0x0A))
    {
        if (!strncmp("RXBUF ALL PRINT", uart3_rx_buf, UART3_CMP_SIZE))
        {
            sprintf(uart3_tx_buf, "In A\r\n");
            HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);
        }
        else if (!strncmp("RXBUF CLEAR", uart3_rx_buf, UART3_CMP_SIZE))
        {
            sprintf(uart3_tx_buf, "In B\r\n");
            HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);
        }
        else if (!strncmp("SYSTEM RESET", uart3_rx_buf, UART3_CMP_SIZE))
        {
            sprintf(uart3_tx_buf, "STM32H7A3 System Reset.\r\n");
            HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);
            HAL_NVIC_SystemReset();
        }

        //memset(uart3_rx_buf, 0, uart3_rx_index);
        uart3_rx_index = 0;
    }
    
    else if (uart3_rx_index >= 999)
    {
        uart3_rx_index = 0;
    }
}




