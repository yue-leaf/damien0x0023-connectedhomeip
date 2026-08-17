/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <os_sched.h>
#include <trace.h>
#include <app_task.h>
#include "pm.h"
#include "psram.h"


/**
 * @brief    Contains the initialization of all tasks
 * @note     There is only one task in BLE Peripheral APP, thus only one APP task is init here
 * @return   void
 */
void task_init(void)
{
    app_task_init();
}

static void display_clock_config(void)
{
    uint32_t actual_mhz;
    int32_t ret;
    ret = pm_cpu_freq_set(125, &actual_mhz);

    flash_nor_set_seq_trans_enable(FLASH_NOR_IDX_SPIC0, 1);
    fmc_flash_nor_clock_switch(FLASH_NOR_IDX_SPIC0, 160, &actual_mhz);
    flash_nor_try_high_speed_mode(0, FLASH_NOR_DTR_4_BIT_MODE);

    fmc_psram_clock_switch(PSRAM_IDX_SPIC1, 160, &actual_mhz);
    psram_winbond_opi_init();

    pm_display_freq_set(CLK_PLL1_SRC, 100, 100);
}

/**
 * @brief    Entry of APP code
 * @return   int (To avoid compile warning)
 */
int main(void)
{
    DBG_DIRECT("APP MAIN, Demo for RGB 800*480");

    display_clock_config();
    extern void rtk_lcd_hal_init();
    rtk_lcd_hal_init();
    DBG_DIRECT("APP MAIN, Finish RGB 800*480 Init");

    task_init();
    os_sched_start();

    return 0;
}
/** @} */ /* End of group PERIPH_DEMO_MAIN */


