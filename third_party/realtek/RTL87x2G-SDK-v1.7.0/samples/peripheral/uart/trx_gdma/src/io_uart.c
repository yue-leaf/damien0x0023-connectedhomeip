/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "rtl_rcc.h"
#include "rtl_pinmux.h"
#include "rtl_nvic.h"
#include "trace.h"

#include "rtl_uart.h"
#include "rtl_gdma.h"
#include "io_uart.h"

/* Private typedef -----------------------------------------------------------*/
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

/* Private define ------------------------------------------------------------*/
#define UART_TX_PIN                 P3_0
#define UART_RX_PIN                 P3_1

#define UART_DEMO                   UART3
#define UART_IRQ                    UART3_IRQn
#define UART_HANDLER                UART3_Handler

#define UART_TX_GDMA_CHANNEL_NUM    GDMA_CH_NUM3
#define UART_TX_GDMA_CHANNEL        GDMA_Channel3
#define UART_TX_GDMA_CHANNEL_IRQN   GDMA_Channel3_IRQn
#define UART_TX_GDMA_Handler        GDMA_Channel3_Handler
#define UART_TX_GDMA_BUFFER_SIZE    29//max size=65535

#define UART_RX_GDMA_CHANNEL_NUM    GDMA_CH_NUM4
#define UART_RX_GDMA_CHANNEL        GDMA_Channel4
#define UART_RX_GDMA_CHANNEL_IRQN   GDMA_Channel4_IRQn
#define UART_RX_GDMA_Handler        GDMA_Channel4_Handler
#define UART_RX_GDMA_BUFFER_SIZE    29//max size=65535

/* Globals -------------------------------------------------------------------*/
uint8_t GDMA_SendData_Buffer[UART_TX_GDMA_BUFFER_SIZE];
uint8_t GDMA_ReceiveData_Buffer[UART_RX_GDMA_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  None.
  * @retval None.
  */
void board_uart_init(void)
{
    Pad_Config(UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);

    Pinmux_Config(UART_TX_PIN, UART3_TX);
    Pinmux_Config(UART_RX_PIN, UART3_RX);
}
/**
  * @brief  Initialize UART peripheral.
  * @param  None.
  * @retval None.
  */
void driver_uart_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_UART3, APBPeriph_UART3_CLOCK, ENABLE);

    /* uart init */
    UART_InitTypeDef UART_InitStruct;
    UART_StructInit(&UART_InitStruct);
    UART_InitStruct.UART_Div            = BaudRate_Table[BAUD_RATE_115200].div;
    UART_InitStruct.UART_Ovsr           = BaudRate_Table[BAUD_RATE_115200].ovsr;
    UART_InitStruct.UART_OvsrAdj        = BaudRate_Table[BAUD_RATE_115200].ovsr_adj;
    UART_InitStruct.UART_Parity         = UART_PARITY_NO_PARTY;
    UART_InitStruct.UART_StopBits       = UART_STOP_BITS_1;
    UART_InitStruct.UART_WordLen        = UART_WORD_LENGTH_8BIT;
    UART_InitStruct.UART_RxThdLevel     = 16;
    UART_InitStruct.UART_IdleTime       = UART_RX_IDLE_2BYTE;

    UART_InitStruct.UART_DmaEn          = UART_DMA_ENABLE;
    UART_InitStruct.UART_TxWaterLevel   = 1; /* Better to equal: GDMA_MSize */
    UART_InitStruct.UART_RxWaterLevel   = 1; /* Better to equal: GDMA_MSize */
    UART_InitStruct.UART_TxDmaEn        = ENABLE;
    UART_InitStruct.UART_RxDmaEn        = ENABLE;
    UART_Init(UART_DEMO, &UART_InitStruct);
}

/**
  * @brief  UARt send data continuous.
  * @param  No parameter.
  * @return void
*/
void uart_senddata_continuous(UART_TypeDef *UARTx, const uint8_t *pSend_Buf, uint16_t vCount)
{
    uint8_t count;
    while (vCount / UART_TX_FIFO_SIZE > 0)
    {
        while (UART_GetFlagStatus(UARTx, UART_FLAG_TX_FIFO_EMPTY) == 0);
        for (count = UART_TX_FIFO_SIZE; count > 0; count--)
        {
            UARTx->UART_RBR_THR = *pSend_Buf++;
        }
        vCount -= UART_TX_FIFO_SIZE;
    }

    while (UART_GetFlagStatus(UARTx, UART_FLAG_TX_FIFO_EMPTY) == 0);
    while (vCount--)
    {
        UARTx->UART_RBR_THR = *pSend_Buf++;
    }
}

