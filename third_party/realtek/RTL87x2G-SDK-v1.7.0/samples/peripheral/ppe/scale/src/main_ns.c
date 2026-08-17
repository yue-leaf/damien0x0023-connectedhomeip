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
int main(void)
{
    DBG_DIRECT("APP MAIN");
    DBG_DIRECT("test ppe func = %s, line = %d", __func__, __LINE__);
    extern void PPE_scale_test(void);
    PPE_scale_test();
    task_init();
    os_sched_start();
    DBG_DIRECT("testend ppe func = %s, line = %d", __func__, __LINE__);
    return 0;
}
/** @} */ /* End of group PERIPH_DEMO_MAIN */


