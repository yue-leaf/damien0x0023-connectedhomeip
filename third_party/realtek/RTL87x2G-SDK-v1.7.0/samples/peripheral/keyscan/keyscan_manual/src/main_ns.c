/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <string.h>
#include <stdlib.h>

#include "trace.h"
#include "io_keyscan.h"
#include "os_sched.h"

/**
  * @brief  Initialize global data.
  * @param  No parameter.
  * @return void
  */
void global_data_init(void)
{
    global_data_keyscan_init();
}

/**
  * @brief    Contains the initialization of pinmux settings and pad settings
  * @note     All the pinmux settings and pad settings shall be initiated in this function,
  *           but if legacy driver is used, the initialization of pinmux setting and pad setting
  *           should be peformed with the IO initializing.
  * @return   void
  */
void board_init(void)
{
    board_keyboard_init();
}

/**
  * @brief    Contains the initialization of peripherals
  * @note     Both new architecture driver and legacy driver initialization method can be used
  * @return   void
  */
void driver_init(void)
{
    driver_keyboard_init(ENABLE);
}

/**
  * @brief    Entry of app code
  * @return   int (To avoid compile warning)
  */
int main(void)
{
    __enable_irq();
    global_data_init();
    board_init();
    driver_init();
    timer_keyscan_init();
    os_sched_start();

    return 0;
}
/** @} */ /* End of group PERIPH_DEMO_MAIN */


