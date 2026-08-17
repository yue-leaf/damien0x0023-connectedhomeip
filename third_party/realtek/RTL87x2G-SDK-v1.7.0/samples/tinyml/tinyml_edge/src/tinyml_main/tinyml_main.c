/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "cmsis_compiler.h"
#include "trace.h"
#include "model_process.h"
#include "os_sched.h"
#include "io_uart.h"
#include "ts_queue.h"
#include "ts_mem.h"
#include <os_msg.h>
#include <os_task.h>
#include <app_msg.h>
#include "os_timer.h"
#include "tinyml_main.h"
#include "uart_packet_parser.h"
#include "ts_queue.h"
#include "ts_mem.h"
#include "utils.h"
#include "app_section.h"
#include "clock.h"
#include "ts_realtek.h"

/*============================================================================*
 *                              Macros
 *============================================================================*/
#define TinyML_TASK_PRIORITY             2         //!< Task priorities
#define TinyML_TASK_STACK_SIZE           512 * 20   //!<  Task stack size
#define MAX_NUMBER_OF_IO_MESSAGE             0x30      //!<  IO message queue size

/*============================================================================*
 *                              Variables
 *============================================================================*/
void *TinyML_task_handle;   //!< APP Task handle
void *tinyml_queue_handle;   //!< IO queue handle

void *xUartTimeoutTimerHandle = NULL;


/*============================================================================*
 *                              Functions
 *============================================================================*/
void TinyML_main_task(void *p_param);
void ts_timer_cb(void *xTimer);

pfunc ts_timer_timeout_func = NULL;

/**
 * @brief  Initialize App task
 * @return void
 */

void TinyML_task_init()
{
    os_task_create(&TinyML_task_handle, "app", TinyML_main_task, 0, TinyML_TASK_STACK_SIZE,
                   TinyML_TASK_PRIORITY);
}

bool tinyml_send_msg_to_task(T_IO_MSG *p_msg)
{
    if (os_msg_send(tinyml_queue_handle, p_msg, 0) == false)
    {
        DBG_DIRECT("tinyml_send_msg_to_task fail");
        return false;
    }
    else
    {
        DBG_DIRECT("tinyml_send_msg_to_task success, type=%d, subtype=%d", p_msg->type, p_msg->subtype);
    }

    return true;
}


/**
 * @brief        App task to handle events & messages
 * @param[in]    p_param    Parameters sending to the task
 * @return       void
 */
void TinyML_main_task(void *p_param)
{
    T_IO_MSG event;
    uint32_t actual_mhz = 0;
    pm_cpu_freq_set(125, &actual_mhz);
    DBG_DIRECT("CPU actual clock is %d MHz", actual_mhz);

    dwt_cyccnt_init();
    uart_init();

    /* Register the CPU frequency so the MicroProfiler can convert DWT
     * cycle counts to milliseconds (125 MHz = 125000000 Hz). */
    ts_realtek_register_cpu_freq_hz(125000000);

    os_msg_queue_create(&tinyml_queue_handle, "TSQ", MAX_NUMBER_OF_IO_MESSAGE, sizeof(T_IO_MSG));

    bool retval = false;
    retval = os_timer_create(&xUartTimeoutTimerHandle, "timeout Timer",  1, 8000, false, ts_timer_cb);
    if (!retval)
    {
        APP_PRINT_INFO1("create xUartTimeoutTimer retval=%d", retval);
    }
    while (true)
    {

        if (os_msg_recv(tinyml_queue_handle, &event, 0xFFFFFFFF/*0xFFFFFFFF*/) == true)
        {
            if (event.subtype == IO_MSG_UART_RX_DONE)
            {
                DBG_DIRECT("IO_MSG_UART_RX_DONE");
                extern ts_queue_t *g_ts_uart_data_list;

                ts_queue_t *p_last_node = ts_queue_indexof_last(g_ts_uart_data_list);
                while (p_last_node != NULL)
                {
                    uart_rx_data_parse(p_last_node->p_data, p_last_node->data_length);
                    g_ts_uart_data_list = ts_queue_remove_last_node(g_ts_uart_data_list);
                    p_last_node = ts_queue_indexof_last(g_ts_uart_data_list);
                }
            }
            else if (event.subtype == IO_MSG_UART_DATA_PARSER_SUCCESS)
            {
                DBG_DIRECT("IO_MSG_UART_DATA_PARSER_SUCCESS");

            }
            else if (event.subtype == IO_MSG_UART_DATA_PARSER_FAILED)
            {
                DBG_DIRECT("IO_MSG_UART_DATA_PARSER_FAILED,retry");
            }
        }

    }
}

void ts_timer_cb(void *xTimer)
{
    if (ts_timer_timeout_func != NULL)
    {
        // Call the registered timeout function
        ts_timer_timeout_func(xTimer);
        ts_timer_timeout_func = NULL; // Clear the function after calling it
    }
    else
    {
        DBG_DIRECT("No timeout function registered");
    }

}

void ts_timer_start(void (func)(void *xTimer))
{
    // Start the timer for timeout events
    // This function can be used to handle timeout events in the TinyML task
    // For example, it can be used to reset the UART state or handle other timeout-related logic
    //DBG_DIRECT("Timer started");
    ts_timer_timeout_func = func; // Register the timeout function
    os_timer_start(&xUartTimeoutTimerHandle);
}

void ts_timeout_callback_register(void (*cb)(uint32_t time))
{
    // Register the callback function for timeout events
    // This function can be used to handle timeout events in the TinyML task
    // For example, it can be used to reset the UART state or handle other timeout-related logic
    DBG_DIRECT("Timeout callback registered");
}

