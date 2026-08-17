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
#include "debug.h"

#include <opt.h>
#include <arch.h>
#if !NO_SYS
#include "sys_arch.h"
#endif
#include "tcpip.h"
#include "init.h"
#include "netif.h"
#include "sio.h"
//#include "ethernetif.h"
#include <stats.h>
#include <debug.h>
#include <sys.h>
#include "dhcp.h"
#include <string.h>
#include "trace.h"

int errno;

uint32_t lwip_sys_now;

struct sys_timeouts
{
    struct sys_timeo *next;
};

struct timeoutlist
{
    struct sys_timeouts timeouts;
    xTaskHandle pid;
};

#define SYS_THREAD_MAX 4

static struct timeoutlist s_timeoutlist[SYS_THREAD_MAX];

static uint16_t s_nextthread = 0;

uint32_t sys_jiffies(void)
{
    lwip_sys_now = xTaskGetTickCount();
    return lwip_sys_now;
}

uint32_t sys_now(void)
{
    lwip_sys_now = xTaskGetTickCount();
    return lwip_sys_now;
}

void sys_init(void)
{
    int i;

    // Initialize the the per-thread sys_timeouts structures
    // make sure there are no valid pids in the list
    for (i = 0; i < SYS_THREAD_MAX; i++)
    {
        s_timeoutlist[i].pid = 0;
        s_timeoutlist[i].timeouts.next = NULL;
    }
    // keep track of how many threads have been created
    s_nextthread = 0;
}

struct sys_timeouts *sys_arch_timeouts(void)
{
    int i;
    xTaskHandle pid;
    struct timeoutlist *tl;

    pid = xTaskGetCurrentTaskHandle();
    for (i = 0; i < s_nextthread; i++)
    {
        tl = &(s_timeoutlist[i]);
        if (tl->pid == pid)
        {
            return &(tl->timeouts);
        }
    }

    return NULL;
}

sys_prot_t sys_arch_protect(void)
{
    vPortEnterCritical();
    return 1;
}

void sys_arch_unprotect(sys_prot_t pval)
{
    (void) pval;
    vPortExitCritical();
}

#if !NO_SYS
err_t sys_sem_new(sys_sem_t *sem, uint8_t count)
{
    if (count <= 1)
    {
        *sem = xSemaphoreCreateBinary();
        if (count == 1)
        {
            sys_sem_signal(sem);
        }
    }
    else
    {
        *sem = xSemaphoreCreateCounting(count, count);
    }

#if SYS_STATS
    ++lwip_stats.sys.sem.used;
    if (lwip_stats.sys.sem.max < lwip_stats.sys.sem.used)
    {
        lwip_stats.sys.sem.max = lwip_stats.sys.sem.used;
    }
#endif

    if (*sem != SYS_SEM_NULL)
    {
        return ERR_OK;
    }
    else
    {
#if SYS_STATS
        ++lwip_stats.sys.sem.err;
#endif

        APP_PRINT_INFO0("[sys_sem_new] fail");
        return ERR_MEM;
    }
}

void sys_sem_free(sys_sem_t *sem)
{
#if SYS_STATS
    --lwip_stats.sys.sem.used;
#endif

    vSemaphoreDelete(*sem);
    *sem = SYS_SEM_NULL;
}

int sys_sem_valid(sys_sem_t *sem)
{
    return (*sem != SYS_SEM_NULL);
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
    *sem = SYS_SEM_NULL;
}

uint32_t sys_arch_sem_wait(sys_sem_t *sem, uint32_t timeout)
{
    uint32_t wait_tick = 0;
    uint32_t start_tick = 0 ;

    if (*sem == SYS_SEM_NULL)
    {
        return SYS_ARCH_TIMEOUT;
    }

    start_tick = xTaskGetTickCount();

    if (timeout != 0)
    {
        wait_tick = timeout / portTICK_PERIOD_MS;
        if (wait_tick == 0)
        {
            wait_tick = 1;
        }
    }
    else
    {
        wait_tick = portMAX_DELAY;
    }

    if (xSemaphoreTake(*sem, wait_tick) == pdTRUE)
    {
        return ((xTaskGetTickCount() - start_tick) * portTICK_RATE_MS);
    }
    else
    {
        return SYS_ARCH_TIMEOUT;
    }
}

void sys_sem_signal(sys_sem_t *sem)
{
    if (xSemaphoreGive(*sem) != pdTRUE)
    {
        APP_PRINT_INFO0("[sys_sem_signal] fail!\n");
    }
}

