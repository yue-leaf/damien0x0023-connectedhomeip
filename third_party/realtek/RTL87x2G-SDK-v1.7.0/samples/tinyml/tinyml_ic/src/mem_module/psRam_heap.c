/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


#include <stdlib.h>
#include <string.h>
#include "trace.h"
#include "psRam_heap.h"
#include "app_section.h"

extern void vTaskSuspendAll(void);
extern int xTaskResumeAll(void);

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
 * all the API functions to use the MPU wrappers.  That should only be done when
 * task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE


#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

//#if ( configSUPPORT_DYNAMIC_ALLOCATION == 0 )
//    #error This file must not be used if configSUPPORT_DYNAMIC_ALLOCATION is 0
//#endif

#ifndef configHEAP_CLEAR_MEMORY_ON_FREE
#define configHEAP_CLEAR_MEMORY_ON_FREE    0
#endif

/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE    ( ( size_t ) ( xHeapStructSize << 1 ) )

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE         ( ( size_t ) 8 )

/* Max value that fits in a size_t type. */
#define heapSIZE_MAX              ( ~( ( size_t ) 0 ) )

/* Check if multiplying a and b will result in overflow. */
#define heapMULTIPLY_WILL_OVERFLOW( a, b )    ( ( ( a ) > 0 ) && ( ( b ) > ( heapSIZE_MAX / ( a ) ) ) )

/* Check if adding a and b will result in overflow. */
#define heapADD_WILL_OVERFLOW( a, b )         ( ( a ) > ( heapSIZE_MAX - ( b ) ) )

/* MSB of the xBlockSize member of an psRam_BlockLink_t structure is used to track
 * the allocation status of a block.  When MSB of the xBlockSize member of
 * an psRam_BlockLink_t structure is set then the block belongs to the application.
 * When the bit is free the block is still part of the free heap space. */
#define heapBLOCK_ALLOCATED_BITMASK    ( ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 ) )
#define heapBLOCK_SIZE_IS_VALID( xBlockSize )    ( ( ( xBlockSize ) & heapBLOCK_ALLOCATED_BITMASK ) == 0 )
#define heapBLOCK_IS_ALLOCATED( pxBlock )        ( ( ( pxBlock->xBlockSize ) & heapBLOCK_ALLOCATED_BITMASK ) != 0 )
#define heapALLOCATE_BLOCK( pxBlock )            ( ( pxBlock->xBlockSize ) |= heapBLOCK_ALLOCATED_BITMASK )
#define heapFREE_BLOCK( pxBlock )                ( ( pxBlock->xBlockSize ) &= ~heapBLOCK_ALLOCATED_BITMASK )

/*-----------------------------------------------------------*/

/* Allocate the memory for the heap. */
#if 0//( configAPPLICATION_ALLOCATED_HEAP == 1 )

/* The application writer has already defined the array used for the RTOS
* heap - probably so it can be placed in a special segment or address. */
extern uint8_t ucHeap[ configTOTAL_psRAM_HEAP_SIZE ];
#else

uint8_t ucHeap[ configTOTAL_psRAM_HEAP_SIZE ];
#endif /* configAPPLICATION_ALLOCATED_HEAP */

/* Define the linked list structure.  This is used to link free blocks in order
 * of their memory address. */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK *pxNextFreeBlock;  /*<< The next free block in the list. */
    size_t xBlockSize;                     /*<< The size of the free block. */
} psRam_BlockLink_t;

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void psRamInsertBlockIntoFreeList(psRam_BlockLink_t *pxBlockToInsert) ;

/*
 * Called automatically to setup the required heap structures the first time
 * psRamPortMalloc() is called.
 */
static void psRamHeapInit(void) ;

/*-----------------------------------------------------------*/

/* The size of the structure placed at the beginning of each allocated memory
 * block must by correctly byte aligned. */
static const size_t xHeapStructSize = (sizeof(psRam_BlockLink_t) + ((size_t)(
                                                                        psRAM_portBYTE_ALIGNMENT - 1))) & ~((size_t) psRAM_portBYTE_ALIGNMENT_MASK);

