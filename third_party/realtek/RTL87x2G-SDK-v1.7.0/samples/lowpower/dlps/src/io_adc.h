/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __IO_ADC_H
#define __IO_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl_adc.h"
#include "rtl_gpio.h"
#include "rtl_nvic.h"
#include "rtl_pinmux.h"
#include "rtl_rcc.h"
#include "rtl_tim.h"

#include "app_msg.h"
#include "board.h"
#include "pm.h"
#include "io_dlps.h"

/* Defines --------------------------------------------------------------*/
/** ADC sample channel config.
  * ADC sample channel config:P2_0~P2_7 and VBAT
  * ADC sample channel index select: #define ADC_Channel_Index_0         0
  *                                  #define ADC_Channel_Index_1         1
  *                                  #define ADC_Channel_Index_2         2
  *                                  #define ADC_Channel_Index_3         3
  *                                  #define ADC_Channel_Index_4         4
  *                                  #define ADC_Channel_Index_5         5
  *                                  #define ADC_Channel_Index_6         6
  *                                  #define ADC_Channel_Index_7         7

  * If ADC_SAMPLE_PIN = P2_0, then ADC_SAMPLE_CHANNEL = ADC_Channel_Index_0;
  * If ADC_SAMPLE_PIN = P2_1, then ADC_SAMPLE_CHANNEL = ADC_Channel_Index_1;
  * If ADC_SAMPLE_PIN = P2_2, then ADC_SAMPLE_CHANNEL = ADC_Channel_Index_2;
  * If ADC_SAMPLE_PIN = P2_3, then ADC_SAMPLE_CHANNEL = ADC_Channel_Index_3;
  * If ADC_SAMPLE_PIN = P2_4, then ADC_SAMPLE_CHANNEL = ADC_Channel_Index_4;
  * If ADC_SAMPLE_PIN = P2_5, then ADC_SAMPLE_CHANNEL = ADC_Channel_Index_5;
  * If ADC_SAMPLE_PIN = P2_6, then ADC_SAMPLE_CHANNEL = ADC_Channel_Index_6;
  * If ADC_SAMPLE_PIN = P2_7, then ADC_SAMPLE_CHANNEL = ADC_Channel_Index_7;
  */

/* Defines ------------------------------------------------------------------*/
/** ADC pin define.
  */
#define ADC_SAMPLE_PIN_6                P2_6
#define ADC_SAMPLE_PIN_7                P2_7


/* GPIO pin define*/
#define EVB_KEY0                        P4_0
#define ADC_GPIO_DLPS_WAKEUP_PIN        EVB_KEY0
#define GPIO_PIN_INPUT                  GPIO_GetPin(ADC_GPIO_DLPS_WAKEUP_PIN)
#define GPIO_PORT                       GPIO_GetPort(ADC_GPIO_DLPS_WAKEUP_PIN)
#define GPIO_PIN_INPUT_IRQN             GPIOB5_IRQn
#define GPIO_Input_Handler              GPIOB5_Handler

#define ADC_SAMPLE_CHANNEL_0        ADC_Channel_Index_0
#define ADC_SAMPLE_CHANNEL_1        ADC_Channel_Index_1
#define ADC_SAMPLE_CHANNEL_2        ADC_Channel_Index_2
#define ADC_SAMPLE_CHANNEL_3        ADC_Channel_Index_3
#define ADC_SAMPLE_CHANNEL_4        ADC_Channel_Index_4
#define ADC_SAMPLE_CHANNEL_5        ADC_Channel_Index_5
#define ADC_SAMPLE_CHANNEL_6        ADC_Channel_Index_6
#define ADC_SAMPLE_CHANNEL_7        ADC_Channel_Index_7

/* Config ADC channel and mode --------------------------------------------------------------*/
/* ADC bypass mode or divide mode */
#define ADC_DIVIDE_MODE                     1
#define ADC_BYPASS_MODE                     0
#define ADC_MODE_DIVIDE_OR_BYPASS           ADC_DIVIDE_MODE

#define TIMER_ADC_ONE_SHOT_SAMPLE_PERIOD    (100-1)//10us

/* Globals ------------------------------------------------------------------*/
extern POWER_CheckResult IO_ADC_GPIO_DLPS_Enter_Allowed;

void global_data_adc_gpio_init(void);
void board_adc_init(void);
void driver_adc_init(void);
void board_adc_gpio_init(void);
void driver_gpio_init(void);
void io_adc_gpio_dlps_enter(void);
void io_adc_gpio_dlps_exit(void);
POWER_CheckResult io_adc_gpio_dlps_check(void);
//void io_handle_adc_msg(T_IO_MSG *io_adc_msg);
void io_adc_sample_start(void);



#ifdef __cplusplus
}
#endif

#endif

