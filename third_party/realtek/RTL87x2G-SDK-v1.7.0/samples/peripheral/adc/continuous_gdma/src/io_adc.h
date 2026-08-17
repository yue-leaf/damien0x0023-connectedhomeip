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
#include "rtl876x.h"


#define ADC_SAMPLE_PIN_0                    P2_4
#define ADC_SAMPLE_CHANNEL_0                ADC_Channel_Index_4

#define ADC_GDMA_CHANNEL_NUM                0
#define ADC_GDMA_Channel                    GDMA_Channel0
#define ADC_GDMA_Channel_IRQn               GDMA0_Channel0_IRQn
#define ADC_GDMA_Channel_Handler            GDMA0_Channel0_Handler

#define ADC_CONTINUOUS_SAMPLE_PERIOD        (200-1)//20us

#define ADC_FIFO_MAX                        (32)

#define GDMA_TRANSFER_SIZE                  100

void board_adc_init(void);
void driver_adc_init(void);
void adc_demo(void);

#ifdef __cplusplus
}
#endif

#endif