/* Create a couple of list links to mark the start and end of the list. */
static psRam_BlockLink_t xStart;
static psRam_BlockLink_t *pxEnd = NULL;

/* Keeps track of the number of calls to allocate and free memory as well as the
 * number of free bytes remaining, but says nothing about fragmentation. */
static size_t xFreeBytesRemaining = 0U;
static size_t xMinimumEverFreeBytesRemaining = 0U;
static size_t xNumberOfSuccessfulAllocations = 0;
static size_t xNumberOfSuccessfulFrees = 0;

/*-----------------------------------------------------------*/

#define PSRAM_LOG(format, ...)  //DBG_DIRECT(format, ##__VA_ARGS__)


void *psRamPortMalloc(size_t xWantedSize)
{
    psRam_BlockLink_t *pxBlock;
    psRam_BlockLink_t *pxPreviousBlock;
    psRam_BlockLink_t *pxNewBlockLink;
    void *pvReturn = NULL;
    size_t xAdditionalRequiredSize;

    PSRAM_LOG("[Alipay][LOG] psRamPortMalloc size %d, xHeapStructSize %d", xWantedSize,
              xHeapStructSize);

    vTaskSuspendAll();
    {
        /* If this is the first call to malloc then the heap will require
         * initialisation to setup the list of free blocks. */
        if (pxEnd == NULL)
        {
            psRamHeapInit();
        }
        else
        {
            //mtCOVERAGE_TEST_MARKER();
        }

        if (xWantedSize > 0)
        {
            /* The wanted size must be increased so it can contain a psRam_BlockLink_t
             * structure in addition to the requested amount of bytes. Some
             * additional increment may also be needed for alignment. */
            xAdditionalRequiredSize = xHeapStructSize + psRAM_portBYTE_ALIGNMENT -
                                      (xWantedSize & psRAM_portBYTE_ALIGNMENT_MASK);

            if (heapADD_WILL_OVERFLOW(xWantedSize, xAdditionalRequiredSize) == 0)
            {
                xWantedSize += xAdditionalRequiredSize;
            }
            else
            {
                xWantedSize = 0;
            }
        }
        else
        {
            //mtCOVERAGE_TEST_MARKER();
        }

        /* Check the block size we are trying to allocate is not so large that the
         * top bit is set.  The top bit of the block size member of the psRam_BlockLink_t
         * structure is used to determine who owns the block - the application or
         * the kernel, so it must be free. */
        if (heapBLOCK_SIZE_IS_VALID(xWantedSize) != 0)
        {
            //PSRAM_LOG("[Alipay] psRamPortMalloc 01");
            if ((xWantedSize > 0) && (xWantedSize <= xFreeBytesRemaining))
            {
                //PSRAM_LOG("[Alipay] psRamPortMalloc 02");
                /* Traverse the list from the start (lowest address) block until
                 * one of adequate size is found. */
                pxPreviousBlock = &xStart;
                pxBlock = xStart.pxNextFreeBlock;

                while ((pxBlock->xBlockSize < xWantedSize) && (pxBlock->pxNextFreeBlock != NULL))
                {
                    pxPreviousBlock = pxBlock;
                    pxBlock = pxBlock->pxNextFreeBlock;
                }

                /* If the end marker was reached then a block of adequate size
                 * was not found. */
                if (pxBlock != pxEnd)
                {
                    //PSRAM_LOG("[Alipay] psRamPortMalloc 03");
                    /* Return the memory space pointed to - jumping over the
                     * psRam_BlockLink_t structure at its start. */
                    pvReturn = (void *)(((uint8_t *) pxPreviousBlock->pxNextFreeBlock) + xHeapStructSize);

                    PSRAM_LOG("[Alipay] psRamPortMalloc src addr 0x%x, size %d", pxBlock, pxBlock->xBlockSize);

                    /* This block is being returned for use so must be taken out
                     * of the list of free blocks. */
                    pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

                    /* If the block is larger than required it can be split into
                     * two. */
                    if ((pxBlock->xBlockSize - xWantedSize) > heapMINIMUM_BLOCK_SIZE)
                    {
                        //PSRAM_LOG("[Alipay] psRamPortMalloc 04");
                        /* This block is to be split into two.  Create a new
                         * block following the number of bytes requested. The void
                         * cast is used to prevent byte alignment warnings from the
                         * compiler. */
                        pxNewBlockLink = (void *)(((uint8_t *) pxBlock) + xWantedSize);
                        //assert_param( ( ( ( size_t ) pxNewBlockLink ) & psRAM_portBYTE_ALIGNMENT_MASK ) == 0 );

                        /* Calculate the sizes of two blocks split from the
                         * single block. */
                        pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
                        pxBlock->xBlockSize = xWantedSize;

                        /* Insert the new block into the list of free blocks. */
                        psRamInsertBlockIntoFreeList(pxNewBlockLink);
                    }
                    else
                    {
                        PSRAM_LOG("[Alipay] psRamPortMalloc 05 0x%x", pxBlock);
                        //mtCOVERAGE_TEST_MARKER();
                    }

                    xFreeBytesRemaining -= pxBlock->xBlockSize;


                    if (xFreeBytesRemaining < xMinimumEverFreeBytesRemaining)
                    {
                        //PSRAM_LOG("[Alipay] psRamPortMalloc 06");
                        xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
                    }
                    else
                    {
                        //PSRAM_LOG("[Alipay] psRamPortMalloc 07");
                        //mtCOVERAGE_TEST_MARKER();
                    }

                    /* The block is being returned - it is allocated and owned
                     * by the application and has no "next" block. */
                    heapALLOCATE_BLOCK(pxBlock);
                    PSRAM_LOG("[Alipay] psRamPortMalloc get size 0x%x", pxBlock->xBlockSize);
                    pxBlock->pxNextFreeBlock = NULL;
                    xNumberOfSuccessfulAllocations++;

                    ////assert_param (0x400d490 != pxBlock)//test

                }
                else
                {
                    //PSRAM_LOG("[Alipay] psRamPortMalloc 08");
                    //mtCOVERAGE_TEST_MARKER();
                }
            }
            else
            {
                //PSRAM_LOG("[Alipay] psRamPortMalloc 09");
                //mtCOVERAGE_TEST_MARKER();
            }
        }
        else
        {
            //PSRAM_LOG("[Alipay] psRamPortMalloc 10");
            //mtCOVERAGE_TEST_MARKER();
        }

        // traceMALLOC( pvReturn, xWantedSize );
    }
    (void) xTaskResumeAll();

#if ( configUSE_MALLOC_FAILED_HOOK == 1 )
    {
        if (pvReturn == NULL)
        {
            //vApplicationMallocFailedHook();
            PSRAM_LOG("[Alipay] psRam malloc failed");
        }
        else
        {
            //mtCOVERAGE_TEST_MARKER();
        }
    }
#endif /* if ( configUSE_MALLOC_FAILED_HOOK == 1 ) */

    //assert_param( ( ( ( size_t ) pvReturn ) & ( size_t ) psRAM_portBYTE_ALIGNMENT_MASK ) == 0 );
    return pvReturn;
}
/*-----------------------------------------------------------*/

