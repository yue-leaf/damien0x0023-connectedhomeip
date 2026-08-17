/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "rtl_rcc.h"
#include "rtl_pinmux.h"
#include "rtl_nvic.h"
#include "trace.h"
#include <string.h>

#include "rtl_spi.h"
#include "io_spi.h"

/* Globals ------------------------------------------------------------------*/
uint8_t master_tx_buf[200];
uint8_t master_rx_buf[200];
uint8_t slave_tx_buf[200];
uint8_t slave_rx_buf[200];

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_spi_master_init(void)
{
    Pad_Config(SPI_MASTER_SCK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(SPI_MASTER_MOSI_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(SPI_MASTER_MISO_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(SPI_MASTER_CS_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    Pinmux_Deinit(SPI_MASTER_SCK_PIN);
    Pinmux_Deinit(SPI_MASTER_MOSI_PIN);
    Pinmux_Deinit(SPI_MASTER_MISO_PIN);
    Pinmux_Deinit(SPI_MASTER_CS_PIN);

    Pinmux_Config(SPI_MASTER_SCK_PIN, SPI1_CLK_MASTER);
    Pinmux_Config(SPI_MASTER_MOSI_PIN, SPI1_MO_MASTER);
    Pinmux_Config(SPI_MASTER_MISO_PIN, SPI1_MI_MASTER);
    Pinmux_Config(SPI_MASTER_CS_PIN, SPI1_CSN_0_MASTER);
}

/**
  * @brief  Initialization of spi slave pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_spi_slave_init(void)
{
    Pad_Config(SPI_SLAVE_SCK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(SPI_SLAVE_MOSI_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(SPI_SLAVE_MISO_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(SPI_SLAVE_CS_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    /* Must deinit P4_0~P4_3 before use SPI SLAVE!!! */
    Pinmux_Deinit(P4_0);
    Pinmux_Deinit(P4_1);
    Pinmux_Deinit(P4_2);
    Pinmux_Deinit(P4_3);

    Pinmux_Deinit(SPI_SLAVE_SCK_PIN);
    Pinmux_Deinit(SPI_SLAVE_MOSI_PIN);
    Pinmux_Deinit(SPI_SLAVE_MISO_PIN);
    Pinmux_Deinit(SPI_SLAVE_CS_PIN);

    Pinmux_Config(SPI_SLAVE_SCK_PIN, SPI0_CLK_SLAVE);
    Pinmux_Config(SPI_SLAVE_MOSI_PIN, SPI0_SI_SLAVE);
    Pinmux_Config(SPI_SLAVE_MISO_PIN, SPI0_SO_SLAVE);
    Pinmux_Config(SPI_SLAVE_CS_PIN, SPI0_CSN_0_SLAVE);
}

/**
  * @brief  Initialize spi master peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_spi_master_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_SPI_MASTER, APBPeriph_SPI_CLOCK_MASTER, ENABLE);

    SPI_InitTypeDef  SPI_InitStruct;
    SPI_StructInit(&SPI_InitStruct);

    SPI_InitStruct.SPI_Direction   = SPI_Direction_FullDuplex;
    SPI_InitStruct.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStruct.SPI_CPHA        = SPI_CPHA_2Edge;
    SPI_InitStruct.SPI_BaudRatePrescaler  = 100;
    SPI_InitStruct.SPI_RxThresholdLevel  = SEND_LENGTH - 1;
    /* cause SPI_INT_RXF interrupt if data length in receive FIFO  >= SPI_RxThresholdLevel + 1*/
    SPI_InitStruct.SPI_FrameFormat = SPI_Frame_Motorola;

    SPI_Init(SPI_MASTER, &SPI_InitStruct);
    SPI_Cmd(SPI_MASTER, ENABLE);

    SPI_INTConfig(SPI_MASTER, SPI_INT_RXF, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = SPI_IRQn_MASTER;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    for (uint32_t i = 0; i < SEND_LENGTH; i++)
    {
        master_tx_buf[i] = 0x1 + i;
    }
    SPI_SendBuffer(SPI_MASTER, master_tx_buf, SEND_LENGTH);
}

/**
  * @brief  Initialize spi slave peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_spi_slave_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_SPI_SLAVE, APBPeriph_SPI_CLOCK_SLAVE, ENABLE);

    SPI_InitTypeDef  SPI_InitStruct;
    SPI_StructInit(&SPI_InitStruct);

    SPI_InitStruct.SPI_Direction   = SPI_Direction_FullDuplex;
    SPI_InitStruct.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStruct.SPI_CPHA        = SPI_CPHA_2Edge;
    /* SPI_Direction_EEPROM mode read data lenth. */
    SPI_InitStruct.SPI_RxThresholdLevel  = SEND_LENGTH - 1;/* Flash id lenth = 3*/
    /* cause SPI_INT_RXF interrupt if data length in receive FIFO  >= SPI_RxThresholdLevel + 1*/
    SPI_InitStruct.SPI_FrameFormat = SPI_Frame_Motorola;

    SPI_Init(SPI_SLAVE, &SPI_InitStruct);
    SPI_Cmd(SPI_SLAVE, ENABLE);

    SPI_INTConfig(SPI_SLAVE, SPI_INT_RXF, ENABLE);

    /* Config SPI interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = SPI_IRQn_SLAVE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    for (uint32_t i = 0; i < SEND_LENGTH; i++)
    {
        slave_tx_buf[i] = 0x10 + i;
    }
    SPI_SendBuffer(SPI_SLAVE, slave_tx_buf, SEND_LENGTH);
}


/**
  * @brief  Demo code of operation about spi.
  * @param  No parameter.
  * @return void
  */
void spi_demo(void)
{
    DBG_DIRECT("spi slave demo");
    /* Configure pad and pinmux firstly! */
    board_spi_slave_init();
    board_spi_master_init();

    /* Initialize spi peripheral */
    driver_spi_slave_init();
    driver_spi_master_init();
}

void SPI_Handler_MASTER(void)
{
    DBG_DIRECT("Enter SPI MASTER Handler");
    if (SPI_GetINTStatus(SPI_MASTER, SPI_INT_RXF))
    {
        SPI_INTConfig(SPI_MASTER, SPI_INT_RXF, DISABLE);
        SPI_ClearINTPendingBit(SPI_MASTER, SPI_INT_RXF);
        uint32_t master_rx_len = SPI_GetRxFIFOLen(SPI_MASTER);
        DBG_DIRECT("SPI MASTER receieves %d data:", master_rx_len);
        for (uint32_t i = 0; i < master_rx_len; i++)
        {
            master_rx_buf[i] = SPI_ReceiveData(SPI_MASTER);
            DBG_DIRECT("master_rx_buf[%d] = 0x%x, slave_tx_buf[%d] = 0x%x", i, master_rx_buf[i], i,
                       slave_tx_buf[i]);
        }
        SPI_INTConfig(SPI_MASTER, SPI_INT_RXF, ENABLE);
    }
}

void SPI_Handler_SLAVE(void)
{
    DBG_DIRECT("Enter SPI SLAVE Handler");
    if (SPI_GetINTStatus(SPI_SLAVE, SPI_INT_RXF))
    {
        SPI_INTConfig(SPI_SLAVE, SPI_INT_RXF, DISABLE);
        SPI_ClearINTPendingBit(SPI_SLAVE, SPI_INT_RXF);
        uint32_t slave_rx_len = SPI_GetRxFIFOLen(SPI_SLAVE);
        DBG_DIRECT("SPI SLAVE receieves %d data:", slave_rx_len);
        for (uint32_t i = 0; i < slave_rx_len; i++)
        {
            slave_rx_buf[i] = SPI_ReceiveData(SPI_SLAVE);
            DBG_DIRECT("slave_rx_buf[%d] = 0x%x, master_tx_buf[%d] = 0x%x", i, slave_rx_buf[i], i,
                       master_tx_buf[i]);
        }
        SPI_INTConfig(SPI_SLAVE, SPI_INT_RXF, ENABLE);
    }

}


