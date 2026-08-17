/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#ifndef __USB_DM_INT_H__
#define __USB_DM_INT_H__
#include "usb_dm.h"
#include "usb_task.h"

T_USB_POWER_STATE usb_dm_state_get(void);
int usb_dm_state_set(T_USB_POWER_STATE state);
int usb_dm_suspend_enter(void);
int usb_dm_suspend_exit(void);
int usb_dm_msg_handle(T_USB_TASK_MSG *msg);
int usb_dm_remote_wakeup_enable(void);
int usb_dm_remote_wakeup_disable(void);
#endif
