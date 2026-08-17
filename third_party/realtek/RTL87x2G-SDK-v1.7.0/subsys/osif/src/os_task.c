/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include "app_section.h"
#include "os_task.h"
#include "osif.h"
#include "trace.h"

bool os_alloc_secure_ctx_imp(uint32_t stack_size)
{
    osif_alloc_secure_ctx(stack_size);

    return true;
}

bool os_task_create_imp(void **pp_handle, const char *p_name, void (*p_routine)(void *),
                        void *p_param, uint16_t stack_size, uint16_t priority)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_task_create(pp_handle, p_name, p_routine, p_param, stack_size, priority);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_task_create error: %u, LR: 0x%x", ret, lr);
        return false;
    }
    return true;
}

bool os_task_delete_imp(void *p_handle)
{
    osif_task_delete(p_handle);

    return true;
}

bool os_task_suspend_imp(void *p_handle)
{
    osif_task_suspend(p_handle);
    return true;
}

bool os_task_resume_imp(void *p_handle)
{
    osif_task_resume(p_handle);
    return true;
}

bool os_task_yield_imp(void)
{
    osif_task_yield();
    return true;
}

bool os_task_handle_get_imp(void **pp_handle)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_task_handle_get(pp_handle);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_task_handle_get error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_task_priority_get_imp(void *p_handle, uint16_t *p_priority)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret =  osif_task_priority_get(p_handle, p_priority);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_task_priority_get error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_task_priority_set_imp(void *p_handle, uint16_t priority)
{
    osif_task_priority_set(p_handle, priority);
    return true;
}

bool os_task_signal_create_imp(void *p_handle, uint32_t count)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret =  osif_task_signal_create(p_handle, count);
    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_task_signal_create error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_task_signal_send_imp(void *p_handle, uint32_t signal)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret =  osif_task_signal_send(p_handle, signal);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_task_signal_send error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_task_signal_recv_imp(uint32_t *p_signal, uint32_t wait_ms)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret =  osif_task_signal_recv(p_signal, wait_ms);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_task_signal_recv error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_task_signal_clear_imp(void *p_handle)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret =  osif_task_signal_clear(p_handle);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_task_signal_clear error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_task_notify_take_imp(long xClearCountOnExit, uint32_t xTicksToWait, uint32_t *p_notify)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret =  osif_task_notify_take(xClearCountOnExit, xTicksToWait, p_notify);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_task_notify_take error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_task_notify_give_imp(void *p_handle)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret =  osif_task_notify_give(p_handle);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_task_notify_give error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

void os_task_status_dump_imp(void)
{
    osif_task_status_dump();
}

void os_task_func_init(void) APP_FLASH_TEXT_SECTION;
void os_task_func_init(void)
{
    os_alloc_secure_ctx = os_alloc_secure_ctx_imp;
    os_task_create = os_task_create_imp;
    os_task_delete = os_task_delete_imp;
    os_task_suspend = os_task_suspend_imp;
    os_task_resume = os_task_resume_imp;
    os_task_yield = os_task_yield_imp;
    os_task_handle_get = os_task_handle_get_imp;
    os_task_priority_get = os_task_priority_get_imp;
    os_task_priority_set = os_task_priority_set_imp;
    os_task_signal_create = os_task_signal_create_imp;
    os_task_signal_send = os_task_signal_send_imp;
    os_task_signal_recv = os_task_signal_recv_imp;
    os_task_signal_clear = os_task_signal_clear_imp;
    os_task_notify_take = os_task_notify_take_imp;
    os_task_notify_give = os_task_notify_give_imp;
    os_task_status_dump = os_task_status_dump_imp;
}
