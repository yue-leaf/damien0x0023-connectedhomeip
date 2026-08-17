/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "rtl_i2s.h"
#include "rtl_i2s.h"
#include "rtl_nvic.h"
#include "rtl_pinmux.h"
#include "rtl_rcc.h"
#include "rtl_gdma.h"

#ifdef __cplusplus
extern "C" {
#endif

#define I2S_CONFIG_GDMA                 0

#define GDMA_TRANSFER_SIZE             200
/* GDMA RX defines */
#define TDM0_TX_GDMA_Channel_NUM          3
#define TDM0_TX_GDMA_Channel              GDMA_Channel3
#define TDM0_TX_GDMA_Channel_IRQn         GDMA0_Channel3_IRQn
#define TDM0_TX_GDMA_Channel_Handler      GDMA0_Channel3_Handler

/* GDMA RX defines */
#define TDM0_RX_GDMA_Channel_NUM          2
#define TDM0_RX_GDMA_Channel              GDMA_Channel2
#define TDM0_RX_GDMA_Channel_IRQn         GDMA0_Channel2_IRQn
#define TDM0_RX_GDMA_Channel_Handler      GDMA0_Channel2_Handler

/* Defines -------------------------------------------------------------------*/
#define I2S0_BCLK_PIN                    P4_0
#define I2S0_LRCK_PIN                    P4_1
#define I2S0_DATA_TX_PIN                 P4_2
#define I2S0_DATA_RX_PIN                 P4_3

#define I2S1_BCLK_PIN                    P2_4
#define I2S1_LRCK_PIN                    P2_5
#define I2S1_DATA_TX_PIN                 P2_6
#define I2S1_DATA_RX_PIN                 P2_7

void board_i2s_init(void);
void driver_i2s0_TDM2_tx_init(void);
void driver_i2s1_TDM2_rx_init(void);
void i2s_demo(void);

#if (I2S_CONFIG_GDMA == 1)
void driver_gdma_tx_init(void);
void driver_gdma_rx_init(void);
#endif


#ifdef __cplusplus
}
#endif