void psRamFree(void *pv)
{
    uint8_t *puc = (uint8_t *) pv;
    psRam_BlockLink_t *pxLink;

    //PSRAM_LOG("[Alipay] psRamFree pv %x", pv);

    if (pv != NULL)
    {
        /* The memory being freed will have an psRam_BlockLink_t structure immediately
         * before it. */
        puc -= xHeapStructSize;

        /* This casting is to keep the compiler from issuing warnings. */
        pxLink = (void *) puc;

        PSRAM_LOG("[Alipay] psRamFree addr 0x%x, pxLink->xBlockSize %x", pxLink, pxLink->xBlockSize);

        PSRAM_ASSERT(heapBLOCK_IS_ALLOCATED(pxLink) != 0);
        PSRAM_ASSERT(pxLink->pxNextFreeBlock == NULL);

        if (heapBLOCK_IS_ALLOCATED(pxLink) != 0)
        {
            if (pxLink->pxNextFreeBlock == NULL)
            {
                /* The block is being returned to the heap - it is no longer
                 * allocated. */
                heapFREE_BLOCK(pxLink);
#if ( configHEAP_CLEAR_MEMORY_ON_FREE == 1 )
                {
                    (void) memset(puc + xHeapStructSize, 0, pxLink->xBlockSize - xHeapStructSize);
                }
#endif

                vTaskSuspendAll();
                {
                    /* Add this block to the list of free blocks. */
                    xFreeBytesRemaining += pxLink->xBlockSize;
                    //traceFREE( pv, pxLink->xBlockSize );
                    psRamInsertBlockIntoFreeList(((psRam_BlockLink_t *) pxLink));
                    xNumberOfSuccessfulFrees++;
                }
                (void) xTaskResumeAll();
            }
            else
            {
                //mtCOVERAGE_TEST_MARKER();
            }
        }
        else
        {
            //mtCOVERAGE_TEST_MARKER();
        }
    }
}
/*-----------------------------------------------------------*/

