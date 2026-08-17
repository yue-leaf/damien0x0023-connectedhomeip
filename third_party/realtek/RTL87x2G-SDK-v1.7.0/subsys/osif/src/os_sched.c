/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#include <stdint.h>
#include <stdbool.h>
#include "app_section.h"
#include "cmsis_compiler.h"
#include "os_sched.h"
#include "osif.h"
#include "trace.h"
#include "rtl876x.h"

void os_init_imp(void)
{
    osif_init();
}

void os_vector_table_update_imp(void)
{
    osif_vector_table_update();
}

void os_delay_imp(uint32_t ms)
{
    osif_delay(ms);
}

uint64_t os_sys_time_get_imp(void)
{
    return osif_sys_time_get();
}

uint64_t os_sys_tick_get_imp(void)
{
    return osif_sys_tick_get();
}

bool os_sched_start_imp(void)
{
    osif_sched_start();

    return true;
}

bool os_sched_stop_imp(void)
{
    osif_sched_stop();

    return true;
}

bool os_sched_suspend_imp(void)
{
    osif_sched_suspend();

    return true;
}

bool os_sched_resume_imp(void)
{
    osif_sched_resume();

    return true;
}

bool os_sched_state_get_imp(long *p_state)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_sched_state_get(p_state);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_sched_state_get error: %u, LR: 0x%x", ret, lr);
        return false;
    }
    return true;
}

bool os_sched_is_start_imp(void)
{
    return osif_sched_is_start();
}

void os_sched_func_init(void) APP_FLASH_TEXT_SECTION;
void os_sched_func_init(void)
{
    os_init = os_init_imp;
    os_vector_table_update = os_vector_table_update_imp;

    os_delay = os_delay_imp;
    os_sys_time_get = os_sys_time_get_imp;
    os_sys_tick_get = os_sys_tick_get_imp;
    os_sched_start = os_sched_start_imp;
    os_sched_stop = os_sched_stop_imp;
    os_sched_suspend = os_sched_suspend_imp;
    os_sched_resume = os_sched_resume_imp;
    os_sched_state_get = os_sched_state_get_imp;
    os_sched_is_start = os_sched_is_start_imp;

}
