/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __IO_UART_H
#define __IO_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl_nvic.h"
#include "rtl_pinmux.h"
#include "rtl_rcc.h"
#include "rtl_uart.h"

#include "board.h"

#include "app_msg.h"

#include "pm.h"

#define UART   UART0
/* UART pin define*/
#define UART_TX_PIN                P3_0
#define UART_RX_PIN                P3_1


/* Globals ------------------------------------------------------------------*/
extern POWER_CheckResult IO_UART_DLPS_Enter_Allowed;

void global_data_uart_init(void);
void board_uart_init(void);
void driver_uart_init(void);
void io_uart_dlps_enter(void);
void io_uart_dlps_exit(void);
POWER_CheckResult io_uart_dlps_check(void);
//void io_handle_uart_msg(T_IO_MSG *io_uart_msg);
void uart_senddata_continuous(UART_TypeDef *UARTx, const uint8_t *pSend_Buf, uint16_t vCount);


#ifdef __cplusplus
}
#endif

#endif

