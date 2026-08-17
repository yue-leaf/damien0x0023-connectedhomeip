/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes -----------------------------------------------------------------*/
#include <stdlib.h>
#include "rtl876x.h"
#include "io_codec.h"
#include "trace.h"
#include <math.h>

/* Defines ------------------------------------------------------------------*/
/* Pin define of DMIC peripheral */
#define PDM_CLK_PIN          P4_3
#define PDM_DAT_PIN          P4_2

/* Pin define of CODEC */
#define CODEC_BCLK_PIN             P4_4
#define CODEC_LRCLK_PIN            P4_5
#define CODEC_TX_PIN               P4_6
#define CODEC_RX_PIN               P4_7

uint32_t *codec_drv_cos_table[CODEC_DRV_COS_TABLE_SAMPLE];

/**
  * @brief  codec drv cos table.
  * @param  No parameter.
  * @return Void
  */
void preload_cos_table()
{
    double result;
    double radian_param;
//    double amplitude = (double)amp_ratio / CODEC_DRV_AMP_DENOM;
    int32_t result_hex;

    for (int i = 0; i < CODEC_DRV_COS_TABLE_SAMPLE; i++)
    {
        result = 0;
        radian_param = (2.0 * PI * FREQ) / CODEC_DRV_SAMPLE_RATE;
        result += cos(radian_param * i);
//        result *= amplitude;
        result_hex = (int32_t)(result * 8388608.0);

        // quantize to 24 bit
        if (result_hex > 8388607)
        {
            result_hex = 8388607;
        }
        else if (result_hex < (-8388608))
        {
            result_hex = (-8388608);
        }
        result_hex &= 0x7FFFFF;

        codec_drv_cos_table[i] = (uint32_t *)(result_hex * 256);
    }
    DBG_DIRECT("CODEC_DRV_COS_TABLE_SAMPLE= %d", CODEC_DRV_COS_TABLE_SAMPLE);
}

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return Void
  */
