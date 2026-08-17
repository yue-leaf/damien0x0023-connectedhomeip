/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "trace.h"
#include "io_pwm_dma.h"

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
    /* Enable Interrupt */
    __enable_irq();

    DBG_DIRECT("Start pwm_dma test!");

    /* Configure PAD and pinmux firstly! */
    board_enhance_pwm_init();

    /* Initialize gdma peripheral */
    driver_gdma_init();

    /* Initialize timer peripheral */
    enhance_timer_init();

    while (1);

}