void UART_SendDataByGDMA(void)
{
    /*--------------initialize test buffer which for sending data to UART---------------------*/
    for (uint32_t i = 0; i < UART_TX_GDMA_BUFFER_SIZE; i++)
    {
        GDMA_SendData_Buffer[i] = 0x10 + i;
    }

    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    /*--------------GDMA init-----------------------------*/
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum       = UART_TX_GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR              = GDMA_DIR_MemoryToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize       = UART_TX_GDMA_BUFFER_SIZE;
    GDMA_InitStruct.GDMA_SourceInc        = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc   = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceDataSize   = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize      = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_SourceAddr       = (uint32_t)GDMA_SendData_Buffer;
    GDMA_InitStruct.GDMA_DestinationAddr  = (uint32_t)(&(UART_DEMO->UART_RBR_THR));
    GDMA_InitStruct.GDMA_DestHandshake    = GDMA_Handshake_UART3_TX;
    GDMA_InitStruct.GDMA_ChannelPriority  = 2;
    GDMA_Init(UART_TX_GDMA_CHANNEL, &GDMA_InitStruct);

    GDMA_INTConfig(UART_TX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    /*-----------------GDMA IRQ init-------------------*/
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = UART_TX_GDMA_CHANNEL_IRQN;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    /*-----------------start to send data-----------*/
    GDMA_Cmd(UART_TX_GDMA_CHANNEL_NUM, ENABLE);
}

void UART_ReceiveDataByGDMA(void)
{
    /*--------------initialize test buffer which for sending data to UART---------------------*/
    for (uint32_t i = 0; i < UART_RX_GDMA_BUFFER_SIZE; i++)
    {
        GDMA_ReceiveData_Buffer[i] = 0;
    }

    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    /*--------------GDMA init-----------------------------*/
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum       = UART_RX_GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR              = GDMA_DIR_PeripheralToMemory;
    GDMA_InitStruct.GDMA_BufferSize       = UART_RX_GDMA_BUFFER_SIZE;
    GDMA_InitStruct.GDMA_SourceInc        = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc   = DMA_DestinationInc_Inc;
    GDMA_InitStruct.GDMA_SourceDataSize   = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize      = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_SourceAddr       = (uint32_t)(&(UART_DEMO->UART_RBR_THR));
    GDMA_InitStruct.GDMA_SourceHandshake  = GDMA_Handshake_UART3_RX;
    GDMA_InitStruct.GDMA_DestinationAddr  = (uint32_t)GDMA_ReceiveData_Buffer;
    GDMA_InitStruct.GDMA_ChannelPriority  = 2;
    GDMA_Init(UART_RX_GDMA_CHANNEL, &GDMA_InitStruct);

    GDMA_INTConfig(UART_RX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    /*-----------------GDMA IRQ init-------------------*/
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = UART_RX_GDMA_CHANNEL_IRQN;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    /*-----------------start to send data-----------*/
    GDMA_Cmd(UART_RX_GDMA_CHANNEL_NUM, ENABLE);
}

/**
  * @brief  Demo code of uart.
  * @param  No parameter.
  * @return void
*/
void uart_demo(void)
{
    uint16_t demoStrLen = 0;

    board_uart_init();
    driver_uart_init();

    uint8_t String_Buf[100];
    char *demoStr = "### Uart trx gdma sample ###\r\n";
    demoStrLen = strlen(demoStr);
    memcpy(String_Buf, demoStr, demoStrLen);
    uart_senddata_continuous(UART_DEMO, String_Buf, demoStrLen);

    /* Init GDMA for uart send data */
    UART_SendDataByGDMA();
    /* Init GDMA for uart rx data */
    UART_ReceiveDataByGDMA();
}

/**
  * @brief  GDMA interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void UART_TX_GDMA_Handler(void)
{
    GDMA_ClearINTPendingBit(UART_TX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer);

    DBG_DIRECT("UART_TX_GDMA_Handler, Data transmission completion!");

#if 0
    /* cyclic transmission */
    GDMA_SetSourceAddress(UART_RX_GDMA_CHANNEL, (uint32_t)(GDMA_SendData_Buffer));
    GDMA_SetDestinationAddress(UART_RX_GDMA_CHANNEL, (uint32_t)(&(UART_DEMO->UART_RBR_THR)));
    GDMA_SetBufferSize(UART_RX_GDMA_CHANNEL, UART_TX_GDMA_BUFFER_SIZE);
    GDMA_ClearINTPendingBit(UART_TX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
    GDMA_Cmd(UART_TX_GDMA_CHANNEL_NUM, ENABLE);
#endif
}

/**
  * @brief  GDMA interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void UART_RX_GDMA_Handler(void)
{
    GDMA_ClearINTPendingBit(UART_RX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer);

    DBG_DIRECT("UART_RX_GDMA_Handler, Data transmission completion!");

    uart_senddata_continuous(UART_DEMO, GDMA_ReceiveData_Buffer, UART_RX_GDMA_BUFFER_SIZE);

#if 0
    /* cyclic reception */
    GDMA_SetSourceAddress(UART_RX_GDMA_CHANNEL, (uint32_t)(&(UART_DEMO->UART_RBR_THR)));
    GDMA_SetDestinationAddress(UART_RX_GDMA_CHANNEL, (uint32_t)(GDMA_ReceiveData_Buffer));
    GDMA_SetBufferSize(UART_RX_GDMA_CHANNEL, UART_RX_GDMA_BUFFER_SIZE);
    GDMA_ClearINTPendingBit(UART_RX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
    GDMA_Cmd(UART_RX_GDMA_CHANNEL_NUM, ENABLE);
#endif
}


