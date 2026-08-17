/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _ALIPAY_MEM_H_
#define _ALIPAY_MEM_H_

#include "os_sched.h"
#include "app_msg.h"
#include "os_sync.h"
#include "stdint.h"
#include "os_mem.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void *ts_realloc(void *mem, size_t size);

void *ts_malloc(uint32_t size);

void *ts_calloc(uint32_t nblock, uint32_t size);

void ts_free(void *pt);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif
