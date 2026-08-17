/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include <string.h>
#include "app_section.h"
#include "os_cfg.h"
#include "cmsis_compiler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "queue.h"
#include "osif.h"
#include "osif_freertos.h"
#include "os_queue.h"
#include "os_mem.h"
#include "os_timer.h"
#include "dlps.h"
#include "os_pm.h"
#include "secure_context.h"
#include "system_rtl876x.h"
#include "utils.h"
#include "trace.h"

T_OS_QUEUE lpm_excluded_handle[PLATFORM_PM_EXCLUDED_TYPE_MAX] =
{
    [0 ...(PLATFORM_PM_EXCLUDED_TYPE_MAX - 1)] = {
        .p_first = NULL,
        .p_last = NULL,
        .count = 0,
    },
};

/* Check if in task context (true), or isr context (false)                  */
/****************************************************************************/
__STATIC_INLINE bool osif_task_context_check(void)
{
    return (__get_IPSR() == 0);
}

/****************************************************************************/
/* Delay current task in a given milliseconds                               */
/****************************************************************************/
void osif_delay(uint32_t ms)
{
    vTaskDelay((TickType_t)((ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS));
}

/****************************************************************************/
/* Get system time in milliseconds                                          */
/****************************************************************************/
uint64_t osif_sys_time_get(void)
{
    if (osif_task_context_check() == true)
    {
        return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
    }
    else
    {
        return (uint32_t)(xTaskGetTickCountFromISR() * portTICK_PERIOD_MS);
    }
}

/****************************************************************************/
/* Get system tick                                                          */
/****************************************************************************/
uint64_t osif_sys_tick_get(void)
{
    if (osif_task_context_check() == true)
    {
        return (uint32_t)(xTaskGetTickCount());
    }
    else
    {
        return (uint32_t)(xTaskGetTickCountFromISR());
    }
}

/****************************************************************************/
/* Start os kernel scheduler                                                */
/****************************************************************************/
OS_EXE_TYPE osif_sched_start(void)
{
    vTaskStartScheduler();

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Stop os kernel scheduler                                                 */
/****************************************************************************/
OS_EXE_TYPE osif_sched_stop(void)
{
    vTaskEndScheduler();

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Suspend os kernel scheduler                                              */
/****************************************************************************/
OS_EXE_TYPE osif_sched_suspend(void)
{
    vTaskSuspendAll();

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Resume os kernel scheduler                                               */
/****************************************************************************/
OS_EXE_TYPE osif_sched_resume(void)
{
    xTaskResumeAll();

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/*Get os kernel scheduler  state                                          */
/****************************************************************************/
OS_EXE_TYPE osif_sched_state_get(long *p_state)
{
    if (p_state == NULL)
    {
        return OS_EXE_ERROR_STATE_IS_NULL;
    }

    *p_state = xTaskGetSchedulerState();

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/*Check os kernel scheduler is started                                      */
/****************************************************************************/
bool osif_sched_is_start(void)
{
    if (taskSCHEDULER_NOT_STARTED == xTaskGetSchedulerState())
    {
        return false;
    }
    else
    {
        return true;
    }
}

/****************************************************************************/
/*Update os kernel vector table                                            */
/****************************************************************************/
void osif_vector_table_update(void)
{
    return;
}

/****************************************************************************/
/*os kernel init                                                            */
/****************************************************************************/
void osif_init(void)
{
    extern void HeapConfigInit(void);
    HeapConfigInit();
}

/****************************************************************************/
/* Create os secure task stack so that the task can call NSC APIs           */
/****************************************************************************/
OS_EXE_TYPE osif_alloc_secure_ctx(uint32_t stack_size)
{
    portALLOCATE_SECURE_CONTEXT(stack_size);

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Create os level task routine                                             */
/****************************************************************************/
OS_EXE_TYPE osif_task_create(void **pp_handle, const char *p_name, void (*p_routine)(void *),
                             void *p_param, uint16_t stack_size, uint16_t priority)
{
    BaseType_t ret;

    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    ret = xTaskCreate(p_routine, (const char *)p_name, stack_size / sizeof(portSTACK_TYPE),
                      p_param, priority, (TaskHandle_t *)pp_handle);
    if (ret == pdPASS)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Delete os level task routine                                             */
/****************************************************************************/
OS_EXE_TYPE osif_task_delete(void *p_handle)
{
    vTaskDelete((TaskHandle_t)p_handle);

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Suspend os level task routine                                            */
/****************************************************************************/
OS_EXE_TYPE osif_task_suspend(void *p_handle)
{
    vTaskSuspend((TaskHandle_t)p_handle);

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Resume os level task routine                                             */
/****************************************************************************/
OS_EXE_TYPE osif_task_resume(void *p_handle)
{
    vTaskResume((TaskHandle_t)p_handle);

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Yield current os level task routine                                      */
/****************************************************************************/
OS_EXE_TYPE osif_task_yield(void)
{
    taskYIELD();

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Get current os level task routine handle                                 */
/****************************************************************************/
OS_EXE_TYPE osif_task_handle_get(void **pp_handle)
{
    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    *pp_handle = (void *)xTaskGetCurrentTaskHandle();

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Get os level task routine priority                                       */
/****************************************************************************/
OS_EXE_TYPE osif_task_priority_get(void *p_handle, uint16_t *p_priority)
{
    if (p_priority == NULL)
    {
        return OS_EXE_ERROR_PRIORITY_IS_NULL;
    }

    *p_priority = uxTaskPriorityGet((TaskHandle_t)p_handle);

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Set os level task routine priority                                       */
/****************************************************************************/
OS_EXE_TYPE osif_task_priority_set(void *p_handle, uint16_t priority)
{
    vTaskPrioritySet((TaskHandle_t)p_handle, priority);

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Create signal to target task                                               */
/****************************************************************************/
OS_EXE_TYPE osif_task_signal_create(void *p_handle, uint32_t count)
{
    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Send signal to target task                                               */
/****************************************************************************/
OS_EXE_TYPE osif_task_signal_send(void *p_handle, uint32_t signal)
{
    BaseType_t ret;

    if (osif_task_context_check() == true)
    {
        ret = xTaskNotify((TaskHandle_t)p_handle, signal, eSetBits);
    }
    else
    {
        BaseType_t task_woken = pdFALSE;

        ret = xTaskNotifyFromISR((TaskHandle_t)p_handle, signal,
                                 eSetBits, &task_woken);

        portEND_SWITCHING_ISR(task_woken);
    }

    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Receive signal in target task                                            */
/****************************************************************************/
OS_EXE_TYPE osif_task_signal_recv(uint32_t *p_signal, uint32_t wait_ms)
{
    TickType_t wait_ticks;
    BaseType_t ret;

    if (wait_ms == 0xFFFFFFFFUL)
    {
        wait_ticks = portMAX_DELAY;
    }
    else
    {
        wait_ticks = (TickType_t)((wait_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS);
    }

    /* task signal should not be received in ISR */
    ret = xTaskNotifyWait(0x00000000UL, 0xFFFFFFFFUL, p_signal, wait_ticks);
    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Take signal in target task                                              */
/****************************************************************************/
OS_EXE_TYPE osif_task_signal_take(uint32_t p_signal, uint32_t wait_ms)
{
    BaseType_t ret;

    TickType_t wait_ticks;
    if (wait_ms == 0xFFFFFFFFUL)
    {
        wait_ticks = portMAX_DELAY;
    }
    else
    {
        wait_ticks = (TickType_t)((wait_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS);
    }

    /* task signal should not be received in ISR */
    ret = ulTaskNotifyTake(p_signal, wait_ticks);
    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }

}

/****************************************************************************/
/* Give signal in target task                                              */
/****************************************************************************/
OS_EXE_TYPE osif_task_signal_give(void *p_handle)
{
    BaseType_t ret = false;

    if (osif_task_context_check() == true)
    {
        ret = xTaskNotifyGive((TaskHandle_t)p_handle);
    }
    else
    {
        BaseType_t task_woken = pdFALSE;

        vTaskNotifyGiveFromISR((TaskHandle_t)p_handle,  &task_woken);

        portEND_SWITCHING_ISR(task_woken);
    }

    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Clear signal in target task                                              */
/****************************************************************************/
OS_EXE_TYPE osif_task_signal_clear(void *p_handle)
{
    BaseType_t ret;

    ret = xTaskNotifyStateClear((TaskHandle_t)p_handle);
    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Take notify in target task                                              */
/****************************************************************************/
OS_EXE_TYPE osif_task_notify_take(long xClearCountOnExit, uint32_t xTicksToWait,
                                  uint32_t *p_notify)
{
    if (p_notify == NULL)
    {
        return OS_EXE_ERROR_NOTIFY_IS_NULL;
    }

    *p_notify = ulTaskNotifyTake((BaseType_t)xClearCountOnExit, (TickType_t)xTicksToWait);

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Give notify in target task                                              */
/****************************************************************************/
OS_EXE_TYPE osif_task_notify_give(void *p_handle)
{
    BaseType_t ret;

    if (osif_task_context_check() == true)
    {
        ret = xTaskNotifyGive((TaskHandle_t)p_handle);

        if (ret == pdFALSE)
        {
            return OS_EXE_FAIL;
        }
    }
    else
    {
        BaseType_t task_woken = pdFALSE;

        vTaskNotifyGiveFromISR((TaskHandle_t)p_handle, &task_woken);

        portEND_SWITCHING_ISR(task_woken);
    }
    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Dump current all task status                                             */
/****************************************************************************/
void osif_task_status_dump(void)
{
    extern void vTaskStatusDump(void);
    vTaskStatusDump();
}

/****************************************************************************/
/* Lock critical section                                                    */
/****************************************************************************/
uint32_t osif_lock(void)
{
    uint32_t flags = 0U;

    if (osif_task_context_check() == true)
    {
        taskENTER_CRITICAL();
    }
    else
    {
        flags = taskENTER_CRITICAL_FROM_ISR();
    }

    return flags;
}

/****************************************************************************/
/* Unlock critical section                                                  */
/****************************************************************************/
void osif_unlock(uint32_t flags)
{
    if (osif_task_context_check() == true)
    {
        taskEXIT_CRITICAL();
    }
    else
    {
        taskEXIT_CRITICAL_FROM_ISR(flags);
    }
}

/****************************************************************************/
/* Create counting semaphore                                                */
/****************************************************************************/
OS_EXE_TYPE osif_sem_create(void **pp_handle, const char *p_name, uint32_t init_count,
                            uint32_t max_count)
{
    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    *pp_handle = (void *)xSemaphoreCreateCounting(max_count, init_count);
    if (*pp_handle != NULL)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Delete counting semaphore                                                */
/****************************************************************************/
OS_EXE_TYPE osif_sem_delete(void *p_handle)
{
    vSemaphoreDelete((QueueHandle_t)p_handle);

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Take counting semaphore                                                  */
/****************************************************************************/
OS_EXE_TYPE osif_sem_take(void *p_handle, uint32_t wait_ms)
{
    BaseType_t ret;

    if (osif_task_context_check() == true)
    {
        TickType_t wait_ticks;

        if (wait_ms == 0xFFFFFFFFUL)
        {
            wait_ticks = portMAX_DELAY;
        }
        else
        {
            wait_ticks = (TickType_t)((wait_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS);
        }

        ret = xSemaphoreTake((QueueHandle_t)p_handle, wait_ticks);
    }
    else
    {
        BaseType_t task_woken = pdFALSE;

        ret = xSemaphoreTakeFromISR((QueueHandle_t)p_handle, &task_woken);

        portEND_SWITCHING_ISR(task_woken);
    }

    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Give counting semaphore                                                  */
/****************************************************************************/
OS_EXE_TYPE osif_sem_give(void *p_handle)
{
    BaseType_t ret;

    if (osif_task_context_check() == true)
    {
        ret = xSemaphoreGive((QueueHandle_t)p_handle);
    }
    else
    {
        BaseType_t task_woken = pdFALSE;

        ret = xSemaphoreGiveFromISR((QueueHandle_t)p_handle, &task_woken);

        portEND_SWITCHING_ISR(task_woken);
    }

    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Create recursive mutex                                                   */
/****************************************************************************/
OS_EXE_TYPE osif_mutex_create(void **pp_handle)
{
    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    *pp_handle = (void *)xSemaphoreCreateRecursiveMutex();
    if (*pp_handle != NULL)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Delete recursive mutex                                                   */
/****************************************************************************/
OS_EXE_TYPE osif_mutex_delete(void *p_handle)
{
    if (xSemaphoreGetMutexHolder((QueueHandle_t)p_handle) == NULL)
    {
        vSemaphoreDelete((QueueHandle_t)p_handle);
        return OS_EXE_SUCCESS;
    }
    else
    {
        /* Do not delete mutex if held by a task */
        return OS_EXE_ERROR_MUTEX_HOLDER_NOT_NULL;
    }
}

/****************************************************************************/
/* Take recursive mutex                                                     */
/****************************************************************************/
OS_EXE_TYPE osif_mutex_take(void *p_handle, uint32_t wait_ms)
{
    TickType_t wait_ticks;
    BaseType_t ret;

    if (wait_ms == 0xFFFFFFFFUL)
    {
        wait_ticks = portMAX_DELAY;
    }
    else
    {
        wait_ticks = (TickType_t)((wait_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS);
    }

    ret = xSemaphoreTakeRecursive((QueueHandle_t)p_handle, wait_ticks);
    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Give recursive mutex                                                     */
/****************************************************************************/
OS_EXE_TYPE osif_mutex_give(void *p_handle)
{
    BaseType_t ret;

    ret = xSemaphoreGiveRecursive((QueueHandle_t)p_handle);
    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Create inter-thread message queue                                        */
/****************************************************************************/
OS_EXE_TYPE osif_msg_queue_create(void **pp_handle, const char *p_name, uint32_t msg_num,
                                  uint32_t msg_size)
{
    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    *pp_handle = (void *)xQueueCreate(msg_num, msg_size);
    if (*pp_handle != NULL)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Delete inter-thread message queue                                        */
/****************************************************************************/
OS_EXE_TYPE osif_msg_queue_delete(void *p_handle)
{
    vQueueDelete((QueueHandle_t)p_handle);

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Peek inter-thread message queue's pending but not received msg number    */
/****************************************************************************/
OS_EXE_TYPE osif_msg_queue_peek(void *p_handle, uint32_t *p_msg_num)
{
    if (osif_task_context_check() == true)
    {
        *p_msg_num = (uint32_t)uxQueueMessagesWaiting((QueueHandle_t)p_handle);
    }
    else
    {
        *p_msg_num = (uint32_t)uxQueueMessagesWaitingFromISR((QueueHandle_t)p_handle);
    }

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Send inter-thread message                                                */
/****************************************************************************/
OS_EXE_TYPE osif_msg_send(void *p_handle, void *p_msg, uint32_t wait_ms)
{
    BaseType_t ret;

    if (osif_task_context_check() == true)
    {
        TickType_t wait_ticks;

        if (wait_ms == 0xFFFFFFFFUL)
        {
            wait_ticks = portMAX_DELAY;
        }
        else
        {
            wait_ticks = (TickType_t)((wait_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS);
        }

        ret = xQueueSendToBack((QueueHandle_t)p_handle, p_msg, wait_ticks);
    }
    else
    {
        BaseType_t task_woken = pdFALSE;

        ret = xQueueSendToBackFromISR((QueueHandle_t)p_handle, p_msg, &task_woken);

        portEND_SWITCHING_ISR(task_woken);
    }

    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Receive inter-thread message                                             */
/****************************************************************************/
OS_EXE_TYPE osif_msg_recv(void *p_handle, void *p_msg, uint32_t wait_ms)
{
    BaseType_t ret;

    if (osif_task_context_check() == true)
    {
        TickType_t wait_ticks;

        if (wait_ms == 0xFFFFFFFFUL)
        {
            wait_ticks = portMAX_DELAY;
        }
        else
        {
            wait_ticks = (TickType_t)((wait_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS);
        }

        ret = xQueueReceive((QueueHandle_t)p_handle, p_msg, wait_ticks);
    }
    else
    {
        BaseType_t task_woken = pdFALSE;

        ret = xQueueReceiveFromISR((QueueHandle_t)p_handle, p_msg, &task_woken);

        portEND_SWITCHING_ISR(task_woken);
    }

    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Peek inter-thread message                                                */
/****************************************************************************/
OS_EXE_TYPE osif_msg_peek(void *p_handle, void *p_msg, uint32_t wait_ms)
{
    BaseType_t ret;

    if (osif_task_context_check() == true)
    {
        TickType_t wait_ticks;

        if (wait_ms == 0xFFFFFFFFUL)
        {
            wait_ticks = portMAX_DELAY;
        }
        else
        {
            wait_ticks = (TickType_t)((wait_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS);
        }

        ret = xQueuePeek((QueueHandle_t)p_handle, p_msg, wait_ticks);
    }
    else
    {
        ret = xQueuePeekFromISR((QueueHandle_t)p_handle, p_msg);
    }

    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Allocate memory                                                          */
/****************************************************************************/
void *osif_mem_alloc(RAM_TYPE ram_type, size_t size)
{
    return pvPortMalloc(ram_type, size);
}

/****************************************************************************/
/* Allocate aligned memory                                                  */
/****************************************************************************/
void *osif_mem_aligned_alloc(RAM_TYPE ram_type, size_t size, uint8_t alignment)
{
    void *p;
    void *p_aligned;

    if (alignment == 0)
    {
        alignment = portBYTE_ALIGNMENT;
    }

    p = pvPortMalloc(ram_type, size + sizeof(void *) + alignment);
    if (p == NULL)
    {
        return p;
    }

    p_aligned = (void *)(((size_t)p + sizeof(void *) + alignment) & ~(alignment - 1));

    memcpy((uint8_t *)p_aligned - sizeof(void *), &p, sizeof(void *));

    return p_aligned;
}

/****************************************************************************/
/* Free memory                                                              */
/****************************************************************************/
void osif_mem_free(void *p_block)
{
    vPortFree(p_block);
}

/****************************************************************************/
/* Free aligned memory                                                      */
/****************************************************************************/
void osif_mem_aligned_free(void *p_block)
{
    void *p;

    memcpy(&p, (uint8_t *)p_block - sizeof(void *), sizeof(void *));

    vPortFree(p);
}

/****************************************************************************/
/* Peek unused (available) memory size                                      */
/****************************************************************************/
size_t osif_mem_peek(RAM_TYPE ram_type)
{
    return xPortGetFreeHeapSize(ram_type);
}

size_t osif_mem_peek_max_free_block(RAM_TYPE ram_type)
{
    extern size_t get_max_free_block_size(RAM_TYPE ram_type);
    return get_max_free_block_size(ram_type);
}

/****************************************************************************/
/* Check os heap usage                                                      */
/****************************************************************************/
void osif_mem_check_heap_usage(void)
{
    extern void check_heap_usage(void);
    check_heap_usage();
}

/****************************************************************************/
/* Get software timer ID                                                    */
/****************************************************************************/
OS_EXE_TYPE osif_timer_id_get(void **pp_handle, uint32_t *p_timer_id)
{
    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    if (*pp_handle == NULL)
    {
        return OS_EXE_ERROR_PTR_HANDLE_IS_NULL;
    }

    *p_timer_id = (uint32_t)pvTimerGetTimerID((TimerHandle_t)(*pp_handle));

    return OS_EXE_SUCCESS;
}


/****************************************************************************/
/* Create software timer                                                    */
/****************************************************************************/
OS_EXE_TYPE osif_timer_create(void **pp_handle, const char *p_timer_name, uint32_t timer_id,
                              uint32_t interval_ms, bool reload, void (*p_timer_callback)(void *))
{
    TickType_t timer_ticks;

    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    if (p_timer_callback == NULL)
    {
        return OS_EXE_ERROR_TIMER_CALLBACK_IS_NULL;
    }

    if (interval_ms == 0)
    {
        interval_ms = portTICK_PERIOD_MS;
    }

    timer_ticks = (TickType_t)((interval_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS);

    if (*pp_handle == NULL)
    {
        *pp_handle = xTimerCreate(p_timer_name, timer_ticks, (BaseType_t)reload,
                                  (void *)timer_id, (TimerCallbackFunction_t)p_timer_callback);
        if (*pp_handle == NULL)
        {
            return OS_EXE_FAIL;
        }
    }
    else
    {
        return OS_EXE_ERROR_PTR_HANDLE_NOT_NULL;
    }

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Start software timer                                                     */
/****************************************************************************/
OS_EXE_TYPE osif_timer_start(void **pp_handle)
{
    BaseType_t ret;

    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    if (*pp_handle == NULL)
    {
        return OS_EXE_ERROR_PTR_HANDLE_IS_NULL;
    }

    if (osif_task_context_check() == true)
    {
        ret = xTimerStart((TimerHandle_t)(*pp_handle), (TickType_t)0);
    }
    else
    {
        BaseType_t task_woken = pdFALSE;

        ret = xTimerStartFromISR((TimerHandle_t)(*pp_handle), &task_woken);

        portEND_SWITCHING_ISR(task_woken);
    }

    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Restart software timer                                                   */
/****************************************************************************/
OS_EXE_TYPE osif_timer_restart(void **pp_handle, uint32_t interval_ms)
{
    TickType_t timer_ticks;
    BaseType_t ret;

    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    if (*pp_handle == NULL)
    {
        return OS_EXE_ERROR_PTR_HANDLE_IS_NULL;
    }

    if (interval_ms == 0)
    {
        interval_ms = portTICK_PERIOD_MS;
    }

    timer_ticks = (TickType_t)((interval_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS);

    if (osif_task_context_check() == true)
    {
        ret = xTimerChangePeriod((TimerHandle_t)(*pp_handle), timer_ticks, (TickType_t)0);
    }
    else
    {
        BaseType_t task_woken = pdFALSE;

        ret = xTimerChangePeriodFromISR((TimerHandle_t)(*pp_handle), timer_ticks, &task_woken);

        portEND_SWITCHING_ISR(task_woken);
    }

    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Stop software timer                                                      */
/****************************************************************************/
OS_EXE_TYPE osif_timer_stop(void **pp_handle)
{
    BaseType_t ret;

    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    if (*pp_handle == NULL)
    {
        return OS_EXE_ERROR_PTR_HANDLE_IS_NULL;
    }

    if (osif_task_context_check() == true)
    {
        ret = xTimerStop((TimerHandle_t)(*pp_handle), (TickType_t)0);
    }
    else
    {
        BaseType_t task_woken = pdFALSE;

        ret = xTimerStopFromISR((TimerHandle_t)(*pp_handle), &task_woken);

        portEND_SWITCHING_ISR(task_woken);
    }

    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Delete software timer                                                    */
/****************************************************************************/
OS_EXE_TYPE osif_timer_delete(void **pp_handle)
{
    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    if (*pp_handle == NULL)
    {
        return OS_EXE_ERROR_PTR_HANDLE_IS_NULL;
    }

    if (xTimerDelete((TimerHandle_t)(*pp_handle), (TickType_t)0) == pdFAIL)
    {
        return OS_EXE_FAIL;
    }

    *pp_handle = NULL;

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Pend function call software timer                                       */
/****************************************************************************/
OS_EXE_TYPE osif_timer_pend_function_call(void (*p_pend_function)(void *, uint32_t),
                                          void *pvParameter1, uint32_t ulParameter2)
{
    BaseType_t ret;

    if (osif_task_context_check() == true)
    {
        ret = xTimerPendFunctionCall(p_pend_function, pvParameter1, ulParameter2, (TickType_t)0);
    }
    else
    {
        BaseType_t task_woken = pdFALSE;

        ret = xTimerPendFunctionCallFromISR(p_pend_function, pvParameter1, ulParameter2, &task_woken);

        portEND_SWITCHING_ISR(task_woken);
    }

    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Get software timer is active                                                */
/****************************************************************************/
OS_EXE_TYPE osif_timer_is_timer_active(void **pp_handle)
{
    BaseType_t ret;

    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    if (*pp_handle == NULL)
    {
        return OS_EXE_ERROR_PTR_HANDLE_IS_NULL;
    }

    ret = xTimerIsTimerActive((TimerHandle_t)(*pp_handle));

    if (ret == pdTRUE)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Get software timer state                                                */
/****************************************************************************/
OS_EXE_TYPE osif_timer_state_get(void **pp_handle, uint32_t *p_timer_state)
{
    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    if (*pp_handle == NULL)
    {
        return OS_EXE_ERROR_PTR_HANDLE_IS_NULL;
    }

    if (p_timer_state == NULL)
    {
        return OS_EXE_ERROR_STATE_IS_NULL;
    }

    *p_timer_state = (uint32_t)xTimerIsTimerActive((TimerHandle_t) * pp_handle);

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Get software timer is period timer or not                                */
/****************************************************************************/
OS_EXE_TYPE osif_timer_get_auto_reload(void **pp_handle, long *p_autoreload)
{
    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    if (*pp_handle == NULL)
    {
        return OS_EXE_ERROR_PTR_HANDLE_IS_NULL;
    }

    if (p_autoreload == NULL)
    {
        return OS_EXE_ERROR_AUTORELOAD_IS_NULL;
    }

    *p_autoreload = uxTimerGetReloadMode((TimerHandle_t)(*pp_handle));

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Dump software timer                                                      */
/****************************************************************************/
OS_EXE_TYPE osif_timer_dump(void)
{
    dumpAllUsedTimer();

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/* Get software timer number                                                */
/****************************************************************************/
OS_EXE_TYPE osif_timer_get_timer_number(void **pp_handle, uint8_t *p_timer_number)
{
    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    if (*pp_handle == NULL)
    {
        return OS_EXE_ERROR_PTR_HANDLE_IS_NULL;
    }

    if (p_timer_number == NULL)
    {
        return OS_EXE_ERROR_TIMER_NUMBER_IS_NULL;
    }

    *p_timer_number = (uint8_t)uxTimerGetTimerNumber((TimerHandle_t)(*pp_handle));

    return OS_EXE_SUCCESS;

}

/****************************************************************************/
/* Get software timer handle                                                */
/****************************************************************************/
OS_EXE_TYPE osif_timer_handle_get(uint8_t timer_idx, void **pp_handle)
{
    if (pp_handle == NULL)
    {
        return OS_EXE_FAIL;
    }

    vTimerGetTimerHandle(timer_idx, pp_handle);

    if (*pp_handle)
    {
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

/****************************************************************************/
/* Init software timer pool                                                */
/****************************************************************************/
void osif_timer_init(void)
{
    os_timer_init_freertos();
}

/****************************************************************************/
/* Create isr trace                                                         */
/****************************************************************************/
bool osif_trace_isr_create(void **pp_handle, const char *p_name, uint32_t priority)
{
    return true;
}

/****************************************************************************/
/* Create isr trace begin                                                   */
/****************************************************************************/
bool osif_trace_isr_begin(void *pp_handle)
{
    return true;
}

/****************************************************************************/
/* Create isr trace end                                                     */
/****************************************************************************/
bool osif_trace_isr_end(void)
{
    return true;
}

/****************************************************************************/
/*dlps restore os kernel scheduler processing                               */
/****************************************************************************/
void osif_pm_bottom_half(void (*pend_func)(void))
{
    if (!os_timer_pend_function_call((void (*)(void *, uint32_t))pend_func, NULL, 0))
    {
        OSIF_PRINT_ERROR0("osif_pm_bottom_half function pend fail");
    }
}

void osif_pm_return_to_idle_task(void)
{
    extern uint32_t TopOfIdleTaskStack;
    extern uint32_t IdleTaskStackLim;

    __set_MSP(*(volatile uint32_t *)SCB->VTOR);

    __ISB();

    __set_PSPLIM(IdleTaskStackLim);

    __ISB();

    __set_PSP(TopOfIdleTaskStack);

    __ISB();

    __set_CONTROL(__get_CONTROL() | BIT1);

    __ISB();

    extern void prvIdleTask(void *);
    prvIdleTask((void *)TopOfIdleTaskStack);
}

void osif_pm_init(void)
{
    power_manager_slave_register_function_to_return(osif_pm_return_to_idle_task);

    platform_pm_register_schedule_bottom_half_callback_func(osif_pm_bottom_half);
}

/****************************************************************************/
/*Restore os kernel scheduler                                               */
/****************************************************************************/
OS_EXE_TYPE osif_sched_restore(void)
{
    vPortStartSchedulerInRestore();

#if ( configENABLE_TRUSTZONE == 1 )
    SecureContext_Init();
    extern volatile SecureContextHandle_t xSecureContext;
    SecureContext_ResetContext(xSecureContext);
    SecureContext_LoadContext(xSecureContext);
#endif

#if ( configENABLE_FPU == 1 )
    extern void prvSetupFPU(void);
    prvSetupFPU();
#endif

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/*os kernel systick handler                                                 */
/****************************************************************************/
void osif_systick_handler(void)
{
    extern void SysTick_Handler(void);
    SysTick_Handler();
}

uint32_t osif_sys_tick_rate_get(void)
{
    return (uint32_t)configTICK_RATE_HZ;
}

uint32_t osif_sys_tick_clk_get(void)
{
    return (uint32_t)configSYSTICK_CLOCK_HZ;
}

uint64_t osif_sys_tick_increase(uint32_t tick_increment)
{
    extern TickType_t xTickCount;
    extern TickType_t xPendedTicks;
    uint32_t xTickCount_old = xTickCount;
    uint32_t xTickCount_new = xTickCount + tick_increment;

    if (xTickCount_new > xTickCount_old)
    {
        xTickCount = xTickCount_new - 1;
        xPendedTicks += 1;
    }
    else // tick_increment = 0 or xTickCount wraps around
    {
        xPendedTicks += tick_increment;
    }
    return xTickCount_old;
}

/****************************************************************************/
/* Get software timer pool next expire value                                 */
/****************************************************************************/
static uint32_t find_minimum_timeout_value(ListItem_t *xListItem, UBaseType_t uxListNum,
                                           PlatformExcludedHandleType type)
{
    for (uint32_t i = 0; i < uxListNum; i ++)
    {
        void *itemOwner = listGET_LIST_ITEM_OWNER(xListItem);
        bool handle_checked = true;

        // search if itemOwner is in the exclude_handle
        T_OS_QUEUE_ELEM *p_cur_queue_item = lpm_excluded_handle[type].p_first;
        while (p_cur_queue_item != NULL)
        {
            void *cur_excluded_handle = *(((PlatformPMExcludedHandleQueueElem *)p_cur_queue_item)->handle);
            if (cur_excluded_handle != NULL)
            {
                UBaseType_t is_auto_reload;
                os_timer_get_auto_reload(&cur_excluded_handle, (void *)&is_auto_reload);
                if ((type == PLATFORM_PM_EXCLUDED_TIMER) && is_auto_reload)
                {
                    DBG_DIRECT("[PM]!!handle=0x%x", cur_excluded_handle);
                    configASSERT(0);
                }

                if (itemOwner == cur_excluded_handle)
                {
                    handle_checked = false;
                    break;
                }
            }
            p_cur_queue_item = p_cur_queue_item->p_next;
        }

        // calculate timediff
        if (handle_checked)
        {
            uint32_t timediff;
            TickType_t expire_time = listGET_LIST_ITEM_VALUE(xListItem);

            extern TickType_t xTickCount;
            timediff = clk_cnt_diff(xTickCount, expire_time, BIT64(32));

//            const char *item_name = NULL;
//            if (type == PLATFORM_PM_EXCLUDED_TIMER)
//            {
//                item_name = pcTimerGetTimerName(itemOwner);
//                DBG_DIRECT("[PM] !**timer check: name:%s, now:0x%x, next:0x%x",
//                           item_name,
//                           xTickCount,
//                           expire_time);
//            }
//            else
//            {
//                item_name = pcTaskGetTaskName(itemOwner);
//                DBG_DIRECT("[PM] !**task check: name:%s, now:0x%x, next:0x%x",
//                           item_name,
//                           xTickCount,
//                           expire_time);
//            }

            return timediff;
        }

        xListItem = xListItem->pxNext;
    }

    return 0xFFFFFFFF;
}

uint32_t osif_pm_next_timeout_value_get(void)
{
    ListItem_t *list_item;
    UBaseType_t list_num;
    uint32_t timer_timeout_tick, task_timeout_tick;

    vTimerGetNextTimeoutItem(&list_item, &list_num, tmrLIST_CURRENT_TIMER_LIST);
    timer_timeout_tick = find_minimum_timeout_value(list_item, list_num, PLATFORM_PM_EXCLUDED_TIMER);

    vTimerGetNextTimeoutItem(&list_item, &list_num, tmrLIST_CURRENT_TASK_LIST);
    task_timeout_tick = find_minimum_timeout_value(list_item, list_num, PLATFORM_PM_EXCLUDED_TASK);

    if (timer_timeout_tick == 0xFFFFFFFF)
    {
        vTimerGetNextTimeoutItem(&list_item, &list_num, tmrLIST_OVERFLOW_TIMER_LIST);
        timer_timeout_tick = find_minimum_timeout_value(list_item, list_num, PLATFORM_PM_EXCLUDED_TIMER);
    }

    if (task_timeout_tick == 0xFFFFFFFF)
    {
        vTimerGetNextTimeoutItem(&list_item, &list_num, tmrLIST_OVERFLOW_TASK_LIST);
        task_timeout_tick = find_minimum_timeout_value(list_item, list_num, PLATFORM_PM_EXCLUDED_TASK);
    }

    return (timer_timeout_tick < task_timeout_tick ? timer_timeout_tick : task_timeout_tick);
}

/****************************************************************************/
/*dlps restore os kernel scheduler processing                               */
/****************************************************************************/
OS_EXE_TYPE osif_register_pm_excluded_handle(void **pp_handle, PlatformExcludedHandleType type)
{
    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    PlatformPMExcludedHandleQueueElem *p_handle_queue_item = (PlatformPMExcludedHandleQueueElem *)
                                                             os_mem_alloc(RAM_TYPE_DATA_ON, sizeof(PlatformPMExcludedHandleQueueElem));

    if (p_handle_queue_item == NULL)
    {
        return OS_EXE_FAIL;
    }

    p_handle_queue_item->handle = pp_handle;
    os_queue_in(&lpm_excluded_handle[type], p_handle_queue_item);

    return OS_EXE_SUCCESS;
}

/****************************************************************************/
/*dlps restore os kernel scheduler processing                               */
/****************************************************************************/
OS_EXE_TYPE osif_unregister_pm_excluded_handle(void **pp_handle, PlatformExcludedHandleType type)
{
    if (pp_handle == NULL)
    {
        return OS_EXE_ERROR_HANDLE_IS_NULL;
    }

    T_OS_QUEUE_ELEM *p_cur_queue_elem = lpm_excluded_handle[type].p_first;
    bool check_found = false;

    while (p_cur_queue_elem != NULL)
    {
        void **p_cur_handle = ((PlatformPMExcludedHandleQueueElem *)p_cur_queue_elem)->handle;
        if (pp_handle == p_cur_handle)
        {
            check_found = true;
            os_queue_delete(&lpm_excluded_handle[type], p_cur_queue_elem);
            break;
        }
        p_cur_queue_elem = p_cur_queue_elem->p_next;
    }

    if (check_found)
    {
        os_mem_free(p_cur_queue_elem);
        return OS_EXE_SUCCESS;
    }
    else
    {
        return OS_EXE_FAIL;
    }
}

void os_freertos_patch_init(void) APP_FLASH_TEXT_SECTION;
void os_freertos_patch_init(void)
{
    os_mem_func_init();
    os_msg_func_init();
#if 0
    os_pool_func_init();
    os_queue_func_init();
#else
    os_pool_func_init();
    os_queue_func_init();
    os_trace_func_init();
#endif
    os_sched_func_init();
    os_sync_func_init();
    os_task_func_init();
    os_timer_func_init();
    os_pm_func_init();
}

//USER_CALL_BACK os_patch_init = os_freertos_patch_init;
