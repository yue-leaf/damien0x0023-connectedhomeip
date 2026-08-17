/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _CHATGPT_QUEUE_H_
#define _CHATGPT_QUEUE_H_

#include <os_msg.h>
#include <os_task.h>
#include <gap.h>
#include <gap_le.h>
#include <gap_msg.h>
#include <app_msg.h>
#include "trace.h"

#include "version.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct list_node
{
    struct list_node *p_next_node ;
    struct list_node *p_prev_node ;
    uint16_t data_length;
    uint8_t p_data[0];
} ts_queue_t;

void ts_queue_printf(ts_queue_t *p_list_head);
ts_queue_t *ts_queue_add_node(ts_queue_t *p_list_head, ts_queue_t *p_node);
ts_queue_t *ts_queue_indexof(ts_queue_t *p_list_head, uint16_t node_index);
ts_queue_t *ts_queue_remove_node(ts_queue_t *p_list_head, uint16_t node_index);
ts_queue_t *ts_queue_add_data(ts_queue_t *p_list_head, uint8_t *p_data, uint16_t length);
ts_queue_t *ts_queue_remove_first_node(ts_queue_t *p_list_head);
ts_queue_t *ts_queue_remove_last_node(ts_queue_t *p_list_head);
ts_queue_t *ts_queue_indexof_last(ts_queue_t *p_list_head);

void ts_queue_clear(ts_queue_t **p_list);

#ifdef __cplusplus
}
#endif

#endif

