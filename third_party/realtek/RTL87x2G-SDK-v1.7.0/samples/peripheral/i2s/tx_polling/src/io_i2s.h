/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "rtl_i2s.h"
#include "rtl_pinmux.h"
#include "rtl_rcc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define I2S_LRCK_PIN                P3_2
#define I2S_BCLK_PIN                P3_3
#define I2S_DATA_TX_PIN             P4_0

#define I2S_NUM                     I2S0
#define I2S_LRCK_PINMUX             LRC_SPORT0
#define I2S_BCLK_PINMUX             BCLK_SPORT0
#define I2S_DATA_TX_PINMUX          DACDAT_SPORT0

#define APB_I2S                     APBPeriph_I2S0
#define APB_I2S_CLOCK               APBPeriph_I2S0_CLOCK

void i2s_demo(void);

#ifdef __cplusplus
}
#endif

