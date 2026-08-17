/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "rtl_nvic.h"
#include "rtl_pinmux.h"
#include "rtl_rcc.h"

#include "rtl_uart.h"
#include "rtl_gdma.h"
#include "trace.h"
/* Defines ------------------------------------------------------------------*/
#define UART_TX_PIN                P3_0
#define UART_RX_PIN                P3_1

#define UART_DEMO                   UART3
#define UART_IRQ                    UART3_IRQn
#define UART_HANDLER                UART3_Handler

#define UART_TX_GDMA_CHANNEL_NUM    GDMA_CH_NUM3
#define UART_TX_GDMA_CHANNEL        GDMA_Channel3
#define UART_TX_GDMA_CHANNEL_IRQN   GDMA_Channel3_IRQn
#define UART_TX_GDMA_Handler        GDMA_Channel3_Handler

#define UART_RX_GDMA_CHANNEL_NUM    GDMA_CH_NUM4
#define UART_RX_GDMA_CHANNEL        GDMA_Channel4
#define UART_RX_GDMA_CHANNEL_IRQN   GDMA_Channel4_IRQn
#define UART_RX_GDMA_Handler        GDMA_Channel4_Handler

/* Set 1 if Allow GDMA TX*/
#define NOT_ALLOW_DEINIT        1

#define GDMA_TRANSFER_SIZE      100
#define GDMA_BLOCK_SIZE         5
uint8_t GDMA_Rx_Buf[GDMA_TRANSFER_SIZE];
uint8_t GDMA_Tx_Buf[GDMA_TRANSFER_SIZE];

uint8_t receive_offset = 0;
uint8_t count = 0;
volatile bool receiveflg = false;

/* Globals ------------------------------------------------------------------*/
typedef struct
{
    uint16_t div;
    uint16_t ovsr;
    uint16_t ovsr_adj;
} UART_BaudRate_TypeDef;

typedef enum
{
    BAUD_RATE_9600,
    BAUD_RATE_19200,
    BAUD_RATE_115200,
    BAUD_RATE_230400,
    BAUD_RATE_460800,
    BAUD_RATE_921600,
    BAUD_RATE_1000000,
    BAUD_RATE_2000000,
    BAUD_RATE_3000000
} UartBaudRate_TypeDef;

const UART_BaudRate_TypeDef BaudRate_Table[9] =
{
    {0x341, 0, 0}, // BAUD_RATE_9600
    {0x150, 1, 3}, // BAUD_RATE_19200
    {0x38,  1, 3}, // BAUD_RATE_115200
    {0x1C,  1, 3}, // BAUD_RATE_230400
    {0xE,   1, 3}, // BAUD_RATE_460800
    {0x7,   1, 3}, // BAUD_RATE_921600
    {0x8,   0, 0}, // BAUD_RATE_1000000
    {0x4,   0, 0}, // BAUD_RATE_2000000
    {0x1,   8, 0x92}, // BAUD_RATE_3000000
};

/****************************************************************************/
/* board init                                                               */
/****************************************************************************/
/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_uart_init(void)
{
    Pad_Config(UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);

    Pinmux_Config(UART_TX_PIN, UART3_TX);
    Pinmux_Config(UART_RX_PIN, UART3_RX);
}

