/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#ifndef __USB_TASK_H__
#define __USB_TASK_H__
#include <stdint.h>

/** @defgroup USB_Task USB Task
  * @{
  */

/**
 * @brief msg group that send from isr defined in \ref T_HAL_USB_COMMON_ISR
 *
 */
#define USB_TASK_MSG_GROUP_HAL      0
#define USB_TASK_MSG_GROUP_DM       1

/**
 * @brief USB task message
 *
 */
typedef struct _usb_task_msg
{
    uint8_t group;
    uint8_t type;

    union
    {
        struct
        {
            uint8_t *buf;
            uint16_t len;
        } complex;
        uint32_t var[2];
    };

} T_USB_TASK_MSG;

/**
 * @brief send message to USB task
 *
 * @param msg: msg to send
 * @return int result, refer to `rtl_errno.h`
 */
int usb_task_msg_send(T_USB_TASK_MSG *msg);

/**
 * @brief create USB task
 *
 * @return int
 */
int usb_task_create(void);

/**
 * @brief release USB task
 *
 * @return int
 */
int usb_task_release(void);

/** @}*/
/** End of USB_Task
*/
#endif // !__USB_TASK_H__
