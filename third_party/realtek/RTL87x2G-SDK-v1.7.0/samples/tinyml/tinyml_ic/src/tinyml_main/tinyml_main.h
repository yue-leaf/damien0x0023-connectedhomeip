/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef AECBA922_A8A5_4F16_8621_BD13815686F8
#define AECBA922_A8A5_4F16_8621_BD13815686F8

#include <stdbool.h>
#include "app_msg.h"

typedef void (*pfunc)(void *);
void ts_timer_start(void (func)(void *xTimer));
bool tinyml_send_msg_to_task(T_IO_MSG *p_msg);

#endif /* AECBA922_A8A5_4F16_8621_BD13815686F8 */
