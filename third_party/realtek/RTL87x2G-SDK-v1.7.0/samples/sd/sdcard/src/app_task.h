/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include "board.h"

extern void *app_task_handle;
extern void *evt_queue_handle;
extern void *io_queue_handle;

/**  @brief Event type definitions.*/
typedef enum
{
    EVENT_GAP_MSG = 0x01,  /**< message from gap layer for stack */
    EVENT_IO_TO_APP = 0x02, /**< message from IO to user application */
} T_EVENT_TYPE;

extern void driver_init(void);

bool app_send_msg_to_apptask(uint32_t *p_send);

void app_task_init(void);



