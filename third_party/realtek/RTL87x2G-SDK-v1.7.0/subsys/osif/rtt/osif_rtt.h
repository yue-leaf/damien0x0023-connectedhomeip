/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _OSIF_RTT_H_
#define _OSIF_RTT_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cmsis_compiler.h"
#include "mem_types.h"
#include "rtthread.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
extern void os_mem_func_init(void);
extern void os_msg_func_init(void);
extern void os_pool_func_init(void);
extern void os_queue_func_init(void);
extern void os_sched_func_init(void);
extern void os_sync_func_init(void);
extern void os_task_func_init(void);
extern void os_timer_func_init(void);
extern void os_trace_func_init(void);
#endif

struct exception_stack_frame
{
    rt_uint32_t r0;
    rt_uint32_t r1;
    rt_uint32_t r2;
    rt_uint32_t r3;
    rt_uint32_t r12;
    rt_uint32_t lr;
    rt_uint32_t pc;
    rt_uint32_t psr;
};

struct stack_frame
{
    rt_uint32_t tz;
    rt_uint32_t lr;
    rt_uint32_t psplim;
    rt_uint32_t control;

    /* r4 ~ r11 register */
    rt_uint32_t r4;
    rt_uint32_t r5;
    rt_uint32_t r6;
    rt_uint32_t r7;
    rt_uint32_t r8;
    rt_uint32_t r9;
    rt_uint32_t r10;
    rt_uint32_t r11;

    struct exception_stack_frame exception_stack_frame;
};

/* Cortex M33 port configuration. */
#define configENABLE_MPU                                0
#define configENABLE_FPU                                1
#define configENABLE_TRUSTZONE                          1

typedef enum
{
    SCHEDULER_SUSPENDED = 0,
    SCHEDULER_NOT_STARTED = 1,
    SCHEDULER_RUNNING = 2
} SCHEDULER_STATE;

enum
{
    RT_TIMER_CMD_CB = 0u,
    RT_TIMER_CMD_START,
    RT_TIMER_CMD_STOP,
    RT_TIMER_CMD_RESTART,
    RT_TIMER_CMD_DEL,
    RT_TIMER_EXCUTE_CALLBACK
};

typedef void (*PendedFunctionOS_t)(void *para1, uint32_t para2);

typedef struct
{
    rt_timer_t timer;
    uint8_t    cb_num;
    void *change_tick;
    PendedFunctionOS_t  pxCallbackFunction;
    void    *pend_para1;
    uint32_t pend_para2;
} rt_timer_queue_cb;

#define TIMER_CMD_QUEUE_LENGTH                          32
#define TIMER_NUMBER_MAX                                32  // (app_sys_cfg.os_cfg.timerMaxNumber)

extern rt_mq_t rt_timer_queue;
extern rt_thread_t rt_timer_task;

typedef struct
{
    void *task_handle;
    void *sem_handle;
} task_sem_item;

#define TASK_SEM_ARRAY_NUMBER                            5

#define configASSERT( x )                               if( ( x ) == 0 )  { vAssertHandler(__FUNCTION__, __LINE__); }

#ifdef __cplusplus
}
#endif

#endif /* _OSIF_RTT_H_ */

