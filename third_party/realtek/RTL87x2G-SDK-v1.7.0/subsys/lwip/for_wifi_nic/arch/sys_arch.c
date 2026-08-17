/*
 * Copyright (c) 2017 Simon Goldschmidt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Simon Goldschmidt
 *
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "lwip/opt.h"
#include "lwip/arch.h"
#if !NO_SYS
#include "sys_arch.h"
#endif
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/stats.h"
#include "lwip/debug.h"
#include "lwip/sys.h"
#include <string.h>
#include "trace.h"

#if !LWIP_SYS_USE_OS_IF
#if LWIP_SYS_ARCH_PROTECT_USES_MUTEX || !LWIP_COMPAT_MUTEX
#if !configUSE_MUTEXES
# error "lwIP FreeRTOS port requires configUSE_MUTEXES"
#endif
#endif
#endif
/*============================================================================*
 *                              Local Macros
 *============================================================================*/

/*============================================================================*
 *                              Local Types
 *============================================================================*/

/*============================================================================*
 *                              Local Variables
 *============================================================================*/
int errno;
#if LWIP_SYS_ARCH_PROTECT_USES_MUTEX
static sys_mutex_t sys_arch_protect_mutex;
#endif
/*============================================================================*
 *                              Functions Declaration
 *============================================================================*/

/*============================================================================*
 *                              Local Functions
 *============================================================================*/

/*============================================================================*
*                              Global Functions
*============================================================================*/
uint32_t sys_jiffies(void)
{
#if LWIP_SYS_USE_OS_IF
    return os_sys_tick_get();
#else
    return xTaskGetTickCount();
#endif
}

uint32_t sys_now(void)
{
#if LWIP_SYS_USE_OS_IF
    return os_sys_time_get();
#else
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
#endif

}

void sys_init(void)
{
#if LWIP_SYS_ARCH_PROTECT_USES_MUTEX
    /* initialize sys_arch_protect global mutex */
#if LWIP_SYS_USE_OS_IF
    os_mutex_create(&sys_arch_protect_mutex);
#else
    sys_arch_protect_mutex = xSemaphoreCreateRecursiveMutex();
#endif
    LWIP_ASSERT("failed to create sys_arch_protect mutex",
                sys_arch_protect_mutex != NULL);
#endif /* LWIP_SYS_ARCH_PROTECT_USES_MUTEX */
}

sys_prot_t sys_arch_protect(void)
{
#if LWIP_SYS_ARCH_PROTECT_USES_MUTEX
    osBaseType_t ret = osFALSE;
    LWIP_ASSERT("sys_arch_protect_mutex != NULL", sys_arch_protect_mutex != NULL);
#if LWIP_SYS_USE_OS_IF
    ret = os_mutex_take(sys_arch_protect_mutex, 0xffffffffUL);
#else
    ret = xSemaphoreTakeRecursive(sys_arch_protect_mutex, portMAX_DELAY);
    LWIP_ASSERT("sys_arch_protect failed to take the mutex", ret == osTRUE);
#endif
    return (sys_prot_t)1;
#else
#if LWIP_SYS_USE_OS_IF
    return os_lock();
#else
    vPortEnterCritical();
    return (sys_prot_t)1;
#endif
#endif
}

void sys_arch_unprotect(sys_prot_t pval)
{
#if LWIP_SYS_ARCH_PROTECT_USES_MUTEX
    osBaseType_t ret = osFALSE;;
    LWIP_ASSERT("sys_arch_protect_mutex != NULL", sys_arch_protect_mutex != NULL);
#if LWIP_SYS_USE_OS_IF
    ret = os_mutex_give(sys_arch_protect_mutex);
#else
    ret = xSemaphoreGiveRecursive(sys_arch_protect_mutex);
    LWIP_ASSERT("sys_arch_protect failed to give the mutex", ret == osTRUE);
#endif
#else
#if LWIP_SYS_USE_OS_IF
    os_unlock(pval);
#else
    (void) pval;
    vPortExitCritical();
#endif
#endif
}

