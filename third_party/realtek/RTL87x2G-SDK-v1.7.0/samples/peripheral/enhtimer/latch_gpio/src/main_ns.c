/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "trace.h"
#include "rtl_gpio.h"
#include "io_latch_gpio.h"

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

    DBG_DIRECT("Start latch_gpio test!");

    /* Initialize timer peripheral */
    driver_enhance_timer_init();

    /* Configure PAD and pinmux firstly! */
    board_gpio_init();

    /* Initialize gpio peripheral */
    driver_gpio_init();

    while (1)
    {
        /* Simulate GPIO trigger signal */
        for (volatile uint32_t i = 0; i < 1000; i++);
        GPIO_WriteBit(GPIO_PORT_OUTPUT, GPIO_PIN_OUTPUT, (BitAction)(1));
        for (volatile uint32_t i = 0; i < 1000; i++);
        GPIO_WriteBit(GPIO_PORT_OUTPUT, GPIO_PIN_OUTPUT, (BitAction)(0));
    }
}