/****************************************************************************/
/* UART driver init                                                         */
/****************************************************************************/
/**
  * @brief  Initialize uart peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_uart_init(void)
{
    UART_DeInit(UART_DEMO);

    RCC_PeriphClockCmd(APBPeriph_UART3, APBPeriph_UART3_CLOCK, ENABLE);
    /* uart init */
    UART_InitTypeDef UART_InitStruct;
    UART_StructInit(&UART_InitStruct);

    /* Config uart baudrate */
    UART_InitStruct.UART_Div            = BaudRate_Table[BAUD_RATE_115200].div;
    UART_InitStruct.UART_Ovsr           = BaudRate_Table[BAUD_RATE_115200].ovsr;
    UART_InitStruct.UART_OvsrAdj        = BaudRate_Table[BAUD_RATE_115200].ovsr_adj;

    UART_InitStruct.UART_Parity         = UART_PARITY_NO_PARTY;
    UART_InitStruct.UART_StopBits       = UART_STOP_BITS_1;
    UART_InitStruct.UART_WordLen        = UART_WORD_LENGTH_8BIT;
    UART_InitStruct.UART_RxThdLevel     = 8; //1~29
    UART_InitStruct.UART_IdleTime       = UART_RX_IDLE_2BYTE; //idle interrupt wait time
    UART_InitStruct.UART_TxWaterLevel   = 1; //Better to equal GDMA_MSize
    UART_InitStruct.UART_RxWaterLevel   = 1; //Better to equal GDMA_MSize
    UART_InitStruct.TxDmaEn             = ENABLE;
    UART_InitStruct.RxDmaEn             = ENABLE;
    UART_InitStruct.dmaEn               = UART_DMA_ENABLE;
    UART_Init(UART_DEMO, &UART_InitStruct);

    /*  Enable UART Interrupt  */
    UART_INTConfig(UART_DEMO, UART_INT_RX_IDLE | UART_INT_LINE_STS, ENABLE);

    /*  Enable UART IRQ  */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = UART_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/****************************************************************************/
/* GDMA_Channel3 driver init                                                */
/****************************************************************************/
void driver_gdma3_init(void)
{
    /* Initialize GDMA */
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = UART_TX_GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_MemoryToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize          = 0;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)GDMA_Tx_Buf;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)(&(UART_DEMO->UART_RBR_THR));
    GDMA_InitStruct.GDMA_DestHandshake       = GDMA_Handshake_UART3_TX;
    GDMA_Init(UART_TX_GDMA_CHANNEL, &GDMA_InitStruct);
    /* Enable transfer finish interrupt */
    GDMA_INTConfig(UART_TX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
    /* Configure NVIC of GDMA */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = UART_TX_GDMA_CHANNEL_IRQN;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

}

/****************************************************************************/
/* GDMA_Channel4 driver init                                                */
/****************************************************************************/
void driver_gdma4_init(void)
{
    /* Initialize GDMA */
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = UART_RX_GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_PeripheralToMemory;
    GDMA_InitStruct.GDMA_BufferSize          = GDMA_BLOCK_SIZE;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Inc;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)(&(UART_DEMO->UART_RBR_THR));
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)GDMA_Rx_Buf;
    GDMA_InitStruct.GDMA_SourceHandshake     = GDMA_Handshake_UART3_RX;

    GDMA_Init(UART_RX_GDMA_CHANNEL, &GDMA_InitStruct);
    /* Enable transfer finish interrupt */
    GDMA_INTConfig(UART_RX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
    /* Configure NVIC of GDMA */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = UART_RX_GDMA_CHANNEL_IRQN;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    GDMA_Cmd(UART_RX_GDMA_CHANNEL_NUM, ENABLE);
}

