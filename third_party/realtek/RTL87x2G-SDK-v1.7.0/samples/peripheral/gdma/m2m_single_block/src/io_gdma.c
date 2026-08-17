/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "rtl_rcc.h"
#include "rtl_nvic.h"
#include "rtl_gdma.h"
#include "trace.h"

#include "io_gdma.h"

/* Globals ------------------------------------------------------------------*/
uint32_t GDMA_Send_Buf[GDMA_TRANSFER_SIZE];
uint32_t GDMA_Recv_Buf[GDMA_TRANSFER_SIZE];

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_gdma_init(void)
{
    uint32_t i = 0;

    /* Test data buffer */
    for (i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        GDMA_Send_Buf[i] = i + 1;
        GDMA_Recv_Buf[i] = 0;
    }

    /* Turn on gdma clock */
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    /* GDMA initial*/
    GDMA_InitStruct.GDMA_ChannelNum          = GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_MemoryToMemory;
    GDMA_InitStruct.GDMA_BufferSize          = GDMA_TRANSFER_SIZE;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Inc;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_8;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_8;
    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)GDMA_Send_Buf;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)GDMA_Recv_Buf;

    GDMA_Init(GDMA_Channel, &GDMA_InitStruct);

    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    /* GDMA irq init */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);

    /* Enable GDMA transmission, User can move to where based on need */
    GDMA_Cmd(GDMA_CHANNEL_NUM, ENABLE);
}

/**
  * @brief  GDMA channel interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void GDMA_Channel_Handler(void)
{
    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Transfer, DISABLE);

    DBG_DIRECT("GDMA_Channel_Handler: Data transmission completion!");

    /* Compare whether the destination data which transported by GDMA is equal to the source data*/
    for (uint32_t i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        if (GDMA_Send_Buf[i] != GDMA_Recv_Buf[i])
        {
            DBG_DIRECT("GDMA_Channel_Handler: Data transmission error! GDMA_Send_Buf = %d, GDMA_Recv_Buf = %d",
                       GDMA_Send_Buf[i], GDMA_Recv_Buf[i]);
        }
    }

    GDMA_ClearINTPendingBit(GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
}