#if !NO_SYS
err_t sys_sem_new(sys_sem_t *sem, uint8_t count)
{
    if (count <= 1)
    {
#if LWIP_SYS_USE_OS_IF
        os_sem_create(sem, "lwip_sem", 0, 1);
#else
        *sem = xSemaphoreCreateBinary();
#endif
    }
    else
    {
#if LWIP_SYS_USE_OS_IF
        os_sem_create(sem, "lwip_sem", count, count);
#else
        *sem = xSemaphoreCreateCounting(count, count);
#endif
    }

    if (*sem == NULL)
    {
        SYS_STATS_INC(sem.err);
        LWIP_PLATFORM_DIAG(("sys_sem_new() fail"));
        return ERR_MEM;
    }
    SYS_STATS_INC_USED(sem);
#if !LWIP_SYS_USE_OS_IF
    if (count == 1)
    {
        osBaseType_t ret = xSemaphoreGive(*sem);
        LWIP_ASSERT("sys_sem_new: initial give failed", ret == osTRUE);
    }
#endif
    return ERR_OK;
}

void sys_sem_free(sys_sem_t *sem)
{
    LWIP_ASSERT("*sem != NULL", *sem != NULL);
#if LWIP_SYS_USE_OS_IF
    os_sem_delete(*sem);
#else
    vSemaphoreDelete(*sem);
#endif
    SYS_STATS_DEC(sem.used);
    *sem = NULL;
}

int sys_sem_valid(sys_sem_t *sem)
{
    return (*sem != NULL);
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
    *sem = NULL;
}

uint32_t sys_arch_sem_wait(sys_sem_t *sem, uint32_t timeout)
{
    LWIP_ASSERT("*sem != NULL", *sem != NULL);
    uint32_t wait_time = timeout;
    osBaseType_t ret = osFALSE;
    uint32_t start_timestamp = sys_jiffies();
    if (timeout != 0)
    {
#if !LWIP_SYS_USE_OS_IF
        wait_time = timeout / portTICK_PERIOD_MS;
        if (wait_time == 0)
        {
            wait_time = 1;
        }
#endif
    }
    else
    {
#if LWIP_SYS_USE_OS_IF
        wait_time = 0xffffffffUL;
#else
        wait_time = portMAX_DELAY;
#endif
    }
#if LWIP_SYS_USE_OS_IF
    ret = os_sem_take(*sem, wait_time);
#else
    ret = xSemaphoreTake(*sem, wait_time);
#endif
    if (ret != osTRUE)
    {
        return SYS_ARCH_TIMEOUT;
    }
    else
    {
        return ((sys_jiffies() - start_timestamp));
    }
}

void sys_sem_signal(sys_sem_t *sem)
{
    LWIP_ASSERT("*sem != NULL", *sem != NULL);
    osBaseType_t ret = osFALSE;
#if LWIP_SYS_USE_OS_IF
    ret = os_sem_give(*sem);
#else
    ret = xSemaphoreGive(*sem);
#endif
    if (ret != osTRUE)
    {
        LWIP_PLATFORM_DIAG(("sys_sem_signal() fail!\n"));
    }
}

err_t sys_mutex_new(sys_mutex_t *mutex)
{
#if LWIP_SYS_USE_OS_IF
    os_mutex_create(mutex);
#else
    *mutex = xSemaphoreCreateRecursiveMutex();
#endif
    if (*mutex == NULL)
    {
        SYS_STATS_INC(mutex.err);
        LWIP_PLATFORM_DIAG(("sys_mutex_new() fail"));
        return ERR_MEM;
    }
    SYS_STATS_INC_USED(mutex);
    return ERR_OK;
}

void sys_mutex_free(sys_mutex_t *mutex)
{
    LWIP_ASSERT("*mutex != NULL", *mutex != NULL);
#if LWIP_SYS_USE_OS_IF
    os_mutex_delete(*mutex);
#else
    vSemaphoreDelete(*mutex);
#endif
    SYS_STATS_DEC(mutex.used);
    *mutex = NULL;
}

