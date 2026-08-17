/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include <string.h>
#include "FreeRTOSConfig.h"
#include "app_section.h"
#include "os_pm.h"
#include "osif.h"
#include "trace.h"
#include "utils.h"
#include "os_sched.h"
#include "os_timer.h"
#include "dlps.h"

extern uint32_t (*platform_rtc_get_counter)(void);

OSPMFeatureConfig os_pm_feature_cfg =
{
    .os_pm_statistic = 0,
    .rsvd = 0,
};

OSPMSystem os_pm_system =
{
    .last_sleep_clk = 0,
    .last_sleep_systick = 0,
    .tickcount_restore_remain_us = 0,
};

bool os_sched_restore_imp(void)
{
    osif_sched_restore();

    return true;
}

void os_systick_handler_imp(void)
{
    osif_systick_handler();
}

uint32_t os_sys_tick_rate_get_imp(void)
{
    return osif_sys_tick_rate_get();
}

uint32_t os_sys_tick_clk_get_imp(void)
{
    return osif_sys_tick_clk_get();
}

uint64_t os_sys_tick_increase_imp(uint32_t tick_increment)
{
    return osif_sys_tick_increase(tick_increment);
}

uint32_t os_pm_next_timeout_value_get_imp(void)
{
    return osif_pm_next_timeout_value_get();
}

bool os_register_pm_excluded_handle_imp(void **handle, PlatformExcludedHandleType type)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_register_pm_excluded_handle(handle, type);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_register_pm_excluded_handle error: %u, LR: 0x%x", ret, lr);
        return false;
    }
    return true;
}

bool os_unregister_pm_excluded_handle_imp(void **handle, PlatformExcludedHandleType type)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_unregister_pm_excluded_handle(handle, type);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_unregister_pm_excluded_handle error: %u, LR: 0x%x", ret, lr);
        return false;
    }
    return true;
}

uint32_t os_pm_get_valid_systick_current_value(bool need_update)
{
    if (need_update)
    {
        UPDATE_TICK_COUNT();
    }

    uint32_t systick_current_value = SysTick->VAL;

    if (need_update)
    {
        if (systick_current_value != 1)
        {
            /* if current value == 1, there is uncertainty of tick count increment, drop 1 tick error */
            /* Otherwise, update tick count if systick timer timeout */
            UPDATE_TICK_COUNT();
        }
    }
    else
    {
        systick_current_value = (portNVIC_INT_CTRL_REG & SCB_ICSR_PENDSTSET_Msk) ? NULL :
                                systick_current_value;
    }

    return systick_current_value;
}

void os_pm_store_tickcount(void)
{
    //------should not change order-------//
    os_pm_system.last_sleep_clk = platform_rtc_get_counter();

    /* stop systick */
    SysTick->CTRL = 0;

    /* update xTickCount and store systick current value */
    os_pm_system.last_sleep_systick = os_pm_get_valid_systick_current_value(true);
}

