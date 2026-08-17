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
#include "rtl_tim.h"

#include "board.h"

#include "app_msg.h"

/* Defines ------------------------------------------------------------------*/
#define IR_DATA_SIZE_MAX                100

#define IR_RX_PIN                       P2_5
#define IR_RX_FIFO_THR_LEVEL            10

/* Private define ------------------------------------------------------------*/
#define PWM_OUT_PIN             P2_6

#define PWM_TIMER_NUM           TIM2
#define PWM_OUT_PIN_PINMUX      TIMER_PWM2

#define PWM_PERIOD              26.3 //uint:us
#define PWM_DUTY_CYCLE          50     //uint:percent
#define PWM_HIGH_COUNT          ((((PWM_PERIOD)*(PWM_DUTY_CYCLE*40))/100)-1)
#define PWM_LOW_COUNT           ((((PWM_PERIOD)*((100-PWM_DUTY_CYCLE)*40))/100)-1)

/**
  * @brief  IR data structure definition
  */
typedef struct
{
    /* Unit of carrierFreq is KHz */
    uint8_t     CarrierFreq;
    uint32_t    DataBuf[IR_DATA_SIZE_MAX];
    uint16_t    DataLen;
} IR_Data_TypeDef;

void global_data_ir_init(void);
void board_ir_init(void);
void driver_ir_init(void);
void ir_demo(void);
void io_handle_ir_msg(T_IO_MSG *io_ir_msg);
void board_pwm_init(void);
void driver_pwm_init(void);

#ifdef __cplusplus
}
#endif

#endif

