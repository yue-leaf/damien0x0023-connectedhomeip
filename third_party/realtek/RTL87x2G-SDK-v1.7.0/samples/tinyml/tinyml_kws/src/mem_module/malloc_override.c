/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * malloc_override.c
 *
 * Root-cause fix for GCC build HardFault in tinyml_gsensor.
 *
 * Problem:
 *   ROM's pvPortMalloc (used for FreeRTOS TCBs/stacks/queues) and newlib's
 *   malloc (via _sbrk) both start allocating from the NS_HEAP region base
 *   address (0x126400), each maintaining an independent heap-end pointer.
 *   They silently overlap: ROM places FreeRTOS objects (e.g. xTimerQueue) in
 *   addresses that newlib's malloc also returns to callers.  Writing to a
 *   malloc'd pointer then corrupts the FreeRTOS queue struct -> memcpy with
 *   garbage pcReadFrom/uxItemSize -> HardFault in prvCopyDataFromQueue.
 *
 * Fix:
 *   Redirect ALL newlib heap operations (malloc/free/calloc/realloc and their
 *   reentrant _r variants) to psRamPortMalloc/psRamFree which allocate from
 *   ucHeap[] in .bss (NS_RAM_APP).  NS_HEAP is then used exclusively by ROM's
 *   pvPortMalloc, eliminating the conflict.
 *
 * Why MDK is unaffected:
 *   MDK links heap_4.c which defines pvPortMalloc using ucHeap; there is only
 *   one allocator in MDK so no conflict exists.
 */

#if defined(__GNUC__) && !defined(__ARMCC_VERSION)

#include <stddef.h>
#include <string.h>  /* memcpy */
#include <reent.h>   /* struct _reent */

#include "psRam_heap.h"

/* ---------- non-reentrant API (called by user code and C++ new/delete) --- */

void *malloc(size_t size)
{
    return psRamPortMalloc(size);
}

void free(void *ptr)
{
    psRamFree(ptr);
}

void *calloc(size_t n, size_t size)
{
    return psRamPortCalloc(n, size);
}

void *realloc(void *ptr, size_t size)
{
    void *newptr;
    if (!ptr)  { return psRamPortMalloc(size); }
    if (!size) { psRamFree(ptr); return NULL;  }
    newptr = psRamPortMalloc(size);
    if (newptr)
    {
        /* Copy at most 'size' bytes.  Safe for shrinking realloc; for growing
         * realloc this reads up to 'size' bytes from the old buffer which may
         * be beyond the original allocation (acceptable in this firmware since
         * growing realloc is not used). */
        memcpy(newptr, ptr, size);
        psRamFree(ptr);
    }
    return newptr;
}

/* ---------- reentrant _r variants (called internally by newlib) ---------- */

void *_malloc_r(struct _reent *r, size_t size)
{
    (void)r;
    return psRamPortMalloc(size);
}

void _free_r(struct _reent *r, void *ptr)
{
    (void)r;
    psRamFree(ptr);
}

void *_calloc_r(struct _reent *r, size_t n, size_t size)
{
    (void)r;
    return psRamPortCalloc(n, size);
}

void *_realloc_r(struct _reent *r, void *ptr, size_t size)
{
    (void)r;
    return realloc(ptr, size);
}

#endif /* __GNUC__ && !__ARMCC_VERSION */
