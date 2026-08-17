/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include "app_section.h"
#include "os_timer.h"
#include "osif.h"
#include "trace.h"

bool os_timer_id_get_imp(void **pp_handle, uint32_t *p_timer_id)
{
    uint32_t timer_id = 0;
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_timer_id_get(pp_handle, &timer_id);

    if (ret == OS_EXE_SUCCESS)
    {
        *p_timer_id = timer_id;
        return true;
    }
    else
    {
        *p_timer_id = 0;
        OSIF_PRINT_ERROR2("os_timer_id_get error: %u, LR: 0x%x", ret, lr);
        return false;
    }
}

bool os_timer_create_imp(void **pp_handle, const char *p_timer_name, uint32_t timer_id,
                         uint32_t interval_ms, bool reload, void (*p_timer_callback)(void *))
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_timer_create(pp_handle, p_timer_name, timer_id, interval_ms, reload, p_timer_callback);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_timer_create error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_timer_start_imp(void **pp_handle)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_timer_start(pp_handle);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_timer_start error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_timer_restart_imp(void **pp_handle, uint32_t interval_ms)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_timer_restart(pp_handle, interval_ms);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_timer_restart error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_timer_stop_imp(void **pp_handle)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_timer_stop(pp_handle);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_timer_stop error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_timer_delete_imp(void **pp_handle)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_timer_delete(pp_handle);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_timer_delete error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_timer_pend_function_call_imp(void (*p_pend_function)(void *, uint32_t),
                                     void *pvParameter1, uint32_t ulParameter2)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_timer_pend_function_call(p_pend_function, pvParameter1, ulParameter2);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_timer_pend_function_call error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_timer_is_timer_active_imp(void **pp_handle)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_timer_is_timer_active(pp_handle);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_timer_is_timer_active error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_timer_state_get_imp(void **pp_handle, uint32_t *p_timer_state)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_timer_state_get(pp_handle, p_timer_state);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_timer_state_get error: %u, LR: 0x%x", ret, lr);
        return false;
    }
    else
    {
        return false;
    }

}

bool os_timer_get_auto_reload_imp(void **pp_handle, long *p_autoreload)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_timer_get_auto_reload(pp_handle, p_autoreload);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_timer_get_auto_reload error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_timer_get_timer_number_imp(void **pp_handle, uint8_t *p_timer_number)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_timer_get_timer_number(pp_handle, p_timer_number);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_timer_get_timer_number error: %d, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_timer_handle_get_imp(uint8_t timer_idx, void **pp_handle)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_timer_handle_get(timer_idx, pp_handle);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_timer_handle_get error: %d, LR: 0x%x", ret, lr);
        return false;
    }
    else
    {
        return false;
    }

}

bool os_timer_dump_imp(void)
{
    osif_timer_dump();
    return true;
}

void os_timer_init_imp(void)
{
    osif_timer_init();
}

void os_timer_func_init(void) APP_FLASH_TEXT_SECTION;
void os_timer_func_init(void)
{
    os_timer_id_get = os_timer_id_get_imp;
    os_timer_create = os_timer_create_imp;
    os_timer_start = os_timer_start_imp;
    os_timer_restart = os_timer_restart_imp;
    os_timer_stop = os_timer_stop_imp;
    os_timer_delete = os_timer_delete_imp;
    os_timer_pend_function_call = os_timer_pend_function_call_imp;
    os_timer_is_timer_active = os_timer_is_timer_active_imp;
    os_timer_state_get = os_timer_state_get_imp;
    os_timer_get_auto_reload = os_timer_get_auto_reload_imp;
    os_timer_get_timer_number = os_timer_get_timer_number_imp;
//    os_timer_handle_get = os_timer_handle_get_imp;
    os_timer_dump = os_timer_dump_imp;
    os_timer_init = os_timer_init_imp;
}
