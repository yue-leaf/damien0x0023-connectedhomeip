/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "trace.h"
#include "rtl876x.h"

#include "io_spi.h"

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

    DBG_DIRECT("Start spi polling test!");

    /* Configure pad and pinmux firstly! */
    board_spi_init();

    /* Initialize gpio peripheral */
    driver_spi_init();

#if (SPI_CONFIG_GPIO_SIM_CS == 1)
    driver_gpio_init();
#endif

    /* Start spi demo */
    spi_demo();

    while (1);
}


