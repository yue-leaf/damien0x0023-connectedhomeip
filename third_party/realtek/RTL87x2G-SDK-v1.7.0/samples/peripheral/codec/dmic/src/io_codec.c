/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes -----------------------------------------------------------------*/
#include <stdlib.h>
#include "io_codec.h"
#include "trace.h"

/* Defines ------------------------------------------------------------------*/
/* Pin define of DMIC peripheral */
#define DMIC_MSBC_CLK_PIN               P2_2
#define DMIC_MSBC_DAT_PIN               P2_3

/* Pin define of Data UART peripheral */
#define UART_TX_PIN                     P3_2
#define UART_RX_PIN                     P3_3

/* Audio operation ---------------------------------------------*/
#define AUDIO_FRAME_SIZE                4000

#define CODEC_BCLK_PIN             P4_4
#define CODEC_LRCLK_PIN            P4_5
#define CODEC_TX_PIN               P4_6
#define CODEC_RX_PIN               P4_7

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return Void
  */
void board_codec_init(void)
{
    Pad_Config(DMIC_MSBC_CLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(DMIC_MSBC_DAT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);

    Pinmux_Config(DMIC_MSBC_CLK_PIN, DMIC1_CLK);
    Pinmux_Config(DMIC_MSBC_DAT_PIN, DMIC1_DAT);

    Pad_Config(CODEC_BCLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(CODEC_LRCLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(CODEC_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(CODEC_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);

    Pinmux_Config(CODEC_BCLK_PIN, BCLK_SPORT0);
    Pinmux_Config(CODEC_LRCLK_PIN, LRC_RX_SPORT0);
    Pinmux_Config(CODEC_TX_PIN,    SDI_CODEC_SLAVE);
    Pinmux_Config(CODEC_RX_PIN,    SDO_CODEC_SLAVE);
}

/**
  * @brief  Initialization of uart pinmux settings and uart pad settings.
  * @param  No parameter.
  * @return Void
  */
void board_uart_init(void)
{
    Pad_Config(UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pinmux_Deinit(UART_TX_PIN);
    Pinmux_Deinit(UART_RX_PIN);
    Pinmux_Config(UART_TX_PIN, UART5_TX);
    Pinmux_Config(UART_RX_PIN, UART5_RX);
}

/**
  * @brief  Initialize codec peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_codec_init(void)
{

    RCC_PeriphClockCmd(APBPeriph_CODEC, APBPeriph_CODEC_CLOCK, ENABLE);

    CODEC_InitTypeDef CODEC_InitStruct;

    CODEC_StructInit(&CODEC_InitStruct);
    CODEC_InitStruct.CODEC_Ch0MicType       = CODEC_CH_DMIC;
    CODEC_InitStruct.CODEC_DmicClock        = DMIC_Clock_2500KHz;
    CODEC_InitStruct.CODEC_Ch0DmicDataLatch = DMIC_Ch_Rising_Latch;
    CODEC_InitStruct.CODEC_SampleRate0      = SAMPLE_RATE_16KHz;
    CODEC_InitStruct.CODEC_I2SFormat        = CODEC_I2S_DataFormat_I2S;
    CODEC_InitStruct.CODEC_I2SChannelLen    = I2S_CHANNELLEN_32;
    CODEC_InitStruct.CODEC_I2SRxDataWidth     = CODEC_I2S_Rx_DataWidth_16Bits;

    CODEC_InitStruct.CODEC_I2SChSequence    = CODEC_I2S_CH_L_R;
    CODEC_Init(CODEC, &CODEC_InitStruct);
    DBG_DIRECT("Use dmic test!");
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
    I2S_InitStruct.I2S_ClockSource        = I2S_CLK_40M;
    I2S_InitStruct.I2S_RxBClockMi         = 0x271;/* <!BCLK = 16K */
    I2S_InitStruct.I2S_RxBClockNi         = 0x10;
    I2S_InitStruct.I2S_DeviceMode         = I2S_DeviceMode_Master;
    I2S_InitStruct.I2S_RxChannelType      = I2S_Channel_Mono;
    I2S_InitStruct.I2S_RxDataFormat       = I2S_Mode;
    I2S_InitStruct.I2S_RxChannelWidth     = I2S_Width_32Bits;
    I2S_InitStruct.I2S_RxDataWidth        = I2S_Width_16Bits;
    I2S_Init(I2S0, &I2S_InitStruct);
}

/**
  * @brief  Initialize uart peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_uart_init(void)
{
    /* Enable clock */
    RCC_PeriphClockCmd(APBPeriph_UART5, APBPeriph_UART5_CLOCK, ENABLE);

    /* UART init */
    UART_InitTypeDef UART_InitStruct;

    UART_StructInit(&UART_InitStruct);
    /* change to 3M baudrate */
    UART_InitStruct.UART_Div             = 1;
    UART_InitStruct.UART_Ovsr            = 8;
    UART_InitStruct.UART_OvsrAdj        = 0x492;

    UART_InitStruct.UART_TxDmaEn         = ENABLE;
    UART_InitStruct.UART_DmaEn           = UART_DMA_ENABLE;
    UART_Init(UART5, &UART_InitStruct);
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

    /* Initialize GDMA peripheral */
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = GDMA_Channel_DMIC_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_PeripheralToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize          = AUDIO_FRAME_SIZE / 4;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_16;
    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)(&(I2S0->I2S_RX_FIFO_RD_ADDR));
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)(&(UART5->UART_RBR_THR));
    GDMA_InitStruct.GDMA_SourceHandshake     = GDMA_Handshake_I2S0_TDM0_RX;
    GDMA_InitStruct.GDMA_DestHandshake       = GDMA_Handshake_UART5_TX;
    GDMA_InitStruct.GDMA_ChannelPriority = 2;//channel prority between 0 to 5

    GDMA_Init(GDMA_Channel_DMIC, &GDMA_InitStruct);
    GDMA_INTConfig(GDMA_Channel_DMIC_NUM, GDMA_INT_Transfer, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GDMA0_Channel0_IRQn;
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
    board_codec_init();
    board_uart_init();

    driver_i2s_init();
    driver_uart_init();
    driver_gdma_init();
    driver_codec_init();

    I2S_Cmd(I2S0, I2S_MODE_TX | I2S_MODE_RX, ENABLE);
    GDMA_Cmd(GDMA_Channel_DMIC_NUM, ENABLE);
}


/**
  * @brief  GDMA interrupt handler function.
  * @param  No parameter.
  * @return Void
*/
void GDMA_Channel_DMIC_Handler(void)
{
    GDMA_SetSourceAddress(GDMA_Channel_DMIC, (uint32_t)(&(I2S0->I2S_RX_FIFO_RD_ADDR)));
    GDMA_SetDestinationAddress(GDMA_Channel_DMIC, (uint32_t)(&(UART5->UART_RBR_THR)));

    GDMA_SetBufferSize(GDMA_Channel_DMIC, AUDIO_FRAME_SIZE / 4);

    GDMA_ClearINTPendingBit(GDMA_Channel_DMIC_NUM, GDMA_INT_Transfer);
    GDMA_Cmd(GDMA_Channel_DMIC_NUM, ENABLE);
}



