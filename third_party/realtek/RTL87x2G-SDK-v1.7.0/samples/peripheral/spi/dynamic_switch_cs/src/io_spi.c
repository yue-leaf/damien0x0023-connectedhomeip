/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "rtl_rcc.h"
#include "rtl_pinmux.h"
#include "trace.h"
#include <string.h>

#include "rtl_spi.h"
#include "io_spi.h"

/* Globals ------------------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_spi_init(void)
{
    Pad_Config(SPI_MASTER_SCK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(SPI_MASTER_MOSI_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(SPI_MASTER_MISO_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(SPI_MASTER_CS0_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(SPI_MASTER_CS1_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(SPI_MASTER_CS2_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    Pinmux_Deinit(SPI_MASTER_SCK_PIN);
    Pinmux_Deinit(SPI_MASTER_MOSI_PIN);
    Pinmux_Deinit(SPI_MASTER_MISO_PIN);
    Pinmux_Deinit(SPI_MASTER_CS0_PIN);
    Pinmux_Deinit(SPI_MASTER_CS1_PIN);
    Pinmux_Deinit(SPI_MASTER_CS2_PIN);

    Pinmux_Config(SPI_MASTER_SCK_PIN, SPI1_CLK_MASTER);
    Pinmux_Config(SPI_MASTER_MOSI_PIN, SPI1_MO_MASTER);
    Pinmux_Config(SPI_MASTER_MISO_PIN, SPI1_MI_MASTER);
    Pinmux_Config(SPI_MASTER_CS0_PIN, SPI1_CSN_0_MASTER);
    Pinmux_Config(SPI_MASTER_CS1_PIN, SPI1_CSN_1_MASTER);
    Pinmux_Config(SPI_MASTER_CS2_PIN, SPI1_CSN_2_MASTER);
}


/**
  * @brief  Initialize spi master peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_spi_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_SPI1, APBPeriph_SPI1_CLOCK, ENABLE);

    SPI_InitTypeDef  SPI_InitStruct;
    SPI_StructInit(&SPI_InitStruct);

    SPI_InitStruct.SPI_Direction   = SPI_Direction_FullDuplex;
    SPI_InitStruct.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStruct.SPI_CPHA        = SPI_CPHA_2Edge;
    SPI_InitStruct.SPI_BaudRatePrescaler  = 100;
    SPI_InitStruct.SPI_RxThresholdLevel  = 0;
    /* cause SPI_INT_RXF interrupt if data length in receive FIFO  >= SPI_RxThresholdLevel + 1*/
    SPI_InitStruct.SPI_FrameFormat = SPI_Frame_Motorola;

    SPI_Init(SPI1, &SPI_InitStruct);
    SPI_Cmd(SPI1, ENABLE);

}


/**
  * @brief  Demo code of operation about spi.
  * @param  No parameter.
  * @return void
  */
void spi_demo(void)
{
    DBG_DIRECT("spi demo");
    uint8_t write_buf[3] = {0x01, 0x02, 0x03};

    /* Configure pad and pinmux firstly! */
    board_spi_init();

    /* Initialize spi peripheral */
    driver_spi_init();

    SPI_SendBuffer(SPI1, write_buf, 3);

    /* wait Tx FIFO empty */
    while (SPI_GetFlagState(SPI1, SPI_FLAG_TFE) == RESET);
    while (SPI_GetFlagState(SPI1, SPI_FLAG_BUSY));

    /* switch to CS1 signal */
    SPI_SetCSNumber(SPI1, 1);

    SPI_SendBuffer(SPI1, write_buf, 3);
    /* wait Tx FIFO empty */
    while (SPI_GetFlagState(SPI1, SPI_FLAG_TFE) == RESET);
    while (SPI_GetFlagState(SPI1, SPI_FLAG_BUSY));

    /* switch to CS2 signal */
    SPI_SetCSNumber(SPI1, 2);

    SPI_SendBuffer(SPI1, write_buf, 3);
    /* wait Tx FIFO empty */
    while (SPI_GetFlagState(SPI1, SPI_FLAG_TFE) == RESET);
    while (SPI_GetFlagState(SPI1, SPI_FLAG_BUSY));
}



