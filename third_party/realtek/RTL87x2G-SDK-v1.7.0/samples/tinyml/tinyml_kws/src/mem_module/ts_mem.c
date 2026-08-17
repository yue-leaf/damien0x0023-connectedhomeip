/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "string.h"
#include "board.h"
#include "os_timer.h"
#include "os_sched.h"
#include "app_msg.h"
#include "os_sync.h"
#include "trace.h"
#include "os_mem.h"
#include "ts_mem.h"
#include "psRam_heap.h"



//typedef struct NEW_BLOCK_LINK
//{
//
//    struct NEW_BLOCK_LINK *pxNextFreeBlock;
//    size_t xBlockSize : 20; /*<< The size of the free block. */
//    size_t magic : 8;
//    size_t xRamType : 3;
//    size_t xAllocateBit : 1;
//    //    size_t LRValue;
//    //    size_t allocTime;
//} psRam_BlockLink_t;
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK *pxNextFreeBlock;  /*<< The next free block in the list. */
    size_t xBlockSize: 30;                     /*<< The size of the free block. */
    size_t xAllocateBit: 1;
} psRam_BlockLink_t;


#define LOG_Mem_Print(format, ...)  //DBG_DIRECT(format, ##__VA_ARGS__)


void *ts_realloc(void *mem, size_t size)
{
    // TODO:
    LOG_Mem_Print("[Yuyin] realloc mem %x size %d", mem, size);

    void *new_ptr;
    psRam_BlockLink_t *pxLink = NULL;
    size_t old_size, new_size = 0;
    if (mem == NULL)
    {
        return ts_malloc(size);
    }

    if (size == 0)
    {
        ts_free(mem);
        return NULL;
    }

    pxLink = (psRam_BlockLink_t *)((uint8_t *)mem - sizeof(psRam_BlockLink_t));
    old_size = pxLink->xBlockSize;

    LOG_Mem_Print("[Yuyin]realloc old %d", old_size - 8);

//  size_t align_size = (size & portBYTE_ALIGNMENT_MASK)? (FreeRTOS_portBYTE_ALIGNMENT - (size & portBYTE_ALIGNMENT_MASK)): 0;
//    if ((old_size - sizeof(psRam_BlockLink_t)) == (size + align_size))
//    {
//      LOG_Mem_Print("[Yuyin] old mem !");
//        return mem;
//    }

    os_sched_suspend();

    new_ptr = ts_malloc(size);
    if (new_ptr == NULL)
    {
        LOG_Mem_Print("[Yuyin] new_ptr failed!");
        return NULL;
    }


    {
        psRam_BlockLink_t *pxLink_new;
        pxLink_new = (psRam_BlockLink_t *)((uint8_t *)new_ptr - sizeof(psRam_BlockLink_t));
        new_size = pxLink_new->xBlockSize;
        memcpy(new_ptr, mem, (old_size < new_size) ? (old_size - sizeof(psRam_BlockLink_t)) :
               (new_size - sizeof(psRam_BlockLink_t)));
        ts_free(mem);

        LOG_Mem_Print("[Yuyin] addr 0x%d, xBlockSize %d, bit %d, ", pxLink_new, pxLink_new->xBlockSize,
                      pxLink_new->xAllocateBit);

    }


    os_sched_resume();
    return new_ptr;
}

size_t ts_getUsedSize(void *pt)
{
    if (pt == NULL)
    {
        return 0;
    }

    psRam_BlockLink_t *pxLink_new;
    pxLink_new = (psRam_BlockLink_t *)((uint8_t *)pt - sizeof(psRam_BlockLink_t));

    LOG_Mem_Print("[Yuyin] ts_getUsedSize %d, addr 0x%x, bit %d", pxLink_new->xBlockSize - 8,
                  pxLink_new, pxLink_new->xAllocateBit);

    return pxLink_new->xBlockSize - 8;
}

static uint32_t g_Max_size = 0;
void *ts_malloc(uint32_t size)
{
    //
    void *pt = psRamPortMalloc(size);
    //void *pt = os_mem_alloc(RAM_TYPE_DSPSHARE, size);
    if (pt == NULL)
    {
        LOG_Mem_Print("[ALipay] malloc failed!!!!!!!");
        return NULL;
    }

    psRam_BlockLink_t *pxLink = (psRam_BlockLink_t *)((uint8_t *)pt - sizeof(psRam_BlockLink_t));
    LOG_Mem_Print("[Yuyin] ts_malloc wanted size %d, addr 0x%x, bit %d, xBlockSize %d", size, pxLink,
                  pxLink->xAllocateBit, pxLink->xBlockSize);

    if ((uint32_t)pxLink > g_Max_size) { g_Max_size = (uint32_t)pxLink; }

    return pt;
}

uint32_t test_get_max_size(void)
{
    return g_Max_size;
}

void *ts_calloc(uint32_t nblock, uint32_t size)
{
#if 1
    void *pt = ts_malloc(nblock * size);
    if (pt)
    {
        memset(pt, 0, nblock * size);
    }
#else
    void *pt = psRamPortCalloc(nblock, size);
#endif
    LOG_Mem_Print("[Yuyin] ts_calloc nblock * size %d, addr %x", nblock * size, pt);
    return pt;
}
void ts_free(void *pt)
{
    if (pt != NULL)
    {
        LOG_Mem_Print("[Yuyin] ts_free");
        psRam_BlockLink_t *pxLink = (psRam_BlockLink_t *)((uint8_t *)pt - sizeof(psRam_BlockLink_t));
        (void)pxLink;
        LOG_Mem_Print("[Yuyin] ts_free size %d, addr 0x%x, bit %d", pxLink->xBlockSize, pxLink,
                      pxLink->xAllocateBit);
        //os_mem_free(pt);
        psRamFree(pt);

        //pt = NULL;
    }

}


