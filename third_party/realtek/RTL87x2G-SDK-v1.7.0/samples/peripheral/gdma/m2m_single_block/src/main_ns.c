/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "trace.h"
#include "rtl876x.h"
#include "io_gdma.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program.
  * @param  None.
  * @retval None.
  */
int main(void)
{
    __enable_irq();

    DBG_DIRECT("Start m2m single block test!");

    /* Initialize gdma peripheral */
    driver_gdma_init();

    while (1);
}


