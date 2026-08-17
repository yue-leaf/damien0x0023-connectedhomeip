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
    __enable_irq();

    DBG_DIRECT("Start ADC interrupt test!");
    adc_demo();

    while (1)
    {
    }
}


