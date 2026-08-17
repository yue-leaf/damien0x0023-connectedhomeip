/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#ifndef __USB_ISR_H__
#define __USB_ISR_H__
#include <stdint.h>
#include "usb_task.h"

/** @defgroup USB_ISR USB ISR
  * @{
  */

/**
 * @brief USB Device driver
 * @param reset: callback to process reset interrupt
 * @param speed_enum_done: callback to process speed enum done interrupt
 * @param setup: callback to process setup packet
 * @param suspend: callback to process suspend interrupt
 * @param resume: callback to process resume interrupt
 * @param sof: callback to process SOF interrupt
 *
 */
typedef struct _usb_device_driver
{
    int (*reset)(void);
    int (*speed_enum_done)(uint8_t speed);
    int (*setup)(uint8_t *data);
    int (*suspend)(void);
    int (*resume)(void);
    int (*sof)(void);
} T_USB_DEVICE_DRIVER;

/**
 * @brief handle msg from ISR in task
 *
 * @param usb_msg: msg from ISR
 */
void usb_isr_msg_handle(T_USB_TASK_MSG *usb_msg);

/**
 * @brief enable USB interrupts
 *
 */
void usb_isr_enable(void);

/**
 * @brief Disable USB interrupts
 *
 */
void usb_isr_disable(void);

/**
 * @brief register device driver
 *
 * @param driver \ref T_USB_DEVICE_DRIVER
 * @par example
 * @code
 *      int dev_driver_reset(void)
 *      {
 *          ...
 *      }
 *
 *      int dev_driver_speed_enum_done(uint8_t speed)
 *      {
 *          ...
 *      }
 *      int dev_driver_setup(uint8_t *data);
 *      {
 *          ...
 *      }
 *
 *      int dev_driver_suspend(void)
 *      {
 *          ...
 *      }
 *
  *     int dev_driver_resume(void)
 *      {
 *          ...
 *      }
 *
 *      T_USB_DEVICE_DRIVER device_driver =
 *      {
 *          .reset = dev_driver_reset,
 *          .speed_enum_done = dev_driver_speed_enum_done,
 *          .setup = dev_driver_setup,
 *          .suspend = dev_driver_suspend,
 *          .resume = dev_driver_resume,
 *      };
 *      usb_isr_device_driver_register(&device_driver);
 * @endcode
 */
void usb_isr_device_driver_register(T_USB_DEVICE_DRIVER *driver);

/**
 * @brief unregister device driver
 *
 * @param driver \ref T_USB_DEVICE_DRIVER
 */
void usb_isr_device_driver_unregister(T_USB_DEVICE_DRIVER *driver);

/**
 * \brief usb isr priority
 *
 * @param PriorityGroup  Priority grouping field.
 */
void usb_isr_set_priority(uint8_t priority);
/** @}*/
/** End of USB_ISR
*/
#endif
