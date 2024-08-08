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
static void LED_Dimming (void)
{
    if (dutycontrol_flag == 0x00)
    {
        if (TIM1_DutyControl_cnt > 20)
        {
            TIM1_DutyControl_cnt = 0;

            htim12.Instance->CCR1++;
        }
        if (htim12.Instance->CCR1 >= htim12.Init.Period)
        {
            dutycontrol_flag = 0x01;

            // sprintf(uart3_tx_buf, " TIM12 CCR1 : %d\r\n", htim12.Instance->CCR1);
            // HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);
        }
    }
    else if (dutycontrol_flag == 0x01)
    {
        if (TIM1_DutyControl_cnt > 20)
        {
            TIM1_DutyControl_cnt = 0;

            htim12.Instance->CCR1--;
        }
        if (htim12.Instance->CCR1 == 0)
        {
            dutycontrol_flag = 0x00;

            // sprintf(uart3_tx_buf, " TIM12 CCR1 : %d\r\n", );
            // HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);
        }
    }
} 

void LED_Process (void)
{
    //if (TIM1_CNT > 499) // 1ms period Timer2 Counter.
    if (TIM1_CNT >= 500) // 1ms period Timer2 Counter.
    {
        TIM1_CNT = 0;

        HAL_GPIO_TogglePin(GPIOB, LD1_Pin);
        // HAL_GPIO_TogglePin(GPIOB, LD3_Pin);
        //HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    }
    else if (TIM2_CNT >= 10) // 100ms * 9 period Timer2 Counter.
    {
        TIM2_CNT = 0;
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    }
    else if (TIM8_CNT >= 100000) // 10us * 99999 period Timer8 Counter.
    {
        TIM8_CNT = 0;
        //HAL_GPIO_TogglePin(GPIOB, LD3_Pin);
    }

    LED_Dimming();
}
#endif

