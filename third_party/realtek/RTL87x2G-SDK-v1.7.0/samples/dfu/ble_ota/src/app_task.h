/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_TASK_H_
#define _APP_TASK_H_

#include "app_msg.h"
#include "stdbool.h"

/** @defgroup PERIPH_APP_TASK Peripheral App Task
  * @brief Peripheral App Task
  * @{
  */

extern void driver_init(void);

/**
 * @brief  Initialize App task
 * @return void
 */
void app_task_init(void);

/**
 * \brief    Send msg to app task.
 *
 * \param[in]   p_msg   Pointer to the IO message to be sent.
 *
 * \return           The status of the message send operation.
 * \retval true      Message was sent successfully.
 * \retval false     Message failed to send.
 */
bool app_send_msg_to_apptask(T_IO_MSG *p_msg);


/** End of PERIPH_APP_TASK
* @}
*/


#endif

