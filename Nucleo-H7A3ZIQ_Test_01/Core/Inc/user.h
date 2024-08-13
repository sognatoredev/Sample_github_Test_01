
#ifndef __USER_H__
#define __USER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "tim.h"

extern uint32_t TIM1_CNT;
extern uint32_t TIM1_DutyControl_cnt;
extern uint32_t TIM2_CNT;
extern uint32_t TIM8_CNT;

extern uint8_t uart3_rx_buf[];
extern uint8_t uart3_tx_buf[];
extern uint8_t uart3_rx_dummy[];
extern uint8_t uart3_tx_dummy[];

extern uint16_t uart3_rx_index;
extern uint16_t uart3_tx_index;

extern uint16_t uart3_rx_cnt;
extern uint16_t uart3_tx_cnt;

extern uint16_t userButton_cnt;
extern uint8_t userButton_state;

extern uint8_t dutycontrol_flag;

#define UART3_CMP_SIZE           (uart3_rx_index - 2)

#ifndef DEBUG
#define DEBUG
#endif
#ifdef DEBUG
#define CONCAT_3(p1, p2, p3)  CONCAT_3_(p1, p2, p3)
/** Auxiliary macro used by @ref CONCAT_3 */
#define CONCAT_3_(p1, p2, p3) p1##p2##p3

#define STRINGIFY_(val) #val
/** Converts a macro argument into a character constant.*/
#define STRINGIFY(val)  STRINGIFY_(val)

/* FW and Board Information */
#define PROJECT_NAME                "STM32H7A3 TEST"

/* board revision */
#define BD_VER_MAJOR                0
#define BD_VER_MINOR                1
/* firmware version */
#define FW_VER_MAJOR                0
#define FW_VER_MINOR                1

/* board revision */
#define HW_BD_REV_01    ( ( 0 << 4 ) | ( 1 ) )
#define STR_BD_VER      "v" STRINGIFY( BD_VER_MAJOR ) "." STRINGIFY( BD_VER_MINOR )
#define DEF_BD_VER      CONCAT_3( HW_BD_REV_01, BD_VER_MAJOR, BD_VER_MINOR )
/* firmware version */
#define STR_FW_VER                      "v" STRINGIFY( FW_VER_MAJOR ) "." STRINGIFY( FW_VER_MINOR ) "." STRINGIFY( FW_VER_PATCH )
#define STR_FW_VER_LEN                  6
#define STR_HW_VER                      "v" STRINGIFY( BD_VER_MAJOR ) "." STRINGIFY( BD_VER_MINOR )
#endif

extern void BootMessagePrint (void);
extern void GetClockSourcePrint (void);

#ifdef __cplusplus
}
#endif
#endif /*__ USER_H__ */

