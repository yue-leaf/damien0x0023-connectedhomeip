/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include "app_section.h"
#include "os_trace.h"
#include "osif.h"
#include "trace.h"


bool os_trace_isr_create_imp(void **pp_handle, const char *p_name, uint32_t priority)
{
    return osif_trace_isr_create(pp_handle, p_name, priority);
}

bool os_trace_isr_begin_imp(void *pp_handle)
{
    return osif_trace_isr_begin(pp_handle);
}

bool os_trace_isr_end_imp(void)
{
    return osif_trace_isr_end();
}

void os_trace_func_init(void) APP_FLASH_TEXT_SECTION;
void os_trace_func_init(void)
{
    os_trace_isr_create = os_trace_isr_create_imp;
    os_trace_isr_begin = os_trace_isr_begin_imp;
    os_trace_isr_end = os_trace_isr_end_imp;
}
