/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include "io_keyscan.h"


/* Globals ------------------------------------------------------------------*/
/* Timer handle */
void *KeyScan_Timer_Handle = NULL;

KeyScan_Data_TypeDef Current_Key_Data;
bool Key_Pressed_Flag = false;


/**
  * @brief  Initialize keyscan global data.
  * @param  No parameter.
  * @return void
  */
void global_data_keyscan_init(void)
{
    /* Data struct init */
    memset(&Current_Key_Data, 0, sizeof(KeyScan_Data_TypeDef));
//    memset(&Pre_Key_Data, 0, sizeof(KeyScan_Data_TypeDef));
}

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return Void
  */
void board_keyboard_init(void)
{
    /* Keypad pad config */
    Pad_Config(KEYBOARD_ROW_0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(KEYBOARD_ROW_1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(KEYBOARD_COLUMN_0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(KEYBOARD_COLUMN_1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);

    /* keypad pinmux config */
    Pinmux_Config(KEYBOARD_ROW_0, KEY_ROW_0);
    Pinmux_Config(KEYBOARD_ROW_1, KEY_ROW_1);
    Pinmux_Config(KEYBOARD_COLUMN_0, KEY_COL_0);
    Pinmux_Config(KEYBOARD_COLUMN_1, KEY_COL_1);
}

/**
  * @brief  Initialize keyboard peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_keyboard_init(FunctionalState KEYSCAN_DebounceEn)
{
    /* Turn on keyscan clock */
    RCC_PeriphClockCmd(APBPeriph_KEYSCAN, APBPeriph_KEYSCAN_CLOCK, ENABLE);

    KEYSCAN_InitTypeDef KEYSCAN_InitStruct;
    KeyScan_StructInit(&KEYSCAN_InitStruct);

    KEYSCAN_InitStruct.rowSize  = KEYBOARD_ROW_SIZE;
    KEYSCAN_InitStruct.colSize  = KEYBOARD_COLUMN_SIZE;
    KEYSCAN_InitStruct.scanmode = KeyScan_Manual_Scan_Mode;
    KEYSCAN_InitStruct.debounceEn = ENABLE;

    KeyScan_Init(KEYSCAN, &KEYSCAN_InitStruct);

    KeyScan_INTConfig(KEYSCAN, KEYSCAN_INT_SCAN_END, ENABLE);
    KeyScan_ClearINTPendingBit(KEYSCAN, KEYSCAN_INT_SCAN_END);
    KeyScan_INTMask(KEYSCAN, KEYSCAN_INT_SCAN_END, DISABLE);  /* Unmask keyscan interrupt */
    KeyScan_Cmd(KEYSCAN, ENABLE);

    /* Keyscan IRQ */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = KEYSCAN_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);
}

void timer_keyscan_callback(void *p_xTimer)
{
    if (true == Key_Pressed_Flag)
    {
//        APP_PRINT_INFO0("[io_keyscan] timer_keyscan_callback: start release timer");
        Key_Pressed_Flag = false;
        driver_keyboard_init(ENABLE);

        /* Start timer to check key status */
        os_timer_restart(&p_xTimer, KEYSCAN_SW_RELEASE_TIMEOUT);
    }
    else
    {
        /* Keyscan release event detected */
        DBG_DIRECT("[io_keyscan] io_keyscan_handle_keys: All keys release.");

        global_data_keyscan_init();
        driver_keyboard_init(ENABLE);
    }
}

void timer_keyscan_init(void)
{
    DBG_DIRECT("[io_keyscan] timer_keyscan_init: keyscan timer init");
    if (false == os_timer_create(&KeyScan_Timer_Handle, "keyscan_timer",  1, \
                                 KEYSCAN_SW_INTERVAL, false, timer_keyscan_callback))
    {
        DBG_DIRECT("[io_keyscan] timer_keyscan_init: timer creat failed!");
    }
}

/**
  * @brief  Keyscan interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void KEYSCAN_Handler(void)
{
    uint32_t fifo_length;

    if (KeyScan_GetFlagState(KEYSCAN, KEYSCAN_INT_FLAG_SCAN_END) == SET)
    {
        /* Read current keyscan interrupt status and mask interrupt */
        KeyScan_INTMask(KEYSCAN, KEYSCAN_INT_SCAN_END, ENABLE);
        memset(&Current_Key_Data, 0, sizeof(KeyScan_Data_TypeDef));

        /* KeyScan fifo not empty */
        if (KeyScan_GetFlagState(KEYSCAN, KEYSCAN_FLAG_EMPTY) != SET)
        {
            fifo_length = (uint32_t)KeyScan_GetFifoDataNum(KEYSCAN);
            KeyScan_Read(KEYSCAN, (uint16_t *)&Current_Key_Data.key[0], fifo_length);
            Current_Key_Data.length = fifo_length;
            Key_Pressed_Flag = true;

            /* Start sw timer to check press status */
            if (!os_timer_restart(&KeyScan_Timer_Handle, KEYSCAN_SW_INTERVAL))
            {
                APP_PRINT_ERROR0("[io_keyscan] Keyscan_Handler: Restart keyscan_timer failed!");
                /* Set flag to default status and reinit keyscan module with debounce enabled */
                global_data_keyscan_init();
                driver_keyboard_init(ENABLE);
                return;
            }

            /* Single key press */
            if (fifo_length == 1)
            {
                DBG_DIRECT("[io_keyscan] io_keyscan_handle_keys: Single key press. key: (%d, %d)",
                           Current_Key_Data.key[0].row, Current_Key_Data.key[0].column);
            }

            /* two keys press */
            if (fifo_length == 2)
            {
                DBG_DIRECT("[io_keyscan] io_keyscan_handle_keys: Two key press. key0: (%d, %d), key1: (%d, %d)",
                           Current_Key_Data.key[0].row, Current_Key_Data.key[0].column, Current_Key_Data.key[1].row,
                           Current_Key_Data.key[1].column);
            }
        }
        KeyScan_ClearINTPendingBit(KEYSCAN, KEYSCAN_INT_SCAN_END);
        KeyScan_INTMask(KEYSCAN, KEYSCAN_INT_SCAN_END, DISABLE);
    }
}


