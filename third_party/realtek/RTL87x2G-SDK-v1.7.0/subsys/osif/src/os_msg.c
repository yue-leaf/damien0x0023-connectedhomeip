/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include "app_section.h"
#include "os_msg.h"
#include "osif.h"
#include "trace.h"

bool os_msg_queue_create_intern_imp(void **pp_handle,  const char *p_name, uint32_t msg_num,
                                    uint32_t msg_size,
                                    const char *p_func, uint32_t file_line)
{
    OS_EXE_TYPE ret;

    ret = osif_msg_queue_create(pp_handle, p_name, msg_num, msg_size);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR5("os_msg_queue_create_intern error: %u, %s<%u> failed msg_num %u, msg_size %u",
                          ret, TRACE_STRING(p_func), file_line, msg_num, msg_size);
        return false;
    }

    return true;
}

bool os_msg_queue_delete_intern_imp(void *p_handle, const char *p_func, uint32_t file_line)
{
    OS_EXE_TYPE ret;

    ret = osif_msg_queue_delete(p_handle);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR4("os_msg_queue_delete_intern error: %u, %s<%u> failed p_handle %p",
                          ret, TRACE_STRING(p_func), file_line, p_handle);
        return false;
    }

    return true;
}

bool os_msg_queue_peek_intern_imp(void *p_handle, uint32_t *p_msg_num,
                                  const char *p_func, uint32_t file_line)
{
    OS_EXE_TYPE ret;

    ret = osif_msg_queue_peek(p_handle, p_msg_num);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR4("os_msg_queue_peek_intern error: %u, %s<%u> failed p_handle %p",
                          ret, TRACE_STRING(p_func), file_line, p_handle);
        return false;
    }

    return true;
}

bool os_msg_send_intern_imp(void *p_handle, void *p_msg, uint32_t wait_ms,
                            const char *p_func, uint32_t file_line)
{
    OS_EXE_TYPE ret;

    ret = osif_msg_send(p_handle, p_msg, wait_ms);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR6("os_msg_send_intern error: %u, %s<%u> failed p_handle %p, p_msg %p, wait_ms %u",
                          ret, TRACE_STRING(p_func), file_line, p_handle, p_msg, wait_ms);
        return false;
    }

    return true;
}

bool os_msg_recv_intern_imp(void *p_handle, void *p_msg, uint32_t wait_ms,
                            const char *p_func, uint32_t file_line)
{
    OS_EXE_TYPE ret;

    ret = osif_msg_recv(p_handle, p_msg, wait_ms);

    if (ret > OS_EXE_SUCCESS)
    {
        if (wait_ms != 0)
        {
            OSIF_PRINT_ERROR6("os_msg_recv_intern error: %u, %s<%u> failed p_handle %p, p_msg %p, wait_ms %u",
                              ret, TRACE_STRING(p_func), file_line, p_handle, p_msg, wait_ms);
        }
        return false;
    }

    return true;
}

bool os_msg_peek_intern_imp(void *p_handle, void *p_msg, uint32_t wait_ms,
                            const char *p_func, uint32_t file_line)
{
    OS_EXE_TYPE ret;

    ret = osif_msg_peek(p_handle, p_msg, wait_ms);

    if (ret > OS_EXE_SUCCESS)
    {
        OSIF_PRINT_ERROR6("os_msg_peek_intern error: %u, %s<%u> failed p_handle %p, p_msg %p, wait_ms %u",
                          ret, TRACE_STRING(p_func), file_line, p_handle, p_msg, wait_ms);
        return false;
    }

    return true;
}


void os_msg_func_init(void) APP_FLASH_TEXT_SECTION;
void os_msg_func_init(void)
{
    os_msg_queue_create_intern = os_msg_queue_create_intern_imp;
    os_msg_queue_delete_intern = os_msg_queue_delete_intern_imp;
    os_msg_queue_peek_intern = os_msg_queue_peek_intern_imp;
    os_msg_send_intern = os_msg_send_intern_imp;
    os_msg_recv_intern = os_msg_recv_intern_imp;
    os_msg_peek_intern = os_msg_peek_intern_imp;
}
