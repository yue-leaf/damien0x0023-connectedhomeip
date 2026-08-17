/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _SYSTEM_TRACE_H_
#define _SYSTEM_TRACE_H_

#include "app_section.h"
#include "trace_heap.h"
#include "trace_stack.h"
#include "trace_timer.h"
#include "trace_queue.h"
#include "trace_system_loading.h"
#include "trace_hardfault.h"


#ifdef __cplusplus
extern "C" {
#endif

extern HEAP_INFO heap_info[TRACE_HEAP_TYPE_NUM];
extern STACK_INFO stack_info;
extern TIMER_INFO timer_info;
extern QUEUE_INFO queue_info[MAX_QUEUE_OBJECTS_NUM];

void system_trace_init(void);
void print_system_trace_lib_version(void);

#ifdef __cplusplus
}
#endif

#endif /*_SYSTEM_TRACE_H_*/
