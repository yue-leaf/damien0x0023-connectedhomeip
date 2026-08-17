/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include "io_spi.h"
#include "utils.h"

/* Globals ------------------------------------------------------------------*/
uint8_t GDMA_Send_Buffer[GDMA_TRANSFER_SIZE];
uint8_t GDMA_Recv_Buffer[GDMA_TRANSFER_SIZE];

volatile bool isDMATxDone = false;
volatile bool isDMARxDone = false;

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_spi_init(void)
{
    Pad_Config(SPI_SCK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI_MOSI_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI_MISO_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);

    Pinmux_Deinit(P4_0);
    Pinmux_Deinit(P4_1);
    Pinmux_Deinit(P4_2);
    Pinmux_Deinit(P4_3);

    Pinmux_Config(SPI_SCK_PIN, SPI0_CLK_MASTER);
    Pinmux_Config(SPI_MOSI_PIN, SPI0_MO_MASTER);
    Pinmux_Config(SPI_MISO_PIN, SPI0_MI_MASTER);

#if (CONFIG_SPI_SW_SIM_CS == 1)
    Pad_Config(SPI_CS_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
#else
    Pad_Config(SPI_CS_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pinmux_Config(SPI_CS_PIN, SPI0_CSN_0_MASTER);
#endif
}

/**
  * @brief  Initialize SPI peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_spi_init(void)
{
    /*----------------test buffer init------------------*/
    memset(GDMA_Recv_Buffer, 0, sizeof(GDMA_Recv_Buffer) / sizeof(GDMA_Recv_Buffer[0]));

    SPI_InitTypeDef  SPI_InitStructure;

    SPI_DeInit(FLASH_SPI);
    RCC_PeriphClockCmd(APBPeriph_SPI0, APBPeriph_SPI0_CLOCK, ENABLE);

    /*----------------------SPI init---------------------------------*/
    SPI_StructInit(&SPI_InitStructure);
    SPI_InitStructure.SPI_Direction   = SPI_Direction_FullDuplex;//SPI_Direction_RxOnly;
    SPI_InitStructure.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA        = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_BaudRatePrescaler = 20;
    SPI_InitStructure.SPI_FrameFormat = SPI_Frame_Motorola;
    SPI_InitStructure.SPI_TxDmaEn       = ENABLE;
    SPI_InitStructure.SPI_RxDmaEn     = ENABLE;
    SPI_InitStructure.SPI_RxWaterlevel = 1 - 1;
    SPI_InitStructure.SPI_TxWaterlevel  = 32 - 1;
    SPI_Init(FLASH_SPI, &SPI_InitStructure);

}

void driver_tx_gdma_init(void)
{
    memset(GDMA_Send_Buffer, 0, sizeof(GDMA_Send_Buffer) / sizeof(GDMA_Send_Buffer[0]));

    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    /*---------------------GDMA initial------------------------------*/
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = GDMA_TX_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_MemoryToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize          = 0;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;

    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;

    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)GDMA_Send_Buffer;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)SPI0->SPI_DR;
    GDMA_InitStruct.GDMA_DestHandshake       = GDMA_Handshake_SPI0_TX;
    GDMA_InitStruct.GDMA_ChannelPriority     = 1;

    GDMA_Init(GDMA_TX_CHANNEL, &GDMA_InitStruct);

    /* Enable transfer interrupt */
    GDMA_INTConfig(GDMA_TX_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    /*-----------------GDMA IRQ-----------------------------*/
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GDMA_TX_CHANNEL_IRQN;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void driver_rx_gdma_init(void)
{
    memset(GDMA_Recv_Buffer, 0, sizeof(GDMA_Recv_Buffer) / sizeof(GDMA_Recv_Buffer[0]));

    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    /*---------------------GDMA initial------------------------------*/
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = GDMA_RX_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_PeripheralToMemory;
    GDMA_InitStruct.GDMA_BufferSize          = 0;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Inc;

    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;

    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)SPI0->SPI_DR;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)GDMA_Recv_Buffer;
    GDMA_InitStruct.GDMA_SourceHandshake     = GDMA_Handshake_SPI0_RX;
    GDMA_InitStruct.GDMA_ChannelPriority     = 1;

    GDMA_Init(GDMA_RX_CHANNEL, &GDMA_InitStruct);
    GDMA_INTConfig(GDMA_RX_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
    /*-----------------GDMA IRQ-----------------------------*/
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GDMA_RX_CHANNEL_IRQN;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}


