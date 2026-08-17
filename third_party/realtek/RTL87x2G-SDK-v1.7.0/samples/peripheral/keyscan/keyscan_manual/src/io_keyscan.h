/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __IO_ADC_H
#define __IO_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl_keyscan.h"
#include "rtl_nvic.h"
#include "rtl_pinmux.h"
#include "rtl_rcc.h"
#include "string.h"

#include "board.h"
#include "trace.h"
#include "app_msg.h"
#include "os_timer.h"


/* Defines -------------------------------------------------------------------*/
#define KEYSCAN_SW_INTERVAL             (200)/* 200ms */
#define KEYSCAN_SW_RELEASE_TIMEOUT      (10)/* 10ms */

/* Pin define of keypad row and column */
#define KEYBOARD_ROW_SIZE               2
#define KEYBOARD_COLUMN_SIZE            2
#define KEYBOARD_ROW_0                  P2_3
#define KEYBOARD_ROW_1                  P2_4
#define KEYBOARD_COLUMN_0               P4_0
#define KEYBOARD_COLUMN_1               P4_1

/* Typedefs ------------------------------------------------------------------*/
typedef struct
{
    uint32_t length;
    struct
    {
        uint16_t column: 5;   /**< keyscan column buffer data   */
        uint16_t row: 4;      /**< keyscan raw buffer data      */
    } key[26];
} KeyScan_Data_TypeDef;


extern void *KeyScan_Timer_Handle;
extern KeyScan_Data_TypeDef Current_Key_Data;


void global_data_keyscan_init(void);
void board_keyboard_init(void);
void driver_keyboard_init(FunctionalState KEYSCAN_DebounceEn);
void timer_keyscan_init(void);

#ifdef __cplusplus
}
#endif

#endif

