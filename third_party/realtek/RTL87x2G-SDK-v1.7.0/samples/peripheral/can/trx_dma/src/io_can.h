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

#define RX_DMA_BUF_ID       1

#define RX_GDMA_CHANNEL_NUM                0
#define RX_GDMA_Channel                    GDMA_Channel0
#define RX_GDMA_Channel_IRQn               GDMA0_Channel0_IRQn
#define RX_GDMA_Channel_Handler            GDMA0_Channel0_Handler

#define TX_GDMA_CHANNEL_NUM                1
#define TX_GDMA_Channel                    GDMA_Channel1
#define TX_GDMA_Channel_IRQn               GDMA0_Channel1_IRQn
#define TX_GDMA_Channel_Handler            GDMA0_Channel1_Handler
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
void can_basic_rx(void);
void can_demo(void);



