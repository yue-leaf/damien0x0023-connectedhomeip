/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#ifndef _APP_TASK_H_
#define _APP_TASK_H_

#include <string.h>

#include "os_msg.h"
#include "os_queue.h"
#include "os_sched.h"
#include "os_task.h"
#include "os_timer.h"

#include "app_msg.h"

#include "trace.h"

//#include "io_gpio.h"


/**
  * @brief      Send msg to app task.
  * @param[in]  p_handle: The handle to the message queue being peeked.
  * @return     The status of the message queue peek.
  * @retval     true:Message queue was peeked successfully.
  * @retval     false:Message queue was failed to peek.
  */
bool app_send_msg_to_apptask(uint32_t *p_send);

/**
  * @brief  Initialize app task
  * @return void
  */
void app_task_init(void);

void driver_init(void);

void app_handle_io_msg(uint32_t io_msg);

#endif

