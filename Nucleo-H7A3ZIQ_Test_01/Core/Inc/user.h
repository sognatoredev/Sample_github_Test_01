
#ifndef __USER_H__
#define __USER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdio.h>


extern uint32_t TIM1_CNT;
extern uint32_t TIM2_CNT;

extern uint8_t uart3_rx_buf[];
extern uint8_t uart3_tx_buf[];

#ifdef __cplusplus
}
#endif
#endif /*__ USER_H__ */