err_t sys_mutex_new(sys_mutex_t *mutex)
{
    *mutex = xSemaphoreCreateMutex();

    if (*mutex != SYS_MRTEX_NULL)
    {
        return ERR_OK;
    }
    else
    {
        APP_PRINT_INFO0("[sys_mutex_new] fail");
        return ERR_MEM;
    }
}

void sys_mutex_free(sys_mutex_t *mutex)
{
    vSemaphoreDelete(*mutex);
}

void sys_mutex_set_invalid(sys_mutex_t *mutex)
{
    *mutex = SYS_MRTEX_NULL;
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
    if (*mutex == SYS_SEM_NULL)
    {
        //return SYS_ARCH_TIMEOUT;
    }

    xSemaphoreTake(*mutex, portMAX_DELAY);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
    if (*mutex == SYS_SEM_NULL)
    {
        //return SYS_ARCH_TIMEOUT;
    }

    xSemaphoreGive(*mutex);
}

sys_thread_t sys_thread_new(const char *name, lwip_thread_fn function, void *arg, int stacksize,
                            int prio)
{
    sys_thread_t handle = NULL;
    BaseType_t xReturn = pdPASS;

    xReturn = xTaskCreate((TaskFunction_t)function,
                          (const char *)name,
                          (uint16_t)stacksize,
                          (void *)arg,
                          (UBaseType_t)prio,
                          (TaskHandle_t *)&handle);
    if (xReturn != pdPASS)
    {
        APP_PRINT_INFO1("[sys_thread_new] error code = %d", xReturn);
        return NULL;
    }

    return handle;
}

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    *mbox = xQueueCreate((UBaseType_t) size, (UBaseType_t) sizeof(void *));

#if SYS_STATS
    ++lwip_stats.sys.mbox.used;
    if (lwip_stats.sys.mbox.max < lwip_stats.sys.mbox.used)
    {
        lwip_stats.sys.mbox.max = lwip_stats.sys.mbox.used;
    }
#endif
    if (NULL == *mbox)
    {
        return ERR_MEM;
    }

    return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
    if (uxQueueMessagesWaiting(*mbox))
    {
        /* Line for breakpoint.  Should never break here! */
        portNOP();
#if SYS_STATS
        lwip_stats.sys.mbox.err++;
#endif

        // TODO notify the user of failure.
    }

    vQueueDelete(*mbox);

#if SYS_STATS
    --lwip_stats.sys.mbox.used;
#endif
}

int sys_mbox_valid(sys_mbox_t *mbox)
{
    if (*mbox == SYS_MBOX_NULL)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    *mbox = SYS_MBOX_NULL;
}

void sys_mbox_post(sys_mbox_t *q, void *msg)
{
    while (xQueueSend(*q, &msg, portMAX_DELAY) != pdTRUE);
}

err_t sys_mbox_trypost(sys_mbox_t *q, void *msg)
{
    if (xQueueSend(*q, &msg, 0) == pdPASS)
    {
        return ERR_OK;
    }
    else
    {
        return ERR_MEM;
    }
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *q, void *msg)
{
    return sys_mbox_trypost(q, msg);
}

uint32_t sys_arch_mbox_fetch(sys_mbox_t *q, void **msg, uint32_t timeout)
{
    void *dummyptr;
    uint32_t wait_tick = 0;
    uint32_t start_tick = 0 ;

    if (msg == NULL)
    {
        msg = &dummyptr;
    }

    start_tick = sys_now();

    if (timeout != 0)
    {
        wait_tick = timeout / portTICK_PERIOD_MS;
        if (wait_tick == 0)
        {
            wait_tick = 1;
        }
    }
    else
    {
        wait_tick = portMAX_DELAY;
    }

    if (xQueueReceive(*q, &(*msg), wait_tick) == pdTRUE)
    {
        return ((sys_now() - start_tick) * portTICK_PERIOD_MS);
    }
    else
    {
        *msg = NULL;
        return SYS_ARCH_TIMEOUT;
    }
}

uint32_t sys_arch_mbox_tryfetch(sys_mbox_t *q, void **msg)
{
    void *dummyptr;

    if (msg == NULL)
    {
        msg = &dummyptr;
    }

    if (xQueueReceive(*q, &(*msg), 0) == pdTRUE)
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
    int n = vsnprintf((char *)tx_buffer, 256, format, args);
    va_end(args);
    APP_PRINT_INFO1("%s", TRACE_STRING(tx_buffer));
    return n;
}

#endif /* !NO_SYS */
