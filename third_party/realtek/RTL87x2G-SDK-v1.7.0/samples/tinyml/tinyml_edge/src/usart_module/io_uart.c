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
#include "uart_packet_parser.h"
#include "app_msg.h"
#include "ts_queue.h"
#include "tinyml_main.h"

ts_queue_t *g_ts_uart_data_list = NULL;

/* Private typedef -----------------------------------------------------------*/
const UART_BaudRate_TypeDef BaudRate_Table[11] =
{
    {271, 10, 0x24A}, // BAUD_RATE_9600
    {150, 8,  0x3EF}, // BAUD_RATE_19200
    {20, 12,  0x252}, // BAUD_RATE_115200
    {11,  10, 0x3BB}, // BAUD_RATE_230400
    {11,  9,  0x084}, // BAUD_RATE_256000
    {7,   9,  0x3EF}, // BAUD_RATE_384000
    {6,   9,  0x0AA}, // BAUD_RATE_460800
    {3,   9,  0x0AA}, // BAUD_RATE_921600
    {4,   5,  0},     // BAUD_RATE_1000000
    {2,   5,  0},     // BAUD_RATE_2000000
    {1,   8,  0x292}, // BAUD_RATE_3000000
};

uint8_t String_Buf[100];
uint8_t UART_Rev_Temp_Buf[64]; // UART RX FIFO depth is 64 bytes
uint8_t UART_Recv_Data_Buff[2048];
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
    UART_InitStruct.UART_RxThdLevel     = 4;
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
void uart_init(void)
{
    uint16_t demoStrLen = 0;

    board_uart_init();
    driver_uart_init();

    char *demoStr = "### Uart interrupt demo ###\r\n";
    demoStrLen = strlen(demoStr);
    memcpy(String_Buf, demoStr, demoStrLen);
    uart_senddata_continuous(UART_DEMO, String_Buf, demoStrLen);


}

void uart_senddata(uint8_t *pSend_Buf, uint16_t vCount)
{
    if (pSend_Buf == NULL || vCount == 0)
    {
        return;
    }

    uart_senddata_continuous(UART_DEMO, pSend_Buf, vCount);
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
        //DBG_DIRECT("UART_FLAG_RX_IDLE");

        UART_INTConfig(UART_DEMO, UART_INT_RX_IDLE, DISABLE);

        UART_ClearRxFIFO(UART_DEMO);
        UART_INTConfig(UART_DEMO, UART_INT_RX_IDLE, ENABLE);

        if (UART_Recv_Buf_Lenth > 0)
        {
            g_ts_uart_data_list = ts_queue_add_data(g_ts_uart_data_list, UART_Recv_Data_Buff,
                                                    UART_Recv_Buf_Lenth);
            T_IO_MSG uart_msg = {.type = IO_MSG_TYPE_UART, .subtype = IO_MSG_UART_RX_DONE, .u.buf = &g_ts_uart_data_list};
            tinyml_send_msg_to_task(&uart_msg);

            UART_Recv_Buf_Lenth = 0;
        }
    }

    switch (int_status & 0x0E)
    {
    case UART_INT_ID_RX_DATA_TIMEOUT:
        {
            lenth = UART_GetRxFIFODataLen(UART_DEMO);
            if (lenth > sizeof(UART_Rev_Temp_Buf)) { lenth = sizeof(UART_Rev_Temp_Buf); }
            UART_ReceiveData(UART_DEMO, UART_Rev_Temp_Buf, lenth);

            //DBG_DIRECT("UART_INT_ID_RX_TMEOUT, len %d", UART_Recv_Buf_Lenth+lenth);

            memcpy(UART_Recv_Data_Buff + UART_Recv_Buf_Lenth, UART_Rev_Temp_Buf,  lenth);
            UART_Recv_Buf_Lenth += lenth;

            g_ts_uart_data_list = ts_queue_add_data(g_ts_uart_data_list, UART_Recv_Data_Buff,
                                                    UART_Recv_Buf_Lenth);
            T_IO_MSG uart_msg = {.type = IO_MSG_TYPE_UART, .subtype = IO_MSG_UART_RX_DONE, .u.buf = &g_ts_uart_data_list};
            tinyml_send_msg_to_task(&uart_msg);

            UART_Recv_Buf_Lenth = 0;

            break;
        }
    case UART_INT_ID_LINE_STATUS:
        break;

    case UART_INT_ID_RX_LEVEL_REACH:
        {
            lenth = UART_GetRxFIFODataLen(UART_DEMO);
            if (lenth > sizeof(UART_Rev_Temp_Buf)) { lenth = sizeof(UART_Rev_Temp_Buf); }
            UART_ReceiveData(UART_DEMO, UART_Rev_Temp_Buf, lenth);

            memcpy(UART_Recv_Data_Buff + UART_Recv_Buf_Lenth, UART_Rev_Temp_Buf,  lenth);
            UART_Recv_Buf_Lenth += lenth;

            //DBG_DIRECT("UART_INT_ID_RX_LEVEL_REAC, len %d", UART_Recv_Buf_Lenth);
            break;
        }
    case UART_INT_ID_TX_EMPTY:
        break;

    default:
        break;
    }

    UART_INTConfig(UART_DEMO, UART_INT_RD_AVA, ENABLE);
}

