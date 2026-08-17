/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "rtl_spi3w.h"
#include "rtl_pinmux.h"
#include "rtl_rcc.h"

#include "trace.h"

/* Defines -------------------------------------------------------------------*/
#define SPI_3WIRE_CLK_PIN       P2_2
#define SPI_3WIRE_DATA_PIN      P2_3
#define SPI_3WIRE_CS_PIN        P2_4

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return Void
  */
void board_spi3wire_init(void)
{
    Pad_Config(SPI_3WIRE_CLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(SPI_3WIRE_DATA_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(SPI_3WIRE_CS_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    Pinmux_Config(SPI_3WIRE_CLK_PIN, SPI2W_CLK);
    Pinmux_Config(SPI_3WIRE_DATA_PIN, SPI2W_DATA);
    Pinmux_Config(SPI_3WIRE_CS_PIN, SPI2W_CS);
}

/**
  * @brief  Initialize 3wire-spi peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_spi3wire_init(void)
{
    /* Enable SPI3W clock */
    RCC_PeriphClockCmd(APBPeriph_SPI3W, APBPeriph_SPI3W_CLOCK, ENABLE);

    /* Initialize SPI3W */
    SPI3W_InitTypeDef SPI3W_InitStruct;
    SPI3W_StructInit(&SPI3W_InitStruct);

    SPI3W_InitStruct.SPI3W_SysClock       = 20000000;
    SPI3W_InitStruct.SPI3W_Speed          = 800000;
    SPI3W_InitStruct.SPI3W_Mode           = SPI3W_3WIRE_MODE;
    /** The delay time from the end of address phase to the start of read data phase.
      * delay time = (SPI3W_ReadDelay +1)/(2*SPI3W_Speed).
      * delay time = (0x03 + 1)/(2 * speed) = 2.5us
      */
    SPI3W_InitStruct.SPI3W_ReadDelay      = 0x3;
    SPI3W_InitStruct.SPI3W_OutputDelay    = SPI3W_OE_DELAY_NONE;
    SPI3W_InitStruct.SPI3W_ExtMode        = SPI3W_NORMAL_MODE;
    SPI3W_Init(&SPI3W_InitStruct);
}

/**
  * @brief  Read one byte through 3wire spi perpherial.
  * @param  address: address of register which need to read.
  * @return Value of register.
  */
uint8_t spi3wire_readbyte(uint8_t address)
{
    uint8_t reg_value = 0;
    uint32_t timeout = 0;

    /* Check spi busy or not */
    while (SPI3W_GetFlagStatus(SPI3W_FLAG_BUSY) == SET)
    {
        timeout++;
        if (timeout > 0x1ffff)
        {
            break;
        }
    }

    /* Clear Receive data length */
    SPI3W_ClearRxDataLen();

    SPI3W_StartRead(address, 1);

    timeout = 0;
    while (SPI3W_GetFlagStatus(SPI3W_FLAG_BUSY) == SET)
    {
        timeout++;
        if (timeout > 0x1ffff)
        {
            break;
        }
    }

    /* Get the length of received data */
    while (SPI3W_GetRxDataLen() == 0);
    /* Read data */
    SPI3W_ReadBuf(&reg_value, 1);

    return reg_value;
}

/**
  * @brief  Write one byte.
  * @param  address: address of register which need to write data.
  * @param  data: data which need to write.
  * @return TRUE: write success, FALSE: write failure.
  */
bool spi3wire_writebyte(uint8_t address, uint8_t data)
{
    uint32_t timeout = 0;

    /* Check 3wire spi busy or not */
    while (SPI3W_GetFlagStatus(SPI3W_FLAG_BUSY) == SET)
    {
        timeout++;
        if (timeout > 0x1ffff)
        {
            return false;
        }
    }
    /* Write data */
    SPI3W_StartWrite(address, data);

    timeout = 0;
    while (SPI3W_GetFlagStatus(SPI3W_FLAG_BUSY) == SET)
    {
        timeout++;
        if (timeout > 0x1ffff)
        {
            return false;
        }
    }
    return true;
}

/**
  * @brief  Reset mouse.
  * @param  None.
  * @return void.
  */
void mouse_reset(void)
{
    uint32_t delay = 0;

    /* Delay 0.28ms */
    for (delay = 0; delay < 0x1fff; delay++) {;}
}

/**
  * @brief  Read mouse product ID.
  * @param  p_id, --pointer to production id buffer,buffer length should more than two.
  * @return TRUE.
  */
bool mouse_getproductid(uint8_t *p_id)
{

    /* Read product ID number high byte */
    *p_id++ = spi3wire_readbyte(0x00);

    /* Read product ID number low byte */
    *p_id = spi3wire_readbyte(0x01);

    return true;
}

/**
  * @brief  Demo code of 3wire-spi communication.
  * @param  No parameter.
  * @return void
  */
void spi3wire_demo(void)
{
    uint8_t id[2] = {0, 0};

    board_spi3wire_init();
    driver_spi3wire_init();

    /** Send resync time. Resync signal time = 2*1/(2*SPI3W_Speed) = 1.25us
      * This parameter can be only be the following value: 0x0 to 0xf.
      */
    SPI3W_SetResyncTime(2);
    SPI3W_ResyncSignalCmd(ENABLE);
    while (SPI3W_GetFlagStatus(SPI3W_FLAG_RESYNC_BUSY) == SET);
    SPI3W_ResyncSignalCmd(DISABLE);

    /* Enable SPI3W to normal communication */
    SPI3W_Cmd(ENABLE);

    mouse_reset();
    mouse_getproductid(&id[0]);
    DBG_DIRECT("id[0] = 0x%x, id[1] = 0x%x", id[0], id[1]);
    if ((0x3e == id[0]) && (0x01 == (id[1] & 0xFF)))
    {
        /* Notes: DBG_DIRECT is only used in debug demo, do not use in app project.*/
        DBG_DIRECT("SPI3W test pass!");
    }
    else
    {
        /* Notes: DBG_DIRECT is only used in debug demo, do not use in app project.*/
        DBG_DIRECT("SPI3W test failed!");
    }
}




