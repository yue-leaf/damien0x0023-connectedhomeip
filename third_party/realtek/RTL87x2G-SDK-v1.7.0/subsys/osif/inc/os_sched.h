/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _OS_SCHED_H_
#define _OS_SCHED_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup    OS_SCHED Kernel Scheduler
 *
 * \brief       Manage the kernel scheduler functions.
 *
 */

/*============================================================================*
 *                              Functions
*============================================================================*/
/** @defgroup OS_SCHED_Exported_Functions OS Scheduler Exported Functions
  * \ingroup  OS_SCHED
  * @{
  */


/**
 *
 * \brief       Delay current task for a given period in milliseconds.
 *
 * \param[in]       ms  The amount of time in milliseconds that the current task
 *                  should block.
 *
 * <b>Example usage</b>
 * \code{.c}
 * // Task routine implementation.
 * void task_routine(void *p_param)
 * {
 *     for (;;)
 *     {
 *          // Task code goes here.
 *          os_delay(100);
 *
 *          // Do something here per 100ms.
 *     }
 * }
 * \endcode
 *
 */
extern void (*os_delay)(uint32_t ms);

/**
 *
 * \brief   Get the time in milliseconds since os_sched_start() API function was called. The precision of time is affected by the precision of the OS tick.
 *
 * \return  The time in milliseconds.
 *
 * <b>Example usage</b>
 * \code{.c}
 * // Task routine implementation.
 * void task_routine(void *p_param)
 * {
 *     uint64_t last_time;
 *
 *     // Get the last timestamp.
 *     last_time = os_sys_time_get();
 *
 *     for (;;)
 *     {
 *          // Wait for the next cycle.
 *          os_delay(100);
 *
 *          // Do something here per 100ms.
 *     }
 * }
 * \endcode
 *
 */
extern uint64_t (*os_sys_time_get)(void);

/**
 *
 * \brief   Get the time in OS tick counts (default 1 tick = 10ms) since os_sched_start() API function was called.
 *
 * \return  The time in OS tick counts.
 *
 * <b>Example usage</b>
 * \code{.c}
 * // Task routine implementation.
 * void task_routine(void *p_param)
 * {
 *     uint64_t last_tick;
 *
 *     // Get the last timestamp.
 *     last_tick = os_sys_tick_get();
 *
 *     for (;;)
 *     {
 *          // Wait for the next cycle.
 *          os_delay(100);
 *
 *          // Do something here per 100ms.
 *     }
 * }
 * \endcode
 *
 */
extern uint64_t (*os_sys_tick_get)(void);

/**
 *
 * \brief   Start the RTOS kernel scheduler.
 *
 * \return  The status of starting kernel scheduler. It will always return true.
 *
 * <b>Example usage</b>
 * \code{.c}
 * int test(void)
 * {
 *     // Create at least one task before starting kernel scheduler.
 *     if (os_task_create(&p_handle, "task", task_routine,
 *                        NULL, STACK_SIZE, TASK_PRIORITY) == true)
 *     {
 *         // Task was created successfully.
 *     }
 *     else
 *     {
 *         // Task creation failed.
 *         return -1;
 *     }
 *
 *     // Start the kernel scheduler.
 *     os_sched_start();
 *
 *     // Will not get here.
 * }
 * \endcode
 *
 */
extern bool (*os_sched_start)(void);

/**
 *
 * \brief   Stop the RTOS kernel scheduler. All created tasks will be automatically
 *          deleted and multitasking (either preemptive or cooperative) stops.
 *
 * \return  The status of stopping kernel scheduler. It will always return true.
 *
 * <b>Example usage</b>
 * \code{.c}
 * // Task routine implementation.
 * void task_routine(void *p_param)
 * {
 *     for (;;)
 *     {
 *          // Task code goes here.
 *
 *          // At some points, we want to end the real-time kernel processing.
 *          os_sched_stop();
 *     }
 * }
 *
 * int test(void)
 * {
 *     // Create at least one task before starting kernel scheduler.
 *     if (os_task_create(&p_handle, "task", task_routine,
 *                        NULL, STACK_SIZE, TASK_PRIORITY) == true)
 *     {
 *         // Task is created successfully.
 *     }
 *     else
 *     {
 *         // Task failed to create.
 *         return -1;
 *     }
 *
 *     // Start the kernel scheduler.
 *     os_sched_start();
 *
 *     // Will not get here unless a task calls os_sched_stop().
 * }
 * \endcode
 *
 */
extern bool (*os_sched_stop)(void);

/**
 *
 * \brief   Suspends the kernel scheduler without disabling interrupts. Context
 *          switches will not occur while the scheduler is suspended. After calling
 *          os_sched_suspend(), the calling task will continue to execute without
 *          risk of being swapped out until a call to os_sched_resume() has been made.
 *
 * \return  The status of suspending kernel scheduler. It will always return true.
 *
 * <b>Example usage</b>
 * \code{.c}
 * // Task routine implementation.
 * void task_routine(void *p_param)
 * {
 *     for (;;)
 *     {
 *          // Task code goes here.
 *
 *          // At some points, the task wants to perform a long operation, and does not
 *          // want to get swapped out.
 *          os_sched_suspend();
 *
 *          // The long operation.
 *
 *          // The operation is completed, and resume the scheduler.
 *          os_sched_resume();
 *     }
 * }
 * \endcode
 *
 */
extern bool (*os_sched_suspend)(void);

/**
 *
 * \brief   Resume the kernel scheduler after it was suspended by a call to os_sched_suspend().
 *
 * \return  The status of resuming kernel scheduler. It will always return true.
 *
 * <b>Example usage</b>
 * \code{.c}
 * // Task routine implementation.
 * void task_routine(void *p_param)
 * {
 *     for (;;)
 *     {
 *          // Task code goes here.
 *
 *          // At some points, the task wants to perform a long operation and does not
 *          // want to get swapped out.
 *          os_sched_suspend();
 *
 *          // The long operation.
 *
 *          // The operation is completed, and resume the scheduler.
 *          os_sched_resume();
 *     }
 * }
 * \endcode
 *
 */
extern bool (*os_sched_resume)(void);

/**
  * @brief  Check if OS scheduler has already been started.
  * @return The status of OS scheduler.
  * \retval  true      Scheduler has already been started.
  * \retval  false     Scheduler has not been started.
  */
extern bool (*os_sched_is_start)(void);

/**
  * @brief  Get state of OS scheduler.
  * @param[out]  p_state Current state of OS scheduler, 0: suspended; 1: not started; 2: running.
  * @return The result of OS scheduler state get.
  * \retval  true      Scheduler state get success.
  * \retval  false     Scheduler state get fail. It happens when p_state is NULL.
  */
extern bool (*os_sched_state_get)(long *p_state);

/** End of group OS_SCHED_Exported_Functions
  * @}
  */

/**
  * \cond     private
  * \defgroup OS_SCHED_Private_Functions
  * \{
  */

extern void (*os_vector_table_update)(void);

extern void (*os_init)(void);

/**
  *  End of OS_SCHED_Private_Functions
  * \}
  * \endcond
  */

/** End of OS_SCHED
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* _OS_SCHED_H_ */
