/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "io_uart.h"
#include "pm.h"
#include "board.h"
#include "trace.h"
#include <string.h>

uint8_t UART_RX_Buffer[256];
uint8_t UART_RX_Count = 0;
POWER_CheckResult IO_UART_DLPS_Enter_Allowed = POWER_CHECK_FAIL;

/**
  * @brief  Initialize uart global data.
  * @param  No parameter.
  * @return void
  */
void global_data_uart_init(void)
{
    IO_UART_DLPS_Enter_Allowed = POWER_CHECK_PASS;
    UART_RX_Count = 0;
    memset(UART_RX_Buffer, 0, sizeof(UART_RX_Buffer));
}


/**
  * @brief  IO enter dlps check function.
  * @param  No parameter.
  * @return void
  */
POWER_CheckResult io_uart_dlps_check(void)
{
    return IO_UART_DLPS_Enter_Allowed;
}

/**
  * @brief  IO enter dlps call back function.
  * @param  No parameter.
  * @return void
  */
void io_uart_dlps_enter(void)
{
    /* Notes: DBG_DIRECT is only used in debug demo, do not use in app project.*/
    DBG_DIRECT("DLPS ENTER");
    /* Switch pad to Software mode */
    Pad_ControlSelectValue(UART_TX_PIN, PAD_SW_MODE);
    Pad_ControlSelectValue(UART_RX_PIN, PAD_SW_MODE);

    System_WakeUpPinEnable(UART_RX_PIN, PAD_WAKEUP_POL_LOW, PAD_WAKEUP_DEB_DISABLE);
}

/**
  * @brief  IO exit dlps call back function.
  * @param  No parameter.
  * @return void
  */
void io_uart_dlps_exit(void)
{
    /* Notes: DBG_DIRECT is only used in debug demo, do not use in app project.*/
    DBG_DIRECT("DLPS EXIT");
    Pad_ControlSelectValue(UART_TX_PIN, PAD_PINMUX_MODE);
    Pad_ControlSelectValue(UART_RX_PIN, PAD_PINMUX_MODE);
}

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param   No parameter.
  * @return  void
  */
void board_uart_init(void)
{
    Pad_Config(UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);

    Pinmux_Config(UART_TX_PIN, UART0_TX);
    Pinmux_Config(UART_RX_PIN, UART0_RX);
}

/**
  * @brief  Initialize RTC peripheral.
  * @param   No parameter.
  * @return  void
  */
void driver_uart_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);

    /* uart init */
    UART_InitTypeDef UART_InitStruct;
    UART_StructInit(&UART_InitStruct);

    UART_Init(UART0, &UART_InitStruct);

    //enable rx interrupt and line status interrupt
    UART_INTConfig(UART0, UART_INT_RD_AVA, ENABLE);
    UART_INTConfig(UART0, UART_INT_RX_IDLE, ENABLE);

    /*  Enable UART IRQ  */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = UART0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);
}

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
  * @brief  UART0 detection interrupt handle function.
  * @param  None.
  * @return None.
  */

void UART0_Handler()
{
    DBG_DIRECT("UART_Handler");
    uint16_t rx_len = 0;

    /* Read interrupt id */
    uint32_t int_status = UART_GetIID(UART0);

    /* Disable interrupt */
    UART_INTConfig(UART0, UART_INT_RD_AVA | UART_INT_RX_LINE_STS, DISABLE);

    if (UART_GetFlagStatus(UART0, UART_FLAG_RX_IDLE) == SET)
    {
        /* Clear flag */
        UART_INTConfig(UART0, UART_INT_RX_IDLE, DISABLE);
        DBG_DIRECT("UART IDLE");

        /* Send msg to app task */
        uart_senddata_continuous(UART0, UART_RX_Buffer, UART_RX_Count);
        global_data_uart_init();
        while (UART_GetFlagStatus(UART0, UART_FLAG_TX_FIFO_EMPTY) == 0)
        {
            IO_UART_DLPS_Enter_Allowed = POWER_CHECK_PASS;
        }

        UART_ClearRxFIFO(UART0);
        UART_INTConfig(UART0, UART_INT_RX_IDLE, ENABLE);
    }

    switch (int_status & 0x0E)
    {
    /* Rx time out(0x0C). */
    case UART_INT_ID_RX_DATA_TIMEOUT:
        DBG_DIRECT("status:UART_INT_ID_RX_DATA_TIMEOUT");
        rx_len = UART_GetRxFIFODataLen(UART0);
        UART_ReceiveData(UART0, &UART_RX_Buffer[UART_RX_Count], rx_len);
        UART_RX_Count += rx_len;
        break;

    /* Receive line status interrupt(0x06). */
    case UART_INT_ID_LINE_STATUS:
        DBG_DIRECT("status:UART_INT_ID_LINE_STATUS");
        break;

    /* Rx data valiable(0x04). */
    case UART_INT_ID_RX_LEVEL_REACH:
        DBG_DIRECT("status:UART_INT_ID_RX_LEVEL_REACH");
        rx_len = UART_GetRxFIFODataLen(UART0);
        UART_ReceiveData(UART0, &UART_RX_Buffer[UART_RX_Count], rx_len);
        UART_RX_Count += rx_len;
        break;

    /* Tx fifo empty(0x02), not enable. */
    case UART_INT_ID_TX_EMPTY:
        DBG_DIRECT("status:UART_INT_ID_TX_EMPTY");
        /* Do nothing */
        break;
    default:
        break;
    }

    /* enable interrupt again */
    UART_INTConfig(UART0, UART_INT_RD_AVA, ENABLE);
//  IO_UART_DLPS_Enter_Allowed = PM_CHECK_PASS;
}
