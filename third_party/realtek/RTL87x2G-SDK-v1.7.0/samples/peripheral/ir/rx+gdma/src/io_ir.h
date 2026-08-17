/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __IO_IR_H
#define __IO_IR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl_ir.h"
#include "rtl_nvic.h"
#include "rtl_pinmux.h"
#include "rtl_rcc.h"
#include "rtl_gdma.h"
#include "rtl_tim.h"

#include "board.h"

#include "app_msg.h"

#define IR_RX_PIN                               P2_5
#define IR_RX_FIFO_THR_LEVEL                    30
#define IR_DATA_SIZE_MAX                        80
#define IO_TEST_GDMA_TRANSFER_SIZE              IR_DATA_SIZE_MAX

#define PWM_OUT_PIN                             P2_6//Used for IR sampling output, for debug
#define PWM_OUT_PINMUX                          TIMER_PWM5
#define PWM_TIMER_NUM                           TIM5
#define PWM_HIGH_COUNT                          (20000 - 1)
#define PWM_LOW_COUNT                           (20000 - 1)

#define IO_TEST_GDMA_CHANNEL_MUM                1

#if (IO_TEST_GDMA_CHANNEL_MUM == 0)
#define IO_TEST_GDMA_Channel                GDMA_Channel0
#define IO_TEST_GDMA_Channel_IRQn           GDMA0_Channel0_IRQn
#define IO_TEST_GDMA_Channel_Handler        GDMA0_Channel0_Handler
#elif IO_TEST_GDMA_CHANNEL_MUM == 1
#define IO_TEST_GDMA_Channel                GDMA_Channel1
#define IO_TEST_GDMA_Channel_IRQn           GDMA0_Channel1_IRQn
#define IO_TEST_GDMA_Channel_Handler        GDMA0_Channel1_Handler
#elif IO_TEST_GDMA_CHANNEL_MUM == 2
#define IO_TEST_GDMA_Channel                GDMA_Channel2
#define IO_TEST_GDMA_Channel_IRQn           GDMA0_Channel2_IRQn
#define IO_TEST_GDMA_Channel_Handler        GDMA0_Channel2_Handler
#elif IO_TEST_GDMA_CHANNEL_MUM == 3
#define IO_TEST_GDMA_Channel                GDMA_Channel3
#define IO_TEST_GDMA_Channel_IRQn           GDMA0_Channel3_IRQn
#define IO_TEST_GDMA_Channel_Handler        GDMA0_Channel3_Handler
#elif IO_TEST_GDMA_CHANNEL_MUM == 4
#endif

void board_pwm_init(void);
void driver_pwm_init(void);
void board_ir_init(void);
void driver_ir_init(void);
void driver_ir_gdma_init(void);


#ifdef __cplusplus
}
#endif

#endif

