/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include "io_i2s.h"
#include "trace.h"

uint32_t GDMA_Send_Buf[GDMA_TRANSFER_SIZE];
uint32_t GDMA_Recv_Buf[GDMA_TRANSFER_SIZE];
/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return Void
  */
void board_i2s_init(void)
{
    Pad_Config(I2S0_BCLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(I2S0_LRCK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(I2S0_DATA_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(I2S0_DATA_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(I2S1_BCLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(I2S1_LRCK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(I2S1_DATA_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(I2S1_DATA_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);

    Pinmux_Config(I2S0_BCLK_PIN, BCLK_SPORT0);
    Pinmux_Config(I2S0_LRCK_PIN,  LRC_SPORT0);
    Pinmux_Config(I2S0_DATA_TX_PIN,  DACDAT_SPORT0);
    Pinmux_Config(I2S0_DATA_RX_PIN,  ADCDAT_SPORT0);

    Pinmux_Config(I2S1_BCLK_PIN, BCLK_SPORT1);
    Pinmux_Config(I2S1_LRCK_PIN,  LRC_SPORT1);
    Pinmux_Config(I2S1_DATA_TX_PIN,  DACDAT_SPORT1);
    Pinmux_Config(I2S1_DATA_RX_PIN,  ADCDAT_SPORT1);
}

/**
  * @brief  Initialize I2S Tx peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_i2s0_TDM2_tx_init()
{
    RCC_PeriphClockCmd(APBPeriph_I2S0, APBPeriph_I2S0_CLOCK, ENABLE);

    I2S_InitTypeDef I2S_InitStruct;
    I2S_StructInit(&I2S_InitStruct);
    I2S_InitStruct.I2S_TxBClockMi         = 0x271;/* <!BCLK = 16K */
    I2S_InitStruct.I2S_TxBClockNi         = 0x10;
    I2S_InitStruct.I2S_DeviceMode       = I2S_DeviceMode_Master;
    I2S_InitStruct.I2S_TxChannelType    = I2S_Channel_stereo;
    I2S_InitStruct.I2S_RxChannelType    = I2S_Channel_stereo;
    I2S_InitStruct.I2S_TxChSequence     = I2S_CH_L_R;
    I2S_InitStruct.I2S_RxChSequence     = I2S_CH_L_R;
    I2S_InitStruct.I2S_TxDataFormat     = I2S_Mode;
    I2S_InitStruct.I2S_RxDataFormat     = I2S_Mode;
    I2S_InitStruct.I2S_TxBitSequence    = I2S_MSB_First;
    I2S_InitStruct.I2S_RxBitSequence    = I2S_MSB_First;
    I2S_InitStruct.I2S_TxDataWidth      = I2S_Width_16Bits;
    I2S_InitStruct.I2S_RxDataWidth      = I2S_Width_16Bits;
    I2S_InitStruct.I2S_TxChannelWidth   = I2S_Width_16Bits;
    I2S_InitStruct.I2S_RxChannelWidth   = I2S_Width_16Bits;

    I2S_InitStruct.I2S_TxWaterlevel     = 16;
    I2S_InitStruct.I2S_RxWaterlevel     = 16;
    I2S_InitStruct.I2S_TxFifoUsed       = I2S_FIFO_USE_0_REG_0;
    I2S_Init(I2S0, &I2S_InitStruct);

}

/**
  * @brief  Initialize I2S Rx peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_i2s1_TDM2_rx_init()
{
    RCC_PeriphClockCmd(APBPeriph_I2S1, APBPeriph_I2S1_CLOCK, ENABLE);

    I2S_InitTypeDef I2S_InitStruct;
    I2S_StructInit(&I2S_InitStruct);
    I2S_InitStruct.I2S_RxBClockMi         = 0x271;/* <!BCLK = 16K */
    I2S_InitStruct.I2S_RxBClockNi         = 0x10;
    I2S_InitStruct.I2S_DeviceMode       = I2S_DeviceMode_Slave;
    I2S_InitStruct.I2S_TxChannelType    = I2S_Channel_stereo;
    I2S_InitStruct.I2S_RxChannelType    = I2S_Channel_stereo;
    I2S_InitStruct.I2S_TxChSequence     = I2S_CH_L_R;
    I2S_InitStruct.I2S_RxChSequence     = I2S_CH_L_R;
    I2S_InitStruct.I2S_TxDataFormat     = I2S_Mode;
    I2S_InitStruct.I2S_RxDataFormat     = I2S_Mode;
    I2S_InitStruct.I2S_TxBitSequence    = I2S_MSB_First;
    I2S_InitStruct.I2S_RxBitSequence    = I2S_MSB_First;
    I2S_InitStruct.I2S_TxDataWidth      = I2S_Width_16Bits;
    I2S_InitStruct.I2S_RxDataWidth      = I2S_Width_16Bits;
    I2S_InitStruct.I2S_TxChannelWidth   = I2S_Width_16Bits;
    I2S_InitStruct.I2S_RxChannelWidth   = I2S_Width_16Bits;
    I2S_InitStruct.I2S_TxWaterlevel     = 16;
    I2S_InitStruct.I2S_RxWaterlevel     = 16;
    I2S_InitStruct.I2S_RxFifoUsed       = I2S_FIFO_USE_0_REG_0;
    I2S_Init(I2S1, &I2S_InitStruct);
    /* Change the I2S transmission into external codec .*/

#if (I2S_CONFIG_GDMA == 0)

    I2S_INTConfig(I2S1, I2S_INT_RX_READY, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = SPORT1_RX_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
#endif
}

#if (I2S_CONFIG_GDMA == 1)
/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_gdma_tx_init()
{
    uint32_t i = 0;
    /* Test data buffer */
    for (i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        GDMA_Send_Buf[i] = i + 1;
    }

    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);
    /*--------------GDMA init-----------------------------*/
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum      = TDM0_TX_GDMA_Channel_NUM;
    GDMA_InitStruct.GDMA_DIR             = GDMA_DIR_MemoryToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize      = GDMA_TRANSFER_SIZE;//determine total transfer size
    GDMA_InitStruct.GDMA_SourceInc       = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc  = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceDataSize  = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_SourceMsize      = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_DestinationMsize = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_SourceAddr      = (uint32_t)GDMA_Send_Buf;
    GDMA_InitStruct.GDMA_DestinationAddr = (uint32_t)(&(I2S0->I2S_TX_FIFO_WR_ADDR));
    GDMA_InitStruct.GDMA_DestHandshake   = GDMA_Handshake_I2S0_TDM0_TX;
    GDMA_InitStruct.GDMA_ChannelPriority = 2;//channel prority between 0 to 5
    GDMA_InitStruct.GDMA_Secure_En = 1;
    GDMA_Init(TDM0_TX_GDMA_Channel, &GDMA_InitStruct);

    GDMA_INTConfig(TDM0_TX_GDMA_Channel_NUM, GDMA_INT_Transfer, ENABLE);

    /*-----------------GDMA IRQ init-------------------*/
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GDMA0_Channel3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void driver_gdma_rx_init()
{
    uint32_t i = 0;
    /* Test data buffer */
    for (i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        GDMA_Recv_Buf[i] = 0;

    }
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);
    /*--------------GDMA init-----------------------------*/
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum      = TDM0_RX_GDMA_Channel_NUM;
    GDMA_InitStruct.GDMA_DIR             = GDMA_DIR_PeripheralToMemory;
    GDMA_InitStruct.GDMA_BufferSize      = GDMA_TRANSFER_SIZE;//determine total transfer size
    GDMA_InitStruct.GDMA_SourceInc       = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc  = DMA_DestinationInc_Inc;
    GDMA_InitStruct.GDMA_SourceDataSize  = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_SourceMsize      = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_DestinationMsize = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_SourceAddr      = (uint32_t)(&(I2S1->I2S_RX_FIFO_RD_ADDR));
    GDMA_InitStruct.GDMA_DestinationAddr = (uint32_t)GDMA_Recv_Buf;
    GDMA_InitStruct.GDMA_SourceHandshake   = GDMA_Handshake_I2S1_TDM0_RX;
    GDMA_InitStruct.GDMA_ChannelPriority = 2;//channel prority between 0 to 5
    GDMA_InitStruct.GDMA_Secure_En = 1;
    GDMA_Init(TDM0_RX_GDMA_Channel, &GDMA_InitStruct);

    GDMA_INTConfig(TDM0_RX_GDMA_Channel_NUM, GDMA_INT_Transfer, ENABLE);

    /*-----------------GDMA IRQ init-------------------*/
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TDM0_RX_GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}
#endif

void i2s_demo(void)
{
    board_i2s_init();
    driver_i2s0_TDM2_tx_init();
    driver_i2s1_TDM2_rx_init();

#if (I2S_CONFIG_GDMA == 1)
    driver_gdma_tx_init();
    driver_gdma_rx_init();
    I2S_Cmd(I2S1, I2S_MODE_RX, ENABLE);
    GDMA_Cmd(TDM0_RX_GDMA_Channel_NUM, ENABLE);
    I2S_Cmd(I2S0, I2S_MODE_TX, ENABLE);
    GDMA_Cmd(TDM0_TX_GDMA_Channel_NUM, ENABLE);
#else
    I2S_Cmd(I2S1, I2S_MODE_RX, ENABLE);
    I2S_Cmd(I2S0, I2S_MODE_TX, ENABLE);

    uint32_t i = 0x12348800;
    for (uint16_t t = 0; t < 10 ; t++)
    {
        if (I2S_GetTxFIFOFreeLen(I2S0))
        {
            /* 16bit format, lower half word send first! */
            I2S_SendData(I2S0, i++);
        }
    }
#endif

}


/**
  * @brief  GDMA_Tx_Handler.
  * @param  No parameter.
  * @return void
*/
void TDM0_TX_GDMA_Channel_Handler(void)
{
    DBG_DIRECT("in gdma tx handler");
//    GDMA_SetSourceAddress(TDM0_GDMA_Channel, (uint32_t)(&(I2S0->REG_I2S_TX_FIFO_0_WR)));
//    GDMA_SetDestinationAddress(TDM0_GDMA_Channel, (uint32_t)(&(I2S1->REG_I2S_RX_FIFO_0)));

    GDMA_SetBufferSize(TDM0_TX_GDMA_Channel, GDMA_TRANSFER_SIZE);

    GDMA_ClearINTPendingBit(TDM0_TX_GDMA_Channel_NUM, GDMA_INT_Transfer);
    GDMA_Cmd(TDM0_TX_GDMA_Channel_NUM, DISABLE);
}

/**
  * @brief  GDMA_Rx_Handler.
  * @param  No parameter.
  * @return void
*/
void TDM0_RX_GDMA_Channel_Handler(void)
{
    DBG_DIRECT("in gdma Rx handler");
//    GDMA_SetSourceAddress(CODEC_GDMA_Channel, (uint32_t)(&(I2S0->REG_I2S_RX_FIFO_0)));
//    GDMA_SetDestinationAddress(CODEC_GDMA_Channel, (uint32_t)(&(UART5->RB_THR)));

    GDMA_SetBufferSize(TDM0_RX_GDMA_Channel, GDMA_TRANSFER_SIZE);

    GDMA_ClearINTPendingBit(TDM0_RX_GDMA_Channel_NUM, GDMA_INT_Transfer);
    for (uint32_t i = 0; i < GDMA_TRANSFER_SIZE ; i++)
    {
        DBG_DIRECT("recv data [%d]= %x", i, GDMA_Recv_Buf[i]);
    }

    GDMA_Cmd(TDM0_RX_GDMA_Channel_NUM, DISABLE);
}

/**
  * @brief  I2S1 interrupt handle function.
  * @param  None.
  * @return None.
*/
void SPORT1_RX_Handler(void)
{
    /* Notes: DBG_DIRECT is only used in debug demo, do not use in app project.*/
//    DBG_DIRECT("I2S1_RX_Handler");
    if (I2S_GetINTStatus(I2S1, I2S_INT_RX_READY))
    {
        uint8_t len = I2S_GetRxFIFOLen(I2S1);
        DBG_DIRECT("length = %d", len);
        DBG_DIRECT("length = %d", len);
        for (uint8_t i = 0; i < len; i++)
        {
            DBG_DIRECT("data[%d] = 0x%X", i, I2S_ReceiveFIFOData(I2S1));
        }
        I2S_ClearINTPendingBit(I2S1, I2S_INT_RX_READY);
    }
}


