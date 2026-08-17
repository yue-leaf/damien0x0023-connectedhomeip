/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include "trace.h"
#include "rtl876x.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#include "rtl_ir.h"
#include "rtl_nvic.h"
#include "rtl_pinmux.h"
#include "rtl_rcc.h"
#include "rtl_gdma.h"

#include "trace.h"

#define IR_TX_PIN                       P2_5
#define IR_TX_FIFO_THR_LEVEL            2
#define IR_DATA_SIZE_MAX                68

#define IO_TEST_GDMA_TRANSFER_SIZE              IR_DATA_SIZE_MAX

#define IO_TEST_GDMA_CHANNEL_MUM                1

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_ir_init(void);

/**
  * @brief  Initialize ir peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_ir_init(uint32_t vFreq);

void ir_demo(void);


