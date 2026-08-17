/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _PSRAM_HEAP_HEAD_
#define _PSRAM_HEAP_HEAD_

//typedef struct A_BLOCK_LINK
//{
//    struct A_BLOCK_LINK * pxNextFreeBlock; /*<< The next free block in the list. */
//    size_t xBlockSize: 30;                     /*<< The size of the free block. */
//  size_t xAllocateBit: 1;
//} psRam_BlockLink_t;

#define psRAM_portBYTE_ALIGNMENT  8
#define psRAM_portBYTE_ALIGNMENT_MASK ( 0x0007 )

#define configTOTAL_psRAM_HEAP_SIZE (100 * 1024)

#define PSRAM_ASSERT(e) \
    do \
    { \
        if(!(e)) \
        { \
            DBG_DIRECT("(" #e ") psram assert failed! Func: %s. Line: %d.", __func__, __LINE__); \
            DBG_DIRECT("(" #e ") psram assert failed! Func: %s. Line: %d.", __func__, __LINE__); \
            for(;;); \
        } \
    } while(0)

void *psRamPortMalloc(size_t xWantedSize);
void psRamFree(void *pv);
size_t psRamGetFreeHeapSize(void);
size_t psRamGetMinimumEverFreeHeapSize(void);
void *psRamPortCalloc(size_t xNum,
                      size_t xSize);

#endif