void sys_mutex_set_invalid(sys_mutex_t *mutex)
{
    *mutex = NULL;
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
    osBaseType_t ret = osFALSE;
    LWIP_ASSERT("*mutex != NULL", *mutex != NULL);
#if LWIP_SYS_USE_OS_IF
    ret = os_mutex_take(*mutex, 0xffffffffUL);
#else
    ret = xSemaphoreTakeRecursive(*mutex, portMAX_DELAY);
#endif
    LWIP_ASSERT("failed to take the mutex", ret == osTRUE);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
    osBaseType_t ret = osFALSE;
    LWIP_ASSERT("*mutex != NULL", *mutex != NULL);
#if LWIP_SYS_USE_OS_IF
    ret = os_mutex_give(*mutex);
#else
    ret = xSemaphoreGiveRecursive(*mutex);
#endif
    LWIP_ASSERT("failed to take the mutex", ret == osTRUE);
}

sys_thread_t sys_thread_new(const char *name, lwip_thread_fn function, void *arg, int stacksize,
                            int prio)
{
    sys_thread_t handle = NULL;
    osBaseType_t ret = osFALSE;
#if LWIP_SYS_USE_OS_IF
    ret = os_task_create(&handle, name, function, arg, stacksize, prio);
#else
    ret = xTaskCreate((TaskFunction_t)function,
                      (const char *)name,
                      (uint16_t)stacksize / sizeof(portSTACK_TYPE),
                      (void *)arg,
                      (UBaseType_t)prio,
                      (TaskHandle_t *)&handle);
#endif
    if (ret != osTRUE)
    {
        LWIP_PLATFORM_DIAG(("sys_thread_new() error code = %d", ret));
        return NULL;
    }

    return handle;
}

void sys_thread_free(sys_thread_t handle)
{
    osBaseType_t ret = osFALSE;
#if LWIP_SYS_USE_OS_IF
    ret = os_task_delete(handle);
#else
    vTaskDelete((TaskHandle_t)handle);
    ret = osTRUE;
#endif
    if (ret != osTRUE)
    {
        LWIP_PLATFORM_DIAG(("sys_thread_new() error code = %d", ret));
    }
}

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
#if LWIP_SYS_USE_OS_IF
    os_msg_queue_create(mbox, "lwip_mbox", size, sizeof(void *));
#else
    *mbox = xQueueCreate((UBaseType_t) size, (UBaseType_t) sizeof(void *));
#endif
    if (NULL == *mbox)
    {
        SYS_STATS_INC(mbox.err);
        return ERR_MEM;
    }
    SYS_STATS_INC_USED(mbox);
    return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
    LWIP_ASSERT("*mbox != NULL", *mbox != NULL);
    uint32_t msgs_waiting = 0;
#if LWIP_SYS_USE_OS_IF
    os_msg_queue_peek(*mbox, &msgs_waiting);
#else
    msgs_waiting = (uint32_t)uxQueueMessagesWaiting(*mbox);
#endif
    if (msgs_waiting != 0)
    {
        SYS_STATS_INC(mbox.err);
    }
#if LWIP_SYS_USE_OS_IF
    os_msg_queue_delete(*mbox);
#else
    vQueueDelete(*mbox);
#endif
    SYS_STATS_DEC(mbox.used);
}

int sys_mbox_valid(sys_mbox_t *mbox)
{
    return (int)(*mbox != NULL);
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    *mbox = NULL;
}

void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    LWIP_ASSERT("*mbox != NULL", *mbox != NULL);
    osBaseType_t ret = osFALSE;
#if LWIP_SYS_USE_OS_IF
    ret = os_msg_send(*mbox, &msg, 0xFFFFFFFFUL);
#else
    ret = xQueueSend(*mbox, &msg, portMAX_DELAY);
