/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <string.h>
#include "os_task.h"
#include "os_mem.h"
#include "os_msg.h"
#include "app_task.h"
#include "trace.h"

/** @defgroup  PERIPH_APP_TASK Peripheral App Task
    * @brief This file handles the implementation of application task related functions.
    *
    * Create App task and handle events & messages
    * @{
    */
/*============================================================================*
 *                              Macros
 *============================================================================*/
#define APP_TASK_PRIORITY               1   /* Task priorities. */
#define APP_TASK_STACK_SIZE             (512 * 4)

#define MAX_NUMBER_OF_GAP_MESSAGE       0x20
#define MAX_NUMBER_OF_IO_MESSAGE        0x20
#define MAX_NUMBER_OF_EVENT_MESSAGE     (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE)

#define configMINIMAL_SECURE_STACK_SIZE  1024

/*============================================================================*
 *                              Variables
 *============================================================================*/
void *app_task_handle;
void *evt_queue_handle;
void *io_queue_handle;

/*============================================================================*
 *                              Functions
 *============================================================================*/
void app_main_task(void *p_param);

/**
 * \brief    send msg queue to app task.
 *
 * \param[in]   p_handle   The handle to the message queue being peeked.
 *
 * \return           The status of the message queue peek.
 * \retval true      Message queue was peeked successfully.
 * \retval false     Message queue was failed to peek.
 */
bool app_send_msg_to_apptask(uint32_t *p_msg)
{
    uint32_t event = EVENT_IO_TO_APP;

    if (os_msg_send(io_queue_handle, p_msg, 0) == false)
    {
        APP_PRINT_INFO0("send_io_msg_to_app fail");
        return false;
    }
    if (os_msg_send(evt_queue_handle, &event, 0) == false)
    {
        APP_PRINT_INFO0("send_evt_msg_to_app fail");
        return false;
    }
    return true;
}

void app_task_init(void)
{
    os_task_create(&app_task_handle, "app", app_main_task, 0, APP_TASK_STACK_SIZE,
                   APP_TASK_PRIORITY);
}

/**
* @brief
*
*
* @param   pvParameters
* @return  void
*/
void app_main_task(void *p_param)
{
    uint32_t event, io_msg;

    /* This task calls secure side functions. So allocate a secure context for
     * it. */
    //must locate at the first line
    os_alloc_secure_ctx(configMINIMAL_SECURE_STACK_SIZE);

    os_msg_queue_create(&io_queue_handle, "io queue", MAX_NUMBER_OF_IO_MESSAGE,  sizeof(uint32_t));
    os_msg_queue_create(&evt_queue_handle, "evt queue", MAX_NUMBER_OF_EVENT_MESSAGE, sizeof(uint32_t));

    driver_init();
    while (true)
    {
        if (os_msg_recv(evt_queue_handle, &event, 0xFFFFFFFF) == true)
        {
            APP_PRINT_INFO1("app task: recv event type %u", event);
            if (event == EVENT_IO_TO_APP)
            {
                if (os_msg_recv(io_queue_handle, &io_msg, 0) == true)
                {
                    APP_PRINT_INFO0("app task: recv test os io msg success!");
                }
            }
        }
    }
}

/** @} */ /* End of group PERIPH_APP_TASK */
