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
#ifndef SYS_ARCH_H
#define SYS_ARCH_H

#ifdef  __cplusplus
extern "C"
{
#endif

#define LWIP_SYS_USE_OS_IF      1

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "lwip/opt.h"
#include "lwip/arch.h"
#if LWIP_SYS_USE_OS_IF
#include "os_sched.h"
#include "os_msg.h"
#include "os_task.h"
#include "os_sync.h"
#else
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#endif
/*============================================================================*
 *                         Macros
 *============================================================================*/
#define LWIP_SYS_ARCH_PROTECT_USES_MUTEX    0

#if LWIP_SYS_USE_OS_IF
#define SYS_DEFAULT_THREAD_STACK_DEPTH  1024
#define osTRUE                          true
#define osFALSE                         false
typedef bool                            osBaseType_t;
#else
#define SYS_DEFAULT_THREAD_STACK_DEPTH  configMINIMAL_STACK_SIZE
#define osTRUE                          pdTRUE
#define osFALSE                         pdFALSE
typedef BaseType_t                      osBaseType_t;
#endif
/*============================================================================*
 *                         Types
 *============================================================================*/
#if LWIP_SYS_USE_OS_IF
typedef void *sys_sem_t;
typedef void *sys_mutex_t;
typedef void *sys_mbox_t;
typedef void *sys_thread_t;
#else
typedef SemaphoreHandle_t sys_sem_t;
typedef SemaphoreHandle_t sys_mutex_t;
typedef QueueHandle_t sys_mbox_t;
typedef TaskHandle_t sys_thread_t;
#endif
typedef int sys_prot_t;

/*============================================================================*
*                        Export Global Variables
*============================================================================*/

/*============================================================================*
 *                         Functions
 *============================================================================*/
void sys_thread_free(sys_thread_t handle);

#ifdef  __cplusplus
}
#endif
#endif /* SYS_ARCH_H */
