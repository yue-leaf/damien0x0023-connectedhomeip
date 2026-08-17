/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "rtl_rcc.h"
#include "rtl_can.h"
#include "string.h"
#include "rtl_pinmux.h"
#include "rtl_nvic.h"

/*============================================================================*
 *                              Macros
 *============================================================================*/
#define CAN_TX_PIN          P3_2
#define CAN_RX_PIN          P3_4

/*============================================================================*
 *                              Variables
 *============================================================================*/

/*============================================================================*
 *                              Functions
 *============================================================================*/
void can_board_init(void);
void can_driver_init(void);
void can_tx_auto_reply(void);
void can_rx_auto_reply(void);
void can_demo(void);



