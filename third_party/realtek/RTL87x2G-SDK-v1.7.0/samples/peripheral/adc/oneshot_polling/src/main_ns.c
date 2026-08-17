/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "trace.h"

#include "rtl_adc.h"
#include "io_adc.h"

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  Main program.
  * @param  None.
  * @retval None.
  */
int main(void)
{
    DBG_DIRECT("Start adc polling test!");

    adc_demo();

    while (1)
    {
        DBG_DIRECT("adc_demo_start");
        adc_sample_demo();
        for (uint32_t i = 500000; i > 0; i--);
        ADC_Cmd(ADC, ADC_ONE_SHOT_MODE, ENABLE);
    }
}


