/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _TRACE_STACK_H_
#define _TRACE_STACK_H_

#include "trace_common.h"
#include "trace_port_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define configMAX_TASK_NAME_LEN                         ( 12 )

typedef struct _TASK_STACK_INFO_BLOCK
{
    uint32_t task_id;
    uint32_t minimum_ever_remain_size;
    char task_name[configMAX_TASK_NAME_LEN];
} TASK_STACK_INFO_BLOCK_t;


typedef struct _STACK_INFO
{
    uint32_t task_number;
    TASK_STACK_INFO_BLOCK_t task_stack_info[MAX_TASK_COUNT_FOR_DEBUG];
} STACK_INFO;

void trace_task_stack(void);

#ifdef __cplusplus
}
#endif

#endif /*_TRACE_STACK_H_*/
