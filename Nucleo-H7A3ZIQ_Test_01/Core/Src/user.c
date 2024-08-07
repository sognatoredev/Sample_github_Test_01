




/*******************************************************************************
 * INCLUDE FILES
 *******************************************************************************/
/*---- standard files --------------------------------------------------------*/
#include "user.h"



/*******************************************************************************
 * EXTERNAL REFERENCES             NOTE: only use if not available in header file
 *******************************************************************************/
/*---- function prototypes ---------------------------------------------------*/
/*---- data declarations -----------------------------------------------------*/

/*******************************************************************************
 * PUBLIC DECLARATIONS             Defined here, used elsewhere
 *******************************************************************************/
/*---- context ---------------------------------------------------------------*/
/*---- function prototypes ---------------------------------------------------*/
/*---- data declarations -----------------------------------------------------*/
#define UART3_BUFFER_LENGTH 1024

uint32_t TIM1_CNT = 0; // 1ms period Timer1 Counter.
uint32_t TIM1_DutyControl_cnt = 0; // 1ms period Timer1 Counter.
uint32_t TIM2_CNT = 0; // 100ms period Timer2 Counter.
uint32_t TIM8_CNT = 0; // ms period Timer8 Counter.

uint8_t uart3_rx_buf[UART3_BUFFER_LENGTH] = {0};
uint8_t uart3_tx_buf[UART3_BUFFER_LENGTH] = {0};

uint16_t userButton_cnt = 0;
uint8_t userButton_state = 0x00;

uint8_t dutycontrol_flag = 0;


/*******************************************************************************
 * PRIVATE DECLARATIONS            Defined here, used elsewhere
 *******************************************************************************/



/*******************************************************************************
 * PUBLIC FUNCTION DEFINITIONS
 *******************************************************************************/
/* Display Boot Message */
void BootMessagePrint (void)
{
    sprintf(uart3_tx_buf, "-------------------------------------------\r\n");
    HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);

    sprintf(uart3_tx_buf, " Project Name        : %s\r\n", PROJECT_NAME );
    HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);

    sprintf(uart3_tx_buf, " - HW VERSION        : %s\r\n", STR_HW_VER );
    HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);

    sprintf(uart3_tx_buf, " - FW VERSION        : %s\r\n", STR_FW_VER );
    HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);

    sprintf(uart3_tx_buf, " - BUILD TIME        : %s, %s\r\n", __DATE__, __TIME__ );
    HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);

    sprintf(uart3_tx_buf, "-------------------------------------------\r\n" );
    HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);
}

/* Display Clcok Source Message */
void GetClockSourcePrint (void)
{
    sprintf(uart3_tx_buf, "-------------------------------------------\r\n");
    HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);

    sprintf(uart3_tx_buf, " - SYSTEM Clock Frequency        : %lu MHz\r\n", (HAL_RCC_GetSysClockFreq() /1000000));
    HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);

    sprintf(uart3_tx_buf, " - HCLK Clock   Frequency        : %lu MHz\r\n", (HAL_RCC_GetHCLKFreq() / 1000000));
    HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);

    sprintf(uart3_tx_buf, " - PCLK1 Clock  Frequency        : %lu MHz\r\n", (HAL_RCC_GetPCLK1Freq() / 1000000));
    HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);

    sprintf(uart3_tx_buf, " - PCLK2 Clock  Frequency        : %lu MHz\r\n", (HAL_RCC_GetPCLK2Freq() / 1000000));
    HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);

    sprintf(uart3_tx_buf, "-------------------------------------------\r\n" );
    HAL_UART_Transmit(&huart3, (uint8_t *) uart3_tx_buf, strlen(uart3_tx_buf), HAL_MAX_DELAY);
}


/*******************************************************************************
 * PRIVATE FUNCTION DEFINITIONS
 *******************************************************************************/


