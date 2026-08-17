/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include "app_section.h"
#include "os_sync.h"
#include "osif.h"
#include "trace.h"

uint32_t os_lock_imp(void)
{
    return osif_lock();
}

void os_unlock_imp(uint32_t s)
{
    osif_unlock(s);
}

bool os_sem_create_imp(void **pp_handle, const char *p_name, uint32_t init_count,
                       uint32_t max_count)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_sem_create(pp_handle, p_name, init_count, max_count);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_sem_create error: %u, LR: 0x%x", ret, lr);
        return false;
    }
    return true;
}

bool os_sem_delete_imp(void *p_handle)
{
    osif_sem_delete(p_handle);
    return true;
}

bool os_sem_take_imp(void *p_handle, uint32_t wait_ms)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_sem_take(p_handle, wait_ms);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_sem_take error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_sem_give_imp(void *p_handle)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_sem_give(p_handle);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_sem_give error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_mutex_create_imp(void **pp_handle)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_mutex_create(pp_handle);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_mutex_create error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_mutex_delete_imp(void *p_handle)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_mutex_delete(p_handle);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_mutex_delete error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_mutex_take_imp(void *p_handle, uint32_t wait_ms)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_mutex_take(p_handle, wait_ms);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_mutex_take error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

bool os_mutex_give_imp(void *p_handle)
{
    OS_EXE_TYPE ret;
    void *lr = __builtin_return_address(0);

    ret = osif_mutex_give(p_handle);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR2("os_mutex_give error: %u, LR: 0x%x", ret, lr);
        return false;
    }

    return true;
}

void os_sync_func_init(void) APP_FLASH_TEXT_SECTION;
void os_sync_func_init(void)
{
    os_lock = os_lock_imp;
    os_unlock = os_unlock_imp;
    os_sem_create = os_sem_create_imp;
    os_sem_delete = os_sem_delete_imp;
    os_sem_take = os_sem_take_imp;
    os_sem_give = os_sem_give_imp;
    os_mutex_create = os_mutex_create_imp;
    os_mutex_delete = os_mutex_delete_imp;
    os_mutex_take = os_mutex_take_imp;
    os_mutex_give = os_mutex_give_imp;
}

