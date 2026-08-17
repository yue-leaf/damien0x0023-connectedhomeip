/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _TRACE_QUEUE_H_
#define _TRACE_QUEUE_H_

#include "trace_common.h"
#include "trace_port_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QUEUE_INFO
{
    uint16_t capacity_num;
    uint16_t curr_num_in_queue;
    uint16_t maximum_ever_num_in_queue;
    uint16_t reserved;
} QUEUE_INFO;

void trace_queue_init(uint32_t queue_num, ...);
void trace_queue(void);

#ifdef __cplusplus
}
#endif

#endif /*_TRACE_QUEUE_H_*/
