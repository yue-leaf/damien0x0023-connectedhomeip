/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "rtl_i2s.h"
#include "rtl_pinmux.h"
#include "rtl_rcc.h"
#include "io_i2s.h"

/* Defines -------------------------------------------------------------------*/


/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return Void
  */
void board_i2s_init(void)
{
    Pad_Config(I2S_BCLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(I2S_LRCK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(I2S_DATA_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);

    Pinmux_Config(I2S_BCLK_PIN, I2S_BCLK_PINMUX);
    Pinmux_Config(I2S_LRCK_PIN, I2S_LRCK_PINMUX);
    Pinmux_Config(I2S_DATA_TX_PIN, I2S_DATA_TX_PINMUX);
}

/**
  * @brief  Initialize I2S peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_i2s_init(void)
{
    RCC_PeriphClockCmd(APB_I2S, APB_I2S_CLOCK, ENABLE);

    I2S_InitTypeDef I2S_InitStruct;

    I2S_StructInit(&I2S_InitStruct);
    I2S_InitStruct.I2S_ClockSource      = I2S_CLK_40M;
    /* BCLK = 40MHz*(ni/mi),  LRCK =  BCLK / (bclk_div_ratio+1) */
    I2S_InitStruct.I2S_TxBClockMi         = 0x271;    /* <!LRCK = 16K */
    I2S_InitStruct.I2S_TxBClockNi         = 0x10;     /* <!BCLK = 1024K */
    I2S_InitStruct.I2S_DeviceMode         = I2S_DeviceMode_Master;
    I2S_InitStruct.I2S_TxChannelType      = I2S_Channel_stereo;
    I2S_InitStruct.I2S_TxDataWidth        = I2S_Width_16Bits;
    I2S_InitStruct.I2S_TxDataFormat       = I2S_Mode;
    I2S_Init(I2S_NUM, &I2S_InitStruct);
    I2S_Cmd(I2S_NUM, I2S_MODE_TX, ENABLE);
}

/**
  * @brief  Demo code of sending data continuously.
  * @param  No parameter.
  * @return Void
  */
void i2s_senddata(void)
{
    uint32_t i = 0x12348800;

    while (1)
    {
        if (I2S_GetTxFIFOFreeLen(I2S_NUM))
        {
            /* 16bit format, lower half word send first! */
            I2S_SendData(I2S_NUM, i++);
        }
    }
}

/**
  * @brief  Demo code of I2S communication.
  * @param  No parameter.
  * @return Void
*/
void i2s_demo(void)
{
    board_i2s_init();
    driver_i2s_init();
    i2s_senddata();
}




