/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/

#include "app_task.h"

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

#define TEST_FTL_IO_MSG_VAL      0xAA
#define TEST_OS_IO_MSG_VAL       0xAB
/*============================================================================*
 *                              Variables
 *============================================================================*/
void *app_task_handle;   //!< APP Task handle
void *evt_queue_handle;  //!< Event queue handle
void *io_queue_handle;   //!< IO queue handle

/*============================================================================*
 *                              Functions
 *============================================================================*/

void app_main_task(void *p_param);

/**
  * @brief      Send msg to app task.
  * @param[in]  p_handle: The handle to the message queue being peeked.
  * @return     The status of the message queue peek.
  * @retval     true:Message queue was peeked successfully.
  * @retval     false:Message queue was failed to peek.
  */
bool app_send_msg_to_apptask(uint32_t *p_msg)
{
    uint32_t event = EVENT_IO_TO_APP;

    if (os_msg_send(io_queue_handle, p_msg, 0) == false)
    {
        APP_PRINT_INFO0("[app_task] app_send_msg_to_apptask: send_io_msg_to_app fail");
        return false;
    }
    if (os_msg_send(evt_queue_handle, &event, 0) == false)
    {
        APP_PRINT_INFO0("[app_task] app_send_msg_to_apptask: send_evt_msg_to_app fail");
        return false;
    }
    return true;
}

/**
 * @brief  Initialize App task
 * @return void
 */
void app_task_init()
{
    os_task_create(&app_task_handle, "app", app_main_task, 0, APP_TASK_STACK_SIZE, APP_TASK_PRIORITY);
}

/**
  * @brief      App task to handle events & messages
  * @param[in]  p_param: Parameters sending to the task
  * @return     void
  */
void app_main_task(void *p_param)
{
    uint32_t event, io_msg;

    /* This task calls secure side functions. So allocate a secure context for
     * it. */
    //must locate at the first line
    os_alloc_secure_ctx(configMINIMAL_SECURE_STACK_SIZE);

    APP_PRINT_INFO0("app task");

    os_msg_queue_create(&io_queue_handle, "io queue", MAX_NUMBER_OF_IO_MESSAGE,  sizeof(uint32_t));
    os_msg_queue_create(&evt_queue_handle, "evt queue", MAX_NUMBER_OF_EVENT_MESSAGE, sizeof(uint32_t));

//    gap_start_bt_stack(evt_queue_handle, io_queue_handle, MAX_NUMBER_OF_GAP_MESSAGE);
    driver_init();
    while (true)
    {
        // Task code goes here.
        if (os_msg_recv(evt_queue_handle, &event, 0xFFFFFFFF) == true)
        {
            APP_PRINT_INFO1("app task: recv event type %u", event);
            if (event == EVENT_IO_TO_APP)
            {
                if (os_msg_recv(io_queue_handle, &io_msg, 0) == true)
                {
                    app_handle_io_msg(io_msg);
                }
            }
        }
    }
}

/** @} */ /* End of group PERIPH_APP_TASK */


