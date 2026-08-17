/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      app_task.c
   * @brief     Routines to create App task and handle events & messages
   * @author    jane
   * @date      2017-06-02
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <os_msg.h>
#include <os_task.h>
#include <os_sched.h>
#include <app_task.h>

/*============================================================================*
 *                              Macros
 *============================================================================*/
#define configMINIMAL_SECURE_STACK_SIZE  1024
//! Task priorities
#define APP_TASK_PRIORITY             1
//!Task stack size
#define APP_TASK_STACK_SIZE           256 * 8

/*============================================================================*
 *                              Variables
 *============================================================================*/
void *app_task_handle;   //!< APP Task handle

/*============================================================================*
 *                              Functions
 *============================================================================*/
void app_main_task(void *p_param);

/**
 * @brief  Initialize App task
 * @return void
 */
void app_task_init()
{
    os_task_create(&app_task_handle, "app", app_main_task, 0, APP_TASK_STACK_SIZE,
                   APP_TASK_PRIORITY);
}

/**
 * @brief        App task to handle events & messages
 * @param[in]    p_params    Parameters sending to the task
 * @return       void
 */
void app_main_task(void *p_param)
{
    /* This task calls secure side functions. So allocate a secure context for
     * it. */
    //must locate at the first line
    os_alloc_secure_ctx(configMINIMAL_SECURE_STACK_SIZE);

    /* avoid ppt initialization conflict with ble psd procedure */
    os_delay(1000);

    void ppt_demo(void);
    ppt_demo();

    while (1);
}

