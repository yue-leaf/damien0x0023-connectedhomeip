/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include "rtl_codec.h"
#include "rtl_gdma.h"
#include "rtl_i2s.h"
#include "rtl_nvic.h"
#include "rtl_pinmux.h"
#include "rtl_rcc.h"
#include "rtl_uart.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* GDMA RX defines */
#define GDMA_Channel_AMIC_NUM           0
#define GDMA_Channel_AMIC               GDMA_Channel0
#define GDMA_Channel_AMIC_IRQn          GDMA0_Channel0_IRQn
#define GDMA_Channel_AMIC_Handler       GDMA0_Channel0_Handler

#define GDMA_TX_BURST_SIZE              ((uint32_t)1)
#define GDMA_RX_BURST_SIZE              ((uint32_t)1)
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

void codec_demo(void);