/****************************************************************************/
/* UART Handler                                                             */
/****************************************************************************/
void UART3_Handler(void)
{
    uint8_t tmp;
    uint32_t data_len = 0;
    uint32_t int_status = UART_GetIID(UART3);
    if (UART_GetFlagState(UART3, UART_FLAG_RX_IDLE) == SET)
    {
        DBG_DIRECT("UART_FLAG_RX_IDLE");

        /*  Suspend GDMA_Channel2   */
        GDMA_SafeSuspend(UART_RX_GDMA_CHANNEL);
        UART_INTConfig(UART3, UART_INT_RX_IDLE, DISABLE);

        data_len = GDMA_GetTransferLen(UART_RX_GDMA_CHANNEL);
        for (uint32_t i = 0; i < data_len; i++)
        {
            DBG_DIRECT("value is 0x%x", GDMA_Rx_Buf[i]);
        }

        if (data_len)
        {
            receive_offset += data_len;
            memcpy(GDMA_Tx_Buf + GDMA_BLOCK_SIZE * count, GDMA_Rx_Buf, data_len);

            for (uint32_t i = 0; i < GDMA_BLOCK_SIZE * count + data_len; i++)
            {
                /*  Just for debug, DBG_DIRECT will waste of time leading to loss of data */
                //DBG_DIRECT("offset: %d, tx data is 0x%x", i, GDMA_Tx_Buf[i]);
            }

        }
        /* DMA TX flag */
        receiveflg = true;
        driver_gdma4_init();
        UART_ClearRxFIFO(UART3);
        UART_INTConfig(UART3, UART_INT_RX_IDLE, ENABLE);
    }

    UART_INTConfig(UART3, UART_INT_RD_AVA | UART_INT_RX_LINE_STS, DISABLE);

    switch (int_status & 0x0E)
    {
    case UART_INT_ID_LINE_STATUS:
        {
            DBG_DIRECT("Line status error! 0x%x", UART3->UART_LSR);
            break;
        }
    case UART_INT_ID_RX_LEVEL_REACH:
        {
            DBG_DIRECT("UART_INT_ID_RX_LEVEL_REACH: rx fifo len: %d", UART_GetRxFIFODataLen(UART3));
            break;
        }
    case UART_INT_ID_RX_TMEOUT:
        {
            DBG_DIRECT("UART_INT_ID_RX_TMEOUT.");
            while (UART_GetFlagState(UART3, UART_FLAG_RX_DATA_RDY) == SET)
            {
                UART_ReceiveData(UART3, &tmp, 1);
            }
            break;
        }
    default:
        {
            break;
        }
    }
    UART_INTConfig(UART3, UART_INT_RD_AVA | UART_INT_RX_LINE_STS, ENABLE);
}

/****************************************************************************/
/* GDMA3 Handler                                                             */
/****************************************************************************/
void GDMA0_Channel3_Handler(void)
{
    DBG_DIRECT("UART_TX_GDMA_Handler");
    GDMA_Cmd(UART_TX_GDMA_CHANNEL_NUM, DISABLE);
    GDMA_ClearAllTypeINT(UART_TX_GDMA_CHANNEL_NUM);
}

/****************************************************************************/
/* GDMA4 Handler                                                             */
/****************************************************************************/
void GDMA0_Channel4_Handler(void)
{
    DBG_DIRECT("GDMA0_Channel4_Handler");

    /*  Clear interrupt */
    GDMA_Cmd(UART_RX_GDMA_CHANNEL_NUM, DISABLE);
    GDMA_ClearAllTypeINT(UART_RX_GDMA_CHANNEL_NUM);
    receive_offset += GDMA_BLOCK_SIZE;
    count += 1;

    /*print information*/
    for (uint32_t i = 0; i < GDMA_BLOCK_SIZE; i++)
    {
        /*  Just for debug, DBG_DIRECT will waste of time leading to loss of data */
        //DBG_DIRECT("Rx value is 0x%x", GDMA_Rx_Buf[i]);
    }
    memcpy(GDMA_Tx_Buf + GDMA_BLOCK_SIZE * (count - 1), GDMA_Rx_Buf, GDMA_BLOCK_SIZE);

    GDMA_ClearINTPendingBit(UART_RX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
    /*  reset gdma param    */
    GDMA_SetDestinationAddress(UART_RX_GDMA_CHANNEL, (uint32_t)GDMA_Rx_Buf);
    GDMA_Cmd(UART_RX_GDMA_CHANNEL_NUM, ENABLE);
}

/****************************************************************************/
/* main                                                                     */
/****************************************************************************/
/**
  * @brief    Entry of app code
  * @return   int (To avoid compile warning)
  */
void uart_demo(void)
{
    board_uart_init();

    driver_uart_init();

    /* GDMA Channel For Rx*/
    driver_gdma4_init();

    /* GDMA Channel For Tx*/
    driver_gdma3_init();

    while (1)
    {
        if (receiveflg)
        {
            GDMA_SetBufferSize(UART_TX_GDMA_CHANNEL, receive_offset);
            GDMA_Cmd(UART_TX_GDMA_CHANNEL_NUM, ENABLE);
            receive_offset = 0;
            count = 0;
            receiveflg = false;
        }
    }
}