void board_codec_init(void)
{
    Pad_Config(PDM_CLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(PDM_DAT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);

    Pinmux_Config(PDM_CLK_PIN, PDM_CLK);
    Pinmux_Config(PDM_DAT_PIN, PDM_DATA);

}

void board_i2s_init()
{
    Pad_Config(CODEC_BCLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(CODEC_LRCLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(CODEC_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(CODEC_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);

    Pinmux_Config(CODEC_BCLK_PIN, BCLK_SPORT0);
    Pinmux_Config(CODEC_LRCLK_PIN, LRC_SPORT0);
    Pinmux_Config(CODEC_TX_PIN,    DACDAT_SPORT0);

}

/**
  * @brief  Initialize codec peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_codec_init(void)
{

    RCC_PeriphClockCmd(APBPeriph_CODEC, APBPeriph_CODEC_CLOCK, ENABLE);

    CODEC_AnalogCircuitInit();

    CODEC_InitTypeDef CODEC_InitStruct;

    CODEC_StructInit(&CODEC_InitStruct);
    /* PDM initialization parameters section */
    CODEC_InitStruct.CODEC_DaMute = CODEC_UNMUTE;
    CODEC_InitStruct.CODEC_DaGain = 0xAF;
    CODEC_InitStruct.CODEC_DaC_Dither = DAC_DA_DITHER_DISABLE;

    CODEC_InitStruct.CODEC_I2SFormat = CODEC_I2S_DataFormat_I2S;
    CODEC_InitStruct.CODEC_I2SChannelLen = I2S_CHANNELLEN_32;
    CODEC_InitStruct.CODEC_I2STxDataWidth = CODEC_I2S_Tx_DataWidth_24Bits;
    CODEC_InitStruct.CODEC_I2SChSequence = CODEC_I2S_CH_L_R;

    CODEC_Init(CODEC, &CODEC_InitStruct);
    DBG_DIRECT("Use playback test!");
}

/**
  * @brief  Initialize i2s peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_i2s_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_I2S0, APBPeriph_I2S0_CLOCK, ENABLE);

    I2S_InitTypeDef I2S_InitStruct;
    I2S_StructInit(&I2S_InitStruct);
    I2S_InitStruct.I2S_RxBClockMi         = 0x271;/* <!BCLK = 48K */
    I2S_InitStruct.I2S_RxBClockNi         = 0x10;
    I2S_InitStruct.I2S_DeviceMode       = I2S_DeviceMode_Master;
    I2S_InitStruct.I2S_TxChannelType    = I2S_Channel_stereo;
    I2S_InitStruct.I2S_RxChannelType    = I2S_Channel_stereo;
    I2S_InitStruct.I2S_TxChSequence     = I2S_CH_L_R;
    I2S_InitStruct.I2S_RxChSequence     = I2S_CH_L_R;
    I2S_InitStruct.I2S_TxDataFormat     = I2S_Mode;
    I2S_InitStruct.I2S_RxDataFormat     = I2S_Mode;
    I2S_InitStruct.I2S_TxBitSequence    = I2S_MSB_First;
    I2S_InitStruct.I2S_RxBitSequence    = I2S_MSB_First;
    I2S_InitStruct.I2S_TxDataWidth      = I2S_Width_24Bits;
    I2S_InitStruct.I2S_RxDataWidth      = I2S_Width_24Bits;
    I2S_InitStruct.I2S_TxChannelWidth   = I2S_Width_32Bits;
    I2S_InitStruct.I2S_RxChannelWidth   = I2S_Width_32Bits;
    I2S_InitStruct.I2S_TxWaterlevel     = 1;
    I2S_InitStruct.I2S_RxWaterlevel     = 1;
    I2S_InitStruct.I2S_TxFifoUsed       = I2S_FIFO_USE_0_REG_0;
    I2S_Init(I2S0, &I2S_InitStruct);

}

/* Basic verify function */
void i2s_drv_dump_setting(void)
{
    uint16_t i;
    uint32_t base_addr = 0x40020000UL;
    DBG_DIRECT("dump reg: 0x%08X", base_addr);

    for (i = 0; i < 0x60; i += 0x10)
    {
        DBG_DIRECT("%08X: %08X %08X %08X %08X",
                   base_addr + i,
                   HAL_READ32(base_addr, i),
                   HAL_READ32(base_addr, i + 0x04),
                   HAL_READ32(base_addr, i + 0x08),
                   HAL_READ32(base_addr, i + 0x0C));
    }
    DBG_DIRECT("0X40002700 = 0x%x", (*((volatile uint32_t *)0x40002700)));
    DBG_DIRECT("0X40002708 = 0x%x", (*((volatile uint32_t *)0x40002708)));
}

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_gdma_init(void)
{
    /* Enable GDMA clock */
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);
    /*--------------GDMA init-----------------------------*/
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum      = CODEC_GDMA_Channel_NUM;
    GDMA_InitStruct.GDMA_DIR             = GDMA_DIR_MemoryToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize      = CODEC_DRV_COS_TABLE_SAMPLE;//determine total transfer size
    GDMA_InitStruct.GDMA_SourceInc       = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc  = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceDataSize  = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_SourceMsize      = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_DestinationMsize = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_SourceAddr      = (uint32_t)codec_drv_cos_table;
    GDMA_InitStruct.GDMA_DestinationAddr = (uint32_t)(&(I2S0->I2S_TX_FIFO_WR_ADDR));
    GDMA_InitStruct.GDMA_DestHandshake   = GDMA_Handshake_I2S0_TDM0_TX;
    GDMA_InitStruct.GDMA_ChannelPriority = 2;//channel prority between 0 to 5
    GDMA_InitStruct.GDMA_Secure_En = ENABLE;
    GDMA_Init(CODEC_GDMA_Channel, &GDMA_InitStruct);

    GDMA_INTConfig(CODEC_GDMA_Channel_NUM, GDMA_INT_Transfer, ENABLE);

    /*-----------------GDMA IRQ init-------------------*/
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = CODEC_GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Demo code of codec dmic.
  * @param  No parameter.
  * @return Void
*/
void codec_demo(void)
{
    RCC_PeriphClockCmd(APBPeriph_I2S0, APBPeriph_I2S0_CLOCK, ENABLE);
    preload_cos_table();
    board_codec_init();
    driver_codec_init();
    board_i2s_init();
    driver_i2s_init();
    driver_gdma_init();

    I2S_Cmd(I2S0, I2S_MODE_TX, ENABLE);
    GDMA_Cmd(CODEC_GDMA_Channel_NUM, ENABLE);

    i2s_drv_dump_setting();
}


/**
  * @brief  GDMA interrupt handler function.
  * @param  No parameter.
  * @return Void
*/
void CODEC_GDMA_Channel_Handler(void)
{
    GDMA_SetSourceAddress(CODEC_GDMA_Channel, (uint32_t)codec_drv_cos_table);
    GDMA_SetDestinationAddress(CODEC_GDMA_Channel, (uint32_t)(&(I2S0->I2S_TX_FIFO_WR_ADDR)));

    GDMA_SetBufferSize(CODEC_GDMA_Channel, CODEC_DRV_COS_TABLE_SAMPLE);

    GDMA_ClearINTPendingBit(CODEC_GDMA_Channel_NUM, GDMA_INT_Transfer);

    GDMA_Cmd(CODEC_GDMA_Channel_NUM, ENABLE);
}



