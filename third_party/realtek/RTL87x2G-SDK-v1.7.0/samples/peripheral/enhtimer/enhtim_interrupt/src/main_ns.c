/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "trace.h"
#include "rtl_gpio.h"
#include "io_enhtim_interrupt.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program.
  * @param  None.
  * @return None.
  */
int main(void)
{
    /* Enable Interrupt */
    __enable_irq();

    DBG_DIRECT("Start enhtim interrupt test!");

    /* Configure PAD and pinmux firstly! */
    board_gpio_init();

    /* Initialize timer and gpio peripheral */
    driver_gpio_init();
    driver_enhtim_init();

    while (1);
}