void pull_cs_down(bool isCSDown)
{
#if (CONFIG_SPI_SW_SIM_CS == 1)
    if (isCSDown == true)
    {
        Pad_Config(SPI_CS_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_LOW);
    }
    else
    {
        Pad_Config(SPI_CS_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    }
#endif
}


void spi_flash_busy_check(void)
{
    DBG_DIRECT("spi busy check");
    DBG_DIRECT("===================spi busy check==================");
    uint8_t send_buf[2] = {SPI_FLASH_READ_STATUS_REG_1, 0};
    uint8_t recv_len = 0;
    uint8_t recv_data[2] = {0};

    while (1)
    {
        SPI_SendBuffer(FLASH_SPI, send_buf, 2);
        while (SPI_GetFlagState(FLASH_SPI, SPI_FLAG_BUSY) == SET);
        recv_len = SPI_GetRxFIFOLen(FLASH_SPI);
        uint8_t i = 0;
        while (recv_len--)
        {
            recv_data[i++] = SPI_ReceiveData(FLASH_SPI);
        }
        i = 0;
        if ((recv_data[1] & 0x01) == 0)
        {
            break;
        }
    }

    //clear tx/rx FIFO
    platform_delay_us(1000);
    SPI_Cmd(SPI0, DISABLE);
    SPI_Cmd(SPI0, ENABLE);
}

void spi_flash_read_id(void)
{
    DBG_DIRECT("===================read_flash_id==================");
    memset(GDMA_Send_Buffer, 0, sizeof(GDMA_Send_Buffer) / sizeof(GDMA_Send_Buffer[0]));
    memset(GDMA_Recv_Buffer, 0, sizeof(GDMA_Recv_Buffer) / sizeof(GDMA_Recv_Buffer[0]));

    GDMA_Send_Buffer[0] = SPI_FLASH_JEDEC_ID;
    GDMA_SetBufferSize(GDMA_TX_CHANNEL, 4);
    GDMA_SetBufferSize(GDMA_RX_CHANNEL, 4);

    GDMA_INTConfig(GDMA_TX_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
    GDMA_INTConfig(GDMA_RX_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    pull_cs_down(true);

    SPI_Cmd(SPI0, ENABLE);
    GDMA_Cmd(GDMA_RX_CHANNEL_NUM, ENABLE);
    GDMA_Cmd(GDMA_TX_CHANNEL_NUM, ENABLE);

    DBG_DIRECT("after dma cmd enable");

    //wait for read id finish!
    while ((isDMATxDone != true) || (isDMARxDone != true))
    {
        DBG_DIRECT("isDMATxDone = %d", isDMATxDone);
        DBG_DIRECT("isDMARxDone = %d", isDMARxDone);
        platform_delay_ms(1000);
    }

    isDMATxDone = false;
    isDMARxDone = false;

    pull_cs_down(false);

    //clear tx/rx FIFO
    SPI_Cmd(SPI0, DISABLE);
    SPI_Cmd(SPI0, ENABLE);

}

void spi_flash_write_enable(void)
{
    DBG_DIRECT("===================write enable==================");
//  uint8_t len = 0;
//
//    SPI_SendData(FLASH_SPI, SPI_FLASH_WRITE_ENABLE);
//    while (SPI_GetFlagState(FLASH_SPI, SPI_FLAG_BUSY) == SET);
//    len = SPI_GetRxFIFOLen(FLASH_SPI);
//    while (len--)
//    {
//        SPI_ReceiveData(FLASH_SPI);
//    }
//
////    platform_delay_us(10);
//
//  //clear tx/rx FIFO
//  SPI_Cmd(SPI0, DISABLE);
//  platform_delay_us(10);
//  SPI_Cmd(SPI0, ENABLE);

    memset(GDMA_Send_Buffer, 0, sizeof(GDMA_Send_Buffer) / sizeof(GDMA_Send_Buffer[0]));
    memset(GDMA_Recv_Buffer, 0, sizeof(GDMA_Recv_Buffer) / sizeof(GDMA_Recv_Buffer[0]));

    GDMA_Send_Buffer[0] = SPI_FLASH_WRITE_ENABLE;

    GDMA_SetBufferSize(GDMA_TX_CHANNEL, 1);
    GDMA_SetBufferSize(GDMA_RX_CHANNEL, 1);

    GDMA_SetSourceAddress(GDMA_TX_CHANNEL, (uint32_t)GDMA_Send_Buffer);
    GDMA_SetDestinationAddress(GDMA_RX_CHANNEL, (uint32_t)GDMA_Recv_Buffer);

    GDMA_INTConfig(GDMA_TX_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
    GDMA_INTConfig(GDMA_RX_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    pull_cs_down(true);

    GDMA_Cmd(GDMA_RX_CHANNEL_NUM, ENABLE);
    GDMA_Cmd(GDMA_TX_CHANNEL_NUM, ENABLE);

    //wait for read id finish!
    while ((isDMATxDone != true) || (isDMARxDone != true))
    {
        DBG_DIRECT("isDMATxDone = %d", isDMATxDone);
        DBG_DIRECT("isDMARxDone = %d", isDMARxDone);
        platform_delay_ms(1000);
    }

    isDMATxDone = false;
    isDMARxDone = false;

    pull_cs_down(false);

    //clear tx/rx FIFO
    SPI_Cmd(SPI0, DISABLE);
    SPI_Cmd(SPI0, ENABLE);

}

void spi_flash_sector_erase(uint32_t vAddress)
{
    spi_flash_write_enable();

    DBG_DIRECT("===================sector_erase==================");
    memset(GDMA_Send_Buffer, 0, sizeof(GDMA_Send_Buffer) / sizeof(GDMA_Send_Buffer[0]));
    memset(GDMA_Recv_Buffer, 0, sizeof(GDMA_Recv_Buffer) / sizeof(GDMA_Recv_Buffer[0]));

    GDMA_Send_Buffer[0] = SPI_FLASH_SECTOR_ERASE;
    GDMA_Send_Buffer[1] = (vAddress >> 16) & 0xff;
    GDMA_Send_Buffer[2] = (vAddress >> 8) & 0xff;
    GDMA_Send_Buffer[3] = vAddress & 0xff;

    GDMA_SetBufferSize(GDMA_TX_CHANNEL, 4);
    GDMA_SetBufferSize(GDMA_RX_CHANNEL, 4);

    GDMA_SetSourceAddress(GDMA_TX_CHANNEL, (uint32_t)GDMA_Send_Buffer);
    GDMA_SetDestinationAddress(GDMA_RX_CHANNEL, (uint32_t)GDMA_Recv_Buffer);

    GDMA_INTConfig(GDMA_TX_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
    GDMA_INTConfig(GDMA_RX_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    pull_cs_down(true);

    GDMA_Cmd(GDMA_RX_CHANNEL_NUM, ENABLE);
    GDMA_Cmd(GDMA_TX_CHANNEL_NUM, ENABLE);

    //wait for read id finish!
    while ((isDMATxDone != true) || (isDMARxDone != true))
    {
        DBG_DIRECT("isDMATxDone = %d", isDMATxDone);
        DBG_DIRECT("isDMARxDone = %d", isDMARxDone);
        platform_delay_ms(1000);
    }

    isDMATxDone = false;
    isDMARxDone = false;

    pull_cs_down(false);

    //clear tx/rx FIFO
    SPI_Cmd(SPI0, DISABLE);
    SPI_Cmd(SPI0, ENABLE);


//  spi_flash_busy_check();
}

void spi_flash_page_write(uint32_t vWriteAddr, uint8_t *pBuffer, uint16_t vLength)
{
    uint16_t send_data_len = vLength;
    spi_flash_write_enable();

    DBG_DIRECT("===================page_write==================");
    memset(GDMA_Send_Buffer, 0, sizeof(GDMA_Send_Buffer) / sizeof(GDMA_Send_Buffer[0]));
    memset(GDMA_Recv_Buffer, 0, sizeof(GDMA_Recv_Buffer) / sizeof(GDMA_Recv_Buffer[0]));

    GDMA_Send_Buffer[0] = SPI_FLASH_PAGE_PROGRAM;
    GDMA_Send_Buffer[1] = (vWriteAddr >> 16) & 0xff;
    GDMA_Send_Buffer[2] = (vWriteAddr >> 8) & 0xff;
    GDMA_Send_Buffer[3] = vWriteAddr & 0xff;

    if (vLength > SPI_FLASH_PAGE_SIZE)
    {
        send_data_len = SPI_FLASH_PAGE_SIZE;
    }

    for (uint16_t i = 0; i < vLength; i++)
    {
        GDMA_Send_Buffer[4 + i] = pBuffer[i];
    }

    GDMA_SetBufferSize(GDMA_TX_CHANNEL, send_data_len + 4);
    GDMA_SetBufferSize(GDMA_RX_CHANNEL, send_data_len + 4);

    GDMA_INTConfig(GDMA_TX_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
    GDMA_INTConfig(GDMA_RX_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    pull_cs_down(true);

    SPI_Cmd(SPI0, DISABLE);
    SPI_Cmd(SPI0, ENABLE);
    GDMA_Cmd(GDMA_RX_CHANNEL_NUM, ENABLE);
    GDMA_Cmd(GDMA_TX_CHANNEL_NUM, ENABLE);

    //wait for read id finish!
    while ((isDMATxDone != true) || (isDMARxDone != true))
    {
        DBG_DIRECT("isDMATxDone = %d", isDMATxDone);
        DBG_DIRECT("isDMARxDone = %d", isDMARxDone);
        platform_delay_ms(1000);
    }

    isDMATxDone = false;
    isDMARxDone = false;

    pull_cs_down(false);

    //clear tx/rx FIFO
    SPI_Cmd(SPI0, DISABLE);
    SPI_Cmd(SPI0, ENABLE);

//  spi_flash_busy_check();

}

void spi_flash_read(uint8_t vReadCmd, uint32_t vReadAddr, uint8_t *pBuffer, uint16_t vLength)
{
    DBG_DIRECT("===================flash_read==================");
    memset(GDMA_Send_Buffer, 0, sizeof(GDMA_Send_Buffer) / sizeof(GDMA_Send_Buffer[0]));
    memset(GDMA_Recv_Buffer, 0, sizeof(GDMA_Recv_Buffer) / sizeof(GDMA_Recv_Buffer[0]));

    uint16_t read_data_len = vLength;

    if (SPI_FLASH_READ_DATA == vReadCmd)
    {
        read_data_len = vLength + 4;
    }
    else if (SPI_FLASH_FAST_READ == vReadCmd)
    {
        read_data_len = vLength + 5;
    }

    GDMA_SetBufferSize(GDMA_TX_CHANNEL, read_data_len);
    GDMA_SetBufferSize(GDMA_RX_CHANNEL, read_data_len);

    GDMA_Send_Buffer[0] = vReadCmd;
    GDMA_Send_Buffer[1] = (vReadAddr >> 16) & 0xff;
    GDMA_Send_Buffer[2] = (vReadAddr >> 8) & 0xff;
    GDMA_Send_Buffer[3] = vReadAddr & 0xff;

    GDMA_INTConfig(GDMA_TX_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
    GDMA_INTConfig(GDMA_RX_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    pull_cs_down(true);

    SPI_Cmd(SPI0, DISABLE);
    SPI_Cmd(SPI0, ENABLE);
    GDMA_Cmd(GDMA_RX_CHANNEL_NUM, ENABLE);
    GDMA_Cmd(GDMA_TX_CHANNEL_NUM, ENABLE);

    //wait for read id finish!
    while ((isDMATxDone != true) || (isDMARxDone != true))
    {
        DBG_DIRECT("isDMATxDone = %d", isDMATxDone);
        DBG_DIRECT("isDMARxDone = %d", isDMARxDone);
        platform_delay_ms(1000);
    }

    isDMATxDone = false;
    isDMARxDone = false;

    pull_cs_down(false);

    //clear tx/rx FIFO
    SPI_Cmd(SPI0, DISABLE);
    SPI_Cmd(SPI0, ENABLE);

}



/**
  * @brief  Demo code of operation about spi.
  * @param  No parameter.
  * @return void
*/
void spi_dma_demo(void)
{
    uint8_t write_data[100];
    uint8_t read_data[105] = {0};
    for (uint16_t i = 0; i < 100; i++)
    {
        write_data[i] = (i + 10) & 0xFF;
    }

    //read flash JEDEC id
    spi_flash_read_id();

    //sector erase
    spi_flash_sector_erase(0x001000);

    spi_flash_read(SPI_FLASH_FAST_READ, 0x001000, read_data, 100);

    spi_flash_page_write(0x001000, write_data, 100);

    spi_flash_read(SPI_FLASH_FAST_READ, 0x001000, read_data, 100);

}

/**
  * @brief  Demo code of operation about spi.
  * @param  No parameter.
  * @return void
*/
void spi_demo(void)
{
    APP_PRINT_INFO0("[io_spi] spi_demo");
    board_spi_init();
    driver_spi_init();
    driver_tx_gdma_init();
    driver_rx_gdma_init();
    spi_dma_demo();
}

void GDMA_TX_CHANNEL_HANDLER(void)
{
    GDMA_INTConfig(GDMA_TX_CHANNEL_NUM, GDMA_INT_Transfer, DISABLE);
    GDMA_ClearINTPendingBit(GDMA_TX_CHANNEL_NUM, GDMA_INT_Transfer);

    DBG_DIRECT("gdma tx data finish!");
    isDMATxDone = true;

    GDMA_Cmd(GDMA_TX_CHANNEL_NUM, DISABLE);
}

void GDMA_RX_CHANNEL_HANDLER(void)
{
    GDMA_INTConfig(GDMA_RX_CHANNEL_NUM, GDMA_INT_Transfer, DISABLE);
    GDMA_ClearINTPendingBit(GDMA_RX_CHANNEL_NUM, GDMA_INT_Transfer);

    DBG_DIRECT("gdma rx data finish!");
    uint16_t data_len = GDMA_GetTransferLen(GDMA_RX_CHANNEL);
    DBG_DIRECT("data_len = %d", data_len);
    for (uint16_t i = 0; i < data_len; i++)
    {
        DBG_DIRECT("dma rx data[%d] = %x", i, GDMA_Recv_Buffer[i]);
    }
    isDMARxDone = true;

    GDMA_Cmd(GDMA_RX_CHANNEL_NUM, DISABLE);
}

