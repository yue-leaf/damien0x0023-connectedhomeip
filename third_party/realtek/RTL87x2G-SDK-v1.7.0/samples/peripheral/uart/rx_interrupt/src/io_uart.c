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

uint8_t String_Buf[100];
uint8_t UART_Recv_Buf[256];
uint8_t UART_Send_Buf[256];
uint16_t UART_Recv_Buf_Lenth = 0;
volatile bool receive_flag = false;

/* Private define ------------------------------------------------------------*/
#define UART_TX_PIN                 P3_0
#define UART_RX_PIN                 P3_1

#define UART_DEMO                   UART3
#define UART_IRQ                    UART3_IRQn
#define UART_HANDLER                UART3_Handler
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
    UART_InitStruct.UART_RxThdLevel     = 14;
    UART_InitStruct.UART_IdleTime       = UART_RX_IDLE_2BYTE;
    UART_Init(UART_DEMO, &UART_InitStruct);

    /* Enable UART rx data available and rx idle interrupt */
    UART_INTConfig(UART_DEMO, UART_INT_RD_AVA, ENABLE);
    UART_INTConfig(UART_DEMO, UART_INT_RX_IDLE, ENABLE);

    /* Enable UART IRQ  */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = UART_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);

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

    char *demoStr = "### Uart interrupt sample ###\r\n";
    demoStrLen = strlen(demoStr);
    memcpy(String_Buf, demoStr, demoStrLen);
    uart_senddata_continuous(UART_DEMO, String_Buf, demoStrLen);

    /* Loop rx and tx */
    while (1)
    {
        if (receive_flag == true)
        {
            receive_flag = false;
            uart_senddata_continuous(UART_DEMO, UART_Send_Buf, UART_Recv_Buf_Lenth);

            for (uint16_t i = 0; i < UART_Recv_Buf_Lenth; i++)
            {
                UART_Recv_Buf[i] = 0;
            }
            UART_Recv_Buf_Lenth = 0;
        }
    }
}

/**
  * @brief  GPIO interrupt handler function.
  * @param  None.
  * @return None.
  */
void UART_HANDLER()
{
    uint16_t lenth = 0;
    uint32_t int_status = UART_GetIID(UART_DEMO);

    UART_INTConfig(UART_DEMO, UART_INT_RD_AVA, DISABLE);

    if (UART_GetFlagStatus(UART_DEMO, UART_FLAG_RX_IDLE) == SET)
    {
        DBG_DIRECT("UART_FLAG_RX_IDLE");

        UART_INTConfig(UART_DEMO, UART_INT_RX_IDLE, DISABLE);

        UART_ClearRxFIFO(UART_DEMO);
        UART_INTConfig(UART_DEMO, UART_INT_RX_IDLE, ENABLE);
        receive_flag = true;
    }

    switch (int_status & 0x0E)
    {
    case UART_INT_ID_RX_DATA_TIMEOUT:
        {
            DBG_DIRECT("UART_INT_ID_RX_TMEOUT");

            lenth = UART_GetRxFIFODataLen(UART_DEMO);
            UART_ReceiveData(UART_DEMO, UART_Recv_Buf, lenth);
            for (uint8_t i = 0; i < lenth; i++)
            {
                DBG_DIRECT("data=0x%x", UART_Recv_Buf[i]);
                UART_Send_Buf[UART_Recv_Buf_Lenth + i] = UART_Recv_Buf[i];
            }
            UART_Recv_Buf_Lenth += lenth;
            break;
        }
    case UART_INT_ID_LINE_STATUS:
        break;

    case UART_INT_ID_RX_LEVEL_REACH:
        {
            DBG_DIRECT("UART_INT_ID_RX_LEVEL_REACH");

            lenth = UART_GetRxFIFODataLen(UART_DEMO);
            UART_ReceiveData(UART_DEMO, UART_Recv_Buf, lenth);
            for (uint8_t i = 0; i < lenth; i++)
            {
                DBG_DIRECT("data=0x%x", UART_Recv_Buf[i]);
                UART_Send_Buf[UART_Recv_Buf_Lenth + i] = UART_Recv_Buf[i];
            }
            UART_Recv_Buf_Lenth += lenth;
            break;
        }
    case UART_INT_ID_TX_EMPTY:
        break;

    default:
        break;
    }

    UART_INTConfig(UART_DEMO, UART_INT_RD_AVA, ENABLE);
}