void os_pm_restore_tickcount(void)
{
    uint32_t exit_rtc_clk = platform_rtc_get_counter();

    uint32_t exit_cpu_clk = read_cpu_counter();

    uint32_t sleep_time_us = rtc_clk_to_us(clk_cnt_diff(os_pm_system.last_sleep_clk, exit_rtc_clk,
                                                        PF_RTC_BIT_NUM),
                                           ROUND) +
                             os_pm_system.tickcount_restore_remain_us;

    uint32_t sleep_time_clk = us_to_rtc_clk(sleep_time_us, ROUNDDOWN);

    uint32_t sleep_time_tick_count, tick_count_residual_us;

    sleep_time_tick_count = US_TO_TICK_COUNT(sleep_time_us, tick_count_residual_us);

    uint32_t tick_count_residual_systick, systick_residual_us;

    tick_count_residual_systick = US_TO_SYSTICK(tick_count_residual_us, systick_residual_us);

    uint32_t tick_increment = sleep_time_tick_count;

    uint32_t systick_reload = SYSTICK_RELOAD_VALUE - tick_count_residual_systick;

    if (os_pm_system.last_sleep_systick != 0)
    {
        if (tick_count_residual_systick >= os_pm_system.last_sleep_systick)
        {
            tick_increment ++;
            systick_reload = SYSTICK_RELOAD_VALUE - (tick_count_residual_systick -
                                                     os_pm_system.last_sleep_systick);
        }
        else
        {
            systick_reload = os_pm_system.last_sleep_systick - tick_count_residual_systick - 1;
        }
    }

    if (systick_reload == 0)
    {
        tick_increment ++;
        systick_reload = SYSTICK_RELOAD_VALUE + 1;
    }

    SysTick->LOAD = systick_reload;

    SysTick->VAL  = 0;

    uint32_t process_time_cpu_clk = clk_cnt_diff(exit_cpu_clk, read_cpu_counter(),
                                                 CPU_CNT_BIT_NUM);

    SysTick->CTRL = get_sys_tick_clk_type() << 2 |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;

    uint32_t process_time_us = CPU_CLK_TO_US(process_time_cpu_clk);

    os_pm_system.tickcount_restore_remain_us = systick_residual_us + process_time_us;

    /* wait 3 tick of systick timer */
    platform_delay_us(DIVIDE_AND_ROUNDUP(3000000, os_sys_tick_clk_get()));

    uint64_t xTickCount_old = os_sys_tick_increase(tick_increment);

    SysTick->LOAD = SYSTICK_RELOAD_VALUE;

    if (os_pm_feature_cfg.os_pm_statistic)
    {
        uint64_t xTickCount_now = os_sys_tick_get();
        OSIF_PRINT_TRACE4("os_pm_statistic: Tickcount: 0x%08x%08x -> 0x%08x%08x",
                          (uint32_t)(xTickCount_old >> 32),
                          (uint32_t)xTickCount_old,
                          (uint32_t)(xTickCount_now >> 32),
                          (uint32_t)xTickCount_now);
    }
}

#if 0
PMCheckResult os_pm_check(uint32_t *wakeup_time_diff)
{
    uint32_t systick_cur_val = os_pm_get_valid_systick_current_value(false);

    if (systick_cur_val == NULL)
    {
        return PM_CHECK_FAIL;
    }

    uint32_t wakeup_time_diff_tick = os_pm_next_timeout_value_get();

    if (wakeup_time_diff_tick <= 1)
    {
        return PM_CHECK_FAIL;
    }

    if (wakeup_time_diff_tick != PM_INVALID_WAKEUP_TIME_DIFF)
    {
        /* xTickCount unit: 10ms */
        uint32_t wakeup_time_diff_us = TICK_COUNT_TO_US(wakeup_time_diff_tick - 1) +
                                       SYSTICK_TO_US(systick_cur_val) -
                                       os_pm_system.tickcount_restore_remain_us;

        *wakeup_time_diff = us_to_rtc_clk(wakeup_time_diff_us, ROUNDDOWN);
    }

    return PM_CHECK_PASS;
}

void os_pm_store(void)
{
    /* 1. suspend all task then store systick information */
    os_sched_suspend();

    os_pm_store_tickcount();
}

void os_pm_restore(void)
{
    os_pm_restore_tickcount();

    os_sched_resume();

    os_sched_restore();
}
#else
extern PMCheckResult os_pm_check(uint32_t *wakeup_time_diff);
extern void os_pm_store(void);
extern void os_pm_restore(void);
#endif

void os_pm_init_imp(void)
{
    DBG_DIRECT("OS PM init... cb 0x%x", *(volatile uint32_t *)0x138b9c);
    osif_pm_init();

    platform_pm_register_callback_func_with_priority((void *)os_pm_check, PLATFORM_PM_CHECK, 1);
    platform_pm_register_callback_func_with_priority((void *)os_pm_store, PLATFORM_PM_STORE, 1);
    platform_pm_register_callback_func_with_priority((void *)os_pm_restore, PLATFORM_PM_RESTORE, 1);
}

void os_pm_func_init(void) APP_FLASH_TEXT_SECTION;
void os_pm_func_init(void)
{
    os_pm_init = os_pm_init_imp;
    os_sched_restore = os_sched_restore_imp;
    os_systick_handler = os_systick_handler_imp;
    os_sys_tick_rate_get = os_sys_tick_rate_get_imp;
    os_sys_tick_clk_get = os_sys_tick_clk_get_imp;
    os_sys_tick_increase = os_sys_tick_increase_imp;

    os_pm_next_timeout_value_get = os_pm_next_timeout_value_get_imp;

    os_register_pm_excluded_handle = os_register_pm_excluded_handle_imp;
    os_unregister_pm_excluded_handle = os_unregister_pm_excluded_handle_imp;
}
