/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include <string.h>
#include "app_section.h"
#include "os_mem.h"
#include "osif.h"
#include "trace.h"

void *os_mem_alloc_intern_imp(RAM_TYPE ram_type, size_t size,
                              const char *p_func, uint32_t file_line)
{
    void *p;

    p = osif_mem_alloc(ram_type, size);
    if (p == NULL)
    {
        OSIF_PRINT_ERROR5("os_mem_alloc_intern: %s<%u> ram type %u, required size %u, unused size %u",
                          TRACE_STRING(p_func), file_line, ram_type, size, osif_mem_peek(ram_type));
    }

    return p;
}

void *os_mem_zalloc_intern_imp(RAM_TYPE ram_type, size_t size,
                               const char *p_func, uint32_t file_line)
{
    void *p;

    p = osif_mem_alloc(ram_type, size);
    if (p == NULL)
    {
        OSIF_PRINT_ERROR5("os_mem_zalloc_intern: %s<%u> ram type %u, required size %u, unused size %u",
                          TRACE_STRING(p_func), file_line, ram_type, size, osif_mem_peek(ram_type));
    }
    else
    {
        memset(p, 0, size);
    }

    return p;
}

void *os_mem_aligned_alloc_intern_imp(RAM_TYPE ram_type, size_t size, uint8_t alignment,
                                      const char *p_func, uint32_t file_line)
{
    void *p;

    p = osif_mem_aligned_alloc(ram_type, size, alignment);
    if (p == NULL)
    {
        OSIF_PRINT_ERROR5("os_mem_aligned_alloc_intern: %s<%u> ram type %u, required size %u, unused size %u",
                          TRACE_STRING(p_func), file_line, ram_type, size, osif_mem_peek(ram_type));
    }

    return p;
}

void os_mem_free_imp(void *p_block)
{
    osif_mem_free(p_block);
}

void os_mem_aligned_free_imp(void *p_block)
{
    osif_mem_aligned_free(p_block);
}

size_t os_mem_peek_imp(RAM_TYPE ram_type)
{
    return osif_mem_peek(ram_type);
}

size_t os_mem_peek_max_free_block_imp(RAM_TYPE ram_type)
{
    return osif_mem_peek_max_free_block(ram_type);
}

void os_mem_peek_printf_imp(void)
{
    OS_PRINT_TRACE3("heap memory scan:data on %d, buffer on %d, ext data sram %d",
                    os_mem_peek(RAM_TYPE_DATA_ON), os_mem_peek(RAM_TYPE_BUFFER_ON),
                    os_mem_peek(RAM_TYPE_EXT_DATA_SRAM));
}

void os_mem_check_heap_usage_imp(void)
{
    osif_mem_check_heap_usage();
}

void os_mem_func_init(void) APP_FLASH_TEXT_SECTION;
void os_mem_func_init(void)
{
    os_mem_alloc_intern = os_mem_alloc_intern_imp;
    os_mem_zalloc_intern = os_mem_zalloc_intern_imp;
    os_mem_aligned_alloc_intern = os_mem_aligned_alloc_intern_imp;
    os_mem_free = os_mem_free_imp;
    os_mem_aligned_free = os_mem_aligned_free_imp;
    os_mem_peek = os_mem_peek_imp;
    os_mem_peek_max_free_block = os_mem_peek_max_free_block_imp;
    os_mem_peek_printf = os_mem_peek_printf_imp;
    os_mem_check_heap_usage = os_mem_check_heap_usage_imp;
}
