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

#define CAN_RX_FIFO_EN    1
#define CAN_TIME_STAMP_EN 0

#if CAN_TIME_STAMP_EN
#define CAN_TX_TRIGGER_EN 0
#endif

/*============================================================================*
 *                              Variables
 *============================================================================*/

/*============================================================================*
 *                              Functions
 *============================================================================*/
void can_board_init(void);
void can_driver_init(void);
void can_basic_tx(uint32_t buf_id, uint8_t frame_type, \
                  uint16_t frame_id, uint32_t ext_id, uint8_t *tx_data, uint8_t data_len);
void can_basic_rx(uint32_t buf_id);
void can_demo(void);



