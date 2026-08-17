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
#define CODEC_GDMA_Channel_NUM          0
#define CODEC_GDMA_Channel              GDMA_Channel0
#define CODEC_GDMA_Channel_IRQn         GDMA0_Channel0_IRQn
#define CODEC_GDMA_Channel_Handler      GDMA0_Channel0_Handler

/* Private define of CODEC cos table */
#define CODEC_DRV_SAMPLE_RATE             16000   // 48k
#define CODEC_DRV_SAMPLE_INTERVAL         5
#define CODEC_DRV_COS_TABLE_SAMPLE        3200//(CODEC_DRV_SAMPLE_RATE / CODEC_DRV_SAMPLE_INTERVAL)
#define CODEC_DRV_AMP_DENOM               128
#define CODEC_DRV_COS_TABLE_BLK_SIZE      256     // bytes(128 samples)
#define CODEC_DRV_COS_TABLE_SIZE          (CODEC_DRV_COS_TABLE_SAMPLE * 2)
#define CODEC_DRV_COS_TABLE_BLK_NUM       (CODEC_DRV_COS_TABLE_SIZE / CODEC_DRV_COS_TABLE_BLK_SIZE)
#define PI    3.14159265359
#define FREQ  1000
#define CODEC_DRV_TX_FREQ_NUM_MAX         64
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

void codec_demo(void);


