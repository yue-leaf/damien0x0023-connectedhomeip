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

/** ADC data hardware average config,
  * only sample one channel data and onle read data from ADC_Schedule_Index_0.
  * Mode: 0: ADC data hardware average disable
  *       1: ADC data hardware average enable
  */
#define ADC_DATA_HW_AVERAGE         1

/** ADC data output mode config.
  * Mode: 0: ADC data output to schedule table
  *       1: ADC data output to fifo
  */


/** ADC ADC multiple fixed interval sampling config.
  * Mode: 0: ADC sample once and stop
  *       1: ADC timing sampling
  */
#define ADC_MULTI_SAMPLE            1

#define ADC_ONE_SHOT_SAMPLE_PERIOD  (1000000-1)//100ms

void adc_demo(void);

#ifdef __cplusplus
}
#endif

#endif

