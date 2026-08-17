/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include "rtl_lpc.h"
#include "rtl_nvic.h"
#include "rtl_pinmux.h"
#include "rtl_rcc.h"
#include "trace.h"
#include "rtl876x.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/** LPC voltage detect channel config.
  * LPC channel select: LPC_CHANNEL_ADC0
  *                     LPC_CHANNEL_ADC1
  *                     LPC_CHANNEL_ADC2
  *                     LPC_CHANNEL_ADC3
  *                     LPC_CHANNEL_ADC4
  *                     LPC_CHANNEL_ADC5
  *                     LPC_CHANNEL_ADC6
  *                     LPC_CHANNEL_ADC7
  *                     LPC_CHANNEL_VBAT
  */
#define LPC_CAPTURE_CHANNEL             LPC_CHANNEL_ADC2

#define LPC_VOLTAGE_DETECT_EDGE         LPC_Vin_Over_Vth

#define LPC_TEST_PIN                    P2_2

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void board_lpc_init(void);
void driver_lpc_init(void);


