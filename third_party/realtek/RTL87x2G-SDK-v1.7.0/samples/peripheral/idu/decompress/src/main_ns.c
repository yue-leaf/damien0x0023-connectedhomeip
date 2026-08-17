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
#include "flash_nor_device.h"
#include "psram.h"
#include "clock.h"

/**
 * @brief    Contains the initialization of all tasks
 * @note     There is only one task in BLE Peripheral APP, thus only one APP task is init here
 * @return   void
 */
void task_init(void)
{
    app_task_init();
}

/**
 * @brief    Entry of APP code
 * @return   int (To avoid compile warning)
 */
static void display_clock_config(void)
{
    uint32_t actual_mhz;
    pm_cpu_freq_set(125, &actual_mhz);

    flash_nor_set_seq_trans_enable(FLASH_NOR_IDX_SPIC0, 1);
    fmc_flash_nor_clock_switch(FLASH_NOR_IDX_SPIC0, 160, &actual_mhz);
    flash_nor_try_high_speed_mode(0, FLASH_NOR_DTR_4_BIT_MODE);

    fmc_psram_clock_switch(PSRAM_IDX_SPIC1, 160, &actual_mhz);
    psram_winbond_opi_init();

    pm_display_freq_set(CLK_PLL1_SRC, 100, 100);
}

int main(void)
{
    DBG_DIRECT("APP MAIN for imdc demo");

    display_clock_config();

    FLASH_NOR_RET_TYPE ret = flash_nor_try_high_speed_mode(FLASH_NOR_IDX_SPIC0, FLASH_NOR_4_BIT_MODE);
    if (ret)
    {
        DBG_DIRECT("Flash switch 4 bit mode success");
    }

    extern void IDU_test(void);
    IDU_test();
    task_init();
    os_sched_start();

    return 0;
}
/** @} */ /* End of group PERIPH_DEMO_MAIN */


