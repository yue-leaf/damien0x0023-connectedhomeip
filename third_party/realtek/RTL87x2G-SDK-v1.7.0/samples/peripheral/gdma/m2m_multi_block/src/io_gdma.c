/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "io_gdma.h"

/* Private typedef -----------------------------------------------------------*/
#define GDMA_MULTIBLOCK_MODE            LLI_TRANSFER

#define INT_TRANSFER                    1
#define INT_BLOCK                       0

#if INT_TRANSFER
#define GDMA_INT                        GDMA_INT_Transfer
#endif
#if INT_BLOCK
#define GDMA_INT                        GDMA_INT_Block
static uint8_t GDMA_INT_Block_Counter = 0;
#endif

/* Globals ------------------------------------------------------------------*/
uint8_t GDMA_Send_Buf[GDMA_MULTIBLOCK_SIZE][GDMA_TRANSFER_SIZE];
uint8_t GDMA_Recv_Buf[GDMA_MULTIBLOCK_SIZE][GDMA_TRANSFER_SIZE];
GDMA_LLIDef GDMA_LLIStruct[GDMA_MULTIBLOCK_SIZE];

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_gdma_init(void)
{
    /*--------------Initialize test buffer---------------------*/
    for (uint32_t i = 0; i < GDMA_MULTIBLOCK_SIZE; i++)
    {
        for (uint32_t j = 0; j < GDMA_TRANSFER_SIZE; j++)
        {
            GDMA_Send_Buf[i][j] = (i + j) & 0xFF;
            GDMA_Recv_Buf[i][j] = 0;
        }
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
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)GDMA_Send_Buf;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)GDMA_Recv_Buf;

    /* Config GDMA multi-block */
    GDMA_InitStruct.GDMA_Multi_Block_En     = ENABLE;
    GDMA_InitStruct.GDMA_Multi_Block_Mode   = GDMA_MULTIBLOCK_MODE;//LLI_TRANSFER;
    GDMA_InitStruct.GDMA_Multi_Block_Struct = (uint32_t)GDMA_LLIStruct;
    for (uint32_t i = 0; i < GDMA_MULTIBLOCK_SIZE; i++)
    {
        if (i == GDMA_MULTIBLOCK_SIZE - 1)
        {
            //GDMA_LLIStruct[i].LLP=0;
            GDMA_LLIStruct[i].SAR = (uint32_t)GDMA_Send_Buf[i];
            GDMA_LLIStruct[i].DAR = (uint32_t)GDMA_Recv_Buf[i];
            GDMA_LLIStruct[i].LLP = 0;
            /* Configure low 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_LOW = BIT(0)
                                        | (GDMA_InitStruct.GDMA_DestinationDataSize << 1)
                                        | (GDMA_InitStruct.GDMA_SourceDataSize << 4)
                                        | (GDMA_InitStruct.GDMA_DestinationInc << 7)
                                        | (GDMA_InitStruct.GDMA_SourceInc << 9)
                                        | (GDMA_InitStruct.GDMA_DestinationMsize << 11)
                                        | (GDMA_InitStruct.GDMA_SourceMsize << 14)
                                        | (GDMA_InitStruct.GDMA_DIR << 20);
            /* Configure high 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_HIGH = GDMA_InitStruct.GDMA_BufferSize;
        }
        else
        {
            GDMA_LLIStruct[i].SAR = (uint32_t)GDMA_Send_Buf[i];
            GDMA_LLIStruct[i].DAR = (uint32_t)GDMA_Recv_Buf[i];
            GDMA_LLIStruct[i].LLP = (uint32_t)&GDMA_LLIStruct[i + 1];
            /* Configure low 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_LOW = BIT(0)
                                        | (GDMA_InitStruct.GDMA_DestinationDataSize << 1)
                                        | (GDMA_InitStruct.GDMA_SourceDataSize << 4)
                                        | (GDMA_InitStruct.GDMA_DestinationInc << 7)
                                        | (GDMA_InitStruct.GDMA_SourceInc << 9)
                                        | (GDMA_InitStruct.GDMA_DestinationMsize << 11)
                                        | (GDMA_InitStruct.GDMA_SourceMsize << 14)
                                        | (GDMA_InitStruct.GDMA_DIR << 20)
                                        | (GDMA_InitStruct.GDMA_Multi_Block_Mode & LLP_SELECTED_BIT);
            /* Configure high 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_HIGH = GDMA_InitStruct.GDMA_BufferSize;
        }
    }
    GDMA_Init(GDMA_Channel, &GDMA_InitStruct);

    /* GDMA irq init */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);

    /* Enable GDMA Interrupt */
    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT, ENABLE);

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
    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT, DISABLE);

    DBG_DIRECT("GDMA_Channel_Handler: Data transmission completion!");

#if INT_TRANSFER
    /* Compare whether the destination data which transported by GDMA is equal to the source data*/
    for (uint32_t i = 0; i < GDMA_MULTIBLOCK_SIZE; i++)
    {
        for (uint32_t j = 0; j < GDMA_TRANSFER_SIZE; j++)
        {
            if (GDMA_Send_Buf[i][j] != GDMA_Recv_Buf[i][j])
            {
                DBG_DIRECT("GDMA_Channel_Handler: Data transmission error! GDMA_Send_Buf = %d, GDMA_Recv_Buf = %d",
                           GDMA_Send_Buf[i][j], GDMA_Recv_Buf[i][j]);
            }
        }
    }

    GDMA_ClearINTPendingBit(GDMA_CHANNEL_NUM, GDMA_INT);
#endif

#if INT_BLOCK
    /* Compare whether the destination data which transported by GDMA is equal to the source data*/
    for (uint32_t j = 0; j < GDMA_TRANSFER_SIZE; j++)
    {
        if (GDMA_Send_Buf[GDMA_INT_Block_Counter][j] != GDMA_Recv_Buf[GDMA_INT_Block_Counter][j])
        {
            DBG_DIRECT("GDMA_Channel_Handler: Data transmission error! GDMA_Send_Buf = %d, GDMA_Recv_Buf = %d",
                       GDMA_Send_Buf[GDMA_INT_Block_Counter][j], GDMA_Recv_Buf[GDMA_INT_Block_Counter][j]);
        }
    }

    /* Update block counter when every single block transport completel */
    GDMA_INT_Block_Counter++;
    if (GDMA_INT_Block_Counter < GDMA_MULTIBLOCK_SIZE)
    {
        GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Block, ENABLE);
    }
    else
    {
        GDMA_INT_Block_Counter = 0;
    }
#endif
}