#endif
    LWIP_ASSERT("mbox post failed", ret == osTRUE);
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    LWIP_ASSERT("*mbox != NULL", *mbox != NULL);
    osBaseType_t ret = osFALSE;
#if LWIP_SYS_USE_OS_IF
    ret = os_msg_send(*mbox, &msg, 0);
#else
    ret = xQueueSend(*mbox, &msg, 0);
#endif
    if (ret == osTRUE)
    {
        return ERR_OK;
    }
    else
    {
        LWIP_PLATFORM_DIAG(("sys_mbox_trypost() fail! mbox address:0x%x", (uint32_t)mbox));
        SYS_STATS_INC(mbox.err);
        return ERR_MEM;
    }
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
    LWIP_ASSERT("*mbox != NULL", *mbox != NULL);
    osBaseType_t ret = osFALSE;
#if LWIP_SYS_USE_OS_IF
    ret = os_msg_send(*mbox, &msg, 0);
#else
    BaseType_t xHigherPriorityTaskWoken = osFALSE;
    ret = xQueueSendFromISR(*mbox, &msg, &xHigherPriorityTaskWoken);
#endif
    if (ret == osTRUE)
    {
        return ERR_OK;
    }
    else
    {
        LWIP_PLATFORM_DIAG(("sys_mbox_trypost_fromisr() fail! mbox address:0x%x", (uint32_t)mbox));
        SYS_STATS_INC(mbox.err);
        return ERR_MEM;
    }
}

uint32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, uint32_t timeout)
{
    LWIP_ASSERT("*mbox != NULL", *mbox != NULL);
    void *msg_dummy;
    uint32_t wait_time = timeout;
    osBaseType_t ret = osFALSE;
    if (!msg)
    {
        msg = &msg_dummy;
    }
    uint32_t start_timestamp = sys_jiffies();
    if (timeout != 0)
    {
#if !LWIP_SYS_USE_OS_IF
        wait_time = timeout / portTICK_PERIOD_MS;
        if (wait_time == 0)
        {
            wait_time = 1;
        }
#endif
    }
    else
    {
#if LWIP_SYS_USE_OS_IF
        wait_time = 0xffffffffUL;
#else
        wait_time = portMAX_DELAY;
#endif
    }
#if LWIP_SYS_USE_OS_IF
    ret = os_msg_recv(*mbox, &(*msg), wait_time);
#else
    ret = xQueueReceive(*mbox, &(*msg), wait_time);
#endif
    if (ret != osTRUE)
    {
        return SYS_ARCH_TIMEOUT;
    }
    else
    {
        return ((sys_jiffies() - start_timestamp));
    }
}

uint32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    LWIP_ASSERT("*mbox != NULL", *mbox != NULL);
    osBaseType_t ret = osFALSE;
    void *msg_dummy;
    if (!msg)
    {
        msg = &msg_dummy;
    }
#if LWIP_SYS_USE_OS_IF
    ret = os_msg_recv(*mbox, &(*msg), 0);
#else
    ret = xQueueReceive(*mbox, &(*msg), 0);
#endif
    if (ret == osTRUE)
    {
        return ERR_OK;
    }
    else
    {
        return SYS_MBOX_EMPTY;
    }
}

#if LWIP_NETCONN_SEM_PER_THREAD
#error LWIP_NETCONN_SEM_PER_THREAD==1 not supported
#endif /* LWIP_NETCONN_SEM_PER_THREAD */

#include <stdarg.h>
#include <stdio.h>
int lwip_printf(const char *format, ...)
{
    char tx_buffer[256];
    va_list args;
    va_start(args, format);
    int n = vsnprintf((char *)tx_buffer, sizeof(tx_buffer), format, args);
    va_end(args);
    DBG_BUFFER_INTERNAL(LOG_TYPE, SUBTYPE_FORMAT, MODULE_APP, LEVEL_ERROR, "[LwIP] %s", 1,
                        TRACE_STRING(tx_buffer));
    return n;
}
#endif /* !NO_SYS */