size_t psRamGetFreeHeapSize(void)
{
    return xFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

size_t psRamGetMinimumEverFreeHeapSize(void)
{
    return xMinimumEverFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

void psRamInitialiseBlocks(void)
{
    /* This just exists to keep the linker quiet. */
}
/*-----------------------------------------------------------*/

void *psRamPortCalloc(size_t xNum,
                      size_t xSize)
{
    void *pv = NULL;

    if (heapMULTIPLY_WILL_OVERFLOW(xNum, xSize) == 0)
    {
        pv = psRamPortMalloc(xNum * xSize);

        if (pv != NULL)
        {
            (void) memset(pv, 0, xNum * xSize);
        }
    }

    return pv;
}
/*-----------------------------------------------------------*/

static void psRamHeapInit(void)   /*  */
{
    psRam_BlockLink_t *pxFirstFreeBlock;
    uint8_t *pucAlignedHeap;
    uint32_t uxAddress;
    size_t xTotalHeapSize = configTOTAL_psRAM_HEAP_SIZE;

    /* Ensure the heap starts on a correctly aligned boundary. */
    uxAddress = (uint32_t) ucHeap;

    if ((uxAddress & psRAM_portBYTE_ALIGNMENT_MASK) != 0)
    {
        uxAddress += (psRAM_portBYTE_ALIGNMENT - 1);
        uxAddress &= ~((uint32_t) psRAM_portBYTE_ALIGNMENT_MASK);
        xTotalHeapSize -= uxAddress - (uint32_t) ucHeap;
    }

    pucAlignedHeap = (uint8_t *) uxAddress;

    /* xStart is used to hold a pointer to the first item in the list of free
     * blocks.  The void cast is used to prevent compiler warnings. */
    xStart.pxNextFreeBlock = (void *) pucAlignedHeap;
    xStart.xBlockSize = (size_t) 0;

    /* pxEnd is used to mark the end of the list of free blocks and is inserted
     * at the end of the heap space. */
    uxAddress = ((uint32_t) pucAlignedHeap) + xTotalHeapSize;
    uxAddress -= xHeapStructSize;
    uxAddress &= ~((uint32_t) psRAM_portBYTE_ALIGNMENT_MASK);
    pxEnd = (psRam_BlockLink_t *) uxAddress;
    pxEnd->xBlockSize = 0;
    pxEnd->pxNextFreeBlock = NULL;

    /* To start with there is a single free block that is sized to take up the
     * entire heap space, minus the space taken by pxEnd. */
    pxFirstFreeBlock = (psRam_BlockLink_t *) pucAlignedHeap;
    pxFirstFreeBlock->xBlockSize = (size_t)(uxAddress - (uint32_t) pxFirstFreeBlock);
    pxFirstFreeBlock->pxNextFreeBlock = pxEnd;

    /* Only one block exists - and it covers the entire usable heap space. */
    xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
    xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
}
/*-----------------------------------------------------------*/

static void psRamInsertBlockIntoFreeList(psRam_BlockLink_t *pxBlockToInsert)    /*  */
{
    psRam_BlockLink_t *pxIterator;
    uint8_t *puc;

    //PSRAM_LOG("[Alipay][LOG] start 0x%x, end 0x%x", xStart.pxNextFreeBlock, pxEnd);

    /* Iterate through the list until a block is found that has a higher address
     * than the block being inserted. */
    for (pxIterator = &xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert;
         pxIterator = pxIterator->pxNextFreeBlock)
    {
        /* Nothing to do here, just iterate to the right position. */
        //PSRAM_LOG("[Alipay] loop newblock 0x%x, insert 0x%x", pxIterator->pxNextFreeBlock, pxBlockToInsert);

        //assert_param(pxIterator->pxNextFreeBlock);
    }

    /* Do the block being inserted, and the block it is being inserted after
     * make a contiguous block of memory? */
    puc = (uint8_t *) pxIterator;

    if ((puc + pxIterator->xBlockSize) == (uint8_t *) pxBlockToInsert)
    {
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert = pxIterator;
    }
    else
    {
        //mtCOVERAGE_TEST_MARKER();
    }

    /* Do the block being inserted, and the block it is being inserted before
     * make a contiguous block of memory? */
    puc = (uint8_t *) pxBlockToInsert;

    if ((puc + pxBlockToInsert->xBlockSize) == (uint8_t *) pxIterator->pxNextFreeBlock)
    {
        if (pxIterator->pxNextFreeBlock != pxEnd)
        {
            /* Form one big block from the two blocks. */
            pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
            pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
        }
        else
        {
            pxBlockToInsert->pxNextFreeBlock = pxEnd;
        }
    }
    else
    {
        pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
    }

    /* If the block being inserted plugged a gab, so was merged with the block
     * before and the block after, then it's pxNextFreeBlock pointer will have
     * already been set, and should not be set here as that would make it point
     * to itself. */
    if (pxIterator != pxBlockToInsert)
    {
        pxIterator->pxNextFreeBlock = pxBlockToInsert;
    }
    else
    {
        //mtCOVERAGE_TEST_MARKER();
    }
}
/*-----------------------------------------------------------*/
#if 0
void psRamGetHeapStats(HeapStats_t *pxHeapStats)
{
    psRam_BlockLink_t *pxBlock;
    size_t xBlocks = 0, xMaxSize = 0,
           xMinSize = portMAX_DELAY; /* portMAX_DELAY used as a portable way of getting the maximum value. */

    vTaskSuspendAll();
    {
        pxBlock = xStart.pxNextFreeBlock;

        /* pxBlock will be NULL if the heap has not been initialised.  The heap
         * is initialised automatically when the first allocation is made. */
        if (pxBlock != NULL)
        {
            while (pxBlock != pxEnd)
            {
                /* Increment the number of blocks and record the largest block seen
                 * so far. */
                xBlocks++;

                if (pxBlock->xBlockSize > xMaxSize)
                {
                    xMaxSize = pxBlock->xBlockSize;
                }

                if (pxBlock->xBlockSize < xMinSize)
                {
                    xMinSize = pxBlock->xBlockSize;
                }

                /* Move to the next block in the chain until the last block is
                 * reached. */
                pxBlock = pxBlock->pxNextFreeBlock;
            }
        }
    }
    (void) xTaskResumeAll();

    pxHeapStats->xSizeOfLargestFreeBlockInBytes = xMaxSize;
    pxHeapStats->xSizeOfSmallestFreeBlockInBytes = xMinSize;
    pxHeapStats->xNumberOfFreeBlocks = xBlocks;

    taskENTER_CRITICAL();
    {
        pxHeapStats->xAvailableHeapSpaceInBytes = xFreeBytesRemaining;
        pxHeapStats->xNumberOfSuccessfulAllocations = xNumberOfSuccessfulAllocations;
        pxHeapStats->xNumberOfSuccessfulFrees = xNumberOfSuccessfulFrees;
        pxHeapStats->xMinimumEverFreeBytesRemaining = xMinimumEverFreeBytesRemaining;
    }
    taskEXIT_CRITICAL();
}

#endif
/*-----------------------------------------------------------*/
