/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "rtl_rcc.h"
#include "rtl_can.h"
#include "string.h"
#include "rtl_pinmux.h"
#include "rtl_nvic.h"
#include "rtl_gdma.h"
#include "io_can.h"
#include "trace.h"

/*============================================================================*
 *                              Variables
 *============================================================================*/
CANRxDmaData_TypeDef rx_dma_data_struct;

CAN_RAM_TypeDef tx_can_ram_struct;
/*============================================================================*
 *                              Functions
 *============================================================================*/
void can_board_init(void)
{
    /* Config pinmux and pad for CAN. */

    Pinmux_Config(CAN_TX_PIN, CAN_TX);
    Pinmux_Config(CAN_RX_PIN, CAN_RX);

    Pad_Config(CAN_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(CAN_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
}

void can_driver_init(void)
{
    CAN_DeInit();

    /* Enable rcc for CAN before initialize. */

    RCC_PeriphClockCmd(APBPeriph_CAN, APBPeriph_CAN_CLOCK, ENABLE);

    DBG_DIRECT("[CAN]  BUS state: %d, waiting...", CAN_GetBusState());

    /* Initialize CAN. */

    CAN_InitTypeDef init_struct;
    CAN_StructInit(&init_struct);
    init_struct.CAN_AutoReTxEn = DISABLE;

    /* can speed = 40mhz / ((brp + 1)*(1 + tseg1 + 1 + tseg2 + 1)) */
    /* Here config to 500khz. */

    init_struct.CAN_BitTiming.b.can_brp = 3;
    init_struct.CAN_BitTiming.b.can_sjw = 3;
    init_struct.CAN_BitTiming.b.can_tseg1 = 13;
    init_struct.CAN_BitTiming.b.can_tseg2 = 4;

    init_struct.CAN_RxDmaEn = ENABLE;
    CAN_Init(&init_struct);

    /* CAN enable */

    CAN_Cmd(ENABLE);

    /* CAN interrupts enable */

    CAN_INTConfig((CAN_BUS_OFF_INT | CAN_WAKE_UP_INT | CAN_ERROR_INT |
                   CAN_RX_INT | CAN_TX_INT), ENABLE);

    /* NVIC enable */

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = CAN_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    /* polling CAN bus on status */

    while (CAN_GetBusState() != CAN_BUS_STATE_ON)
    {
        __asm volatile
        (
            "nop    \n"
        );
    }

    DBG_DIRECT("[CAN]  BUS ON %d", CAN_GetBusState());
}

void can_dma_rx(void)
{
    CANError_TypeDef rx_error;
    CANRxFrame_TypeDef rx_frame_type;
    rx_frame_type.msg_buf_id = RX_DMA_BUF_ID;
    rx_frame_type.frame_rtr_mask = SET;
    rx_frame_type.frame_ide_mask = SET;
    rx_frame_type.frame_id_mask = CAN_FRAME_ID_MASK_MAX_VALUE;
    rx_frame_type.rx_dma_en = SET;
    rx_frame_type.auto_reply_bit = RESET;
    rx_frame_type.rx_msg_buf_enable = SET;
    rx_error = CAN_SetMsgBufRxMode(&rx_frame_type);

    CAN_MBRxINTConfig(rx_frame_type.msg_buf_id, ENABLE);

    while (CAN_GetRamState() != CAN_RAM_STATE_IDLE)
    {
        __asm volatile
        (
            "nop    \n"
        );
    }

    if (rx_error != CAN_NO_ERR)
    {
        DBG_DIRECT("[CAN]  rx error %d, line %d", rx_error, __LINE__);
    }

    DBG_DIRECT("[CAN]  waiting for rx...");
}

void rx_gdma_driver_init(void)
{
    /* Turn on gdma clock */
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);

    /* GDMA initial*/
    GDMA_InitStruct.GDMA_ChannelNum          = RX_GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_PeripheralToMemory;
    GDMA_InitStruct.GDMA_BufferSize          = 0;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Inc;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t) & (CAN->CAN_RX_DMA_DATA);
    GDMA_InitStruct.GDMA_DestinationAddr     = 0;
    GDMA_InitStruct.GDMA_SourceHandshake     = GDMA_Handshake_CAN_BUS_RX;
    GDMA_InitStruct.GDMA_Secure_En           = ENABLE;

    GDMA_Init(RX_GDMA_Channel, &GDMA_InitStruct);

    GDMA_INTConfig(RX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    /* GDMA irq init */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = RX_GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);
}

void tx_gdma_driver_init()
{
    /* Turn on gdma clock */

    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);

    /* GDMA initial*/

    GDMA_InitStruct.GDMA_ChannelNum          = TX_GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_MemoryToMemory;
    GDMA_InitStruct.GDMA_BufferSize          = sizeof(CAN_RAM_TypeDef);
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Inc;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_8;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_8;
    GDMA_InitStruct.GDMA_SourceAddr          = 0;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)(CAN->CAN_RAM_DATA);
    GDMA_InitStruct.GDMA_Secure_En           = ENABLE;

    GDMA_Init(TX_GDMA_Channel, &GDMA_InitStruct);

    GDMA_INTConfig(TX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    /* GDMA irq init */

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = TX_GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);
}

void can_dma_tx(void)
{
    for (uint8_t index = 0; index < 8; index++)
    {
        ((CAN_RAM_DATA_TYPE_TypeDef *)&tx_can_ram_struct.CAN_RAM_DATA[CAN_RAM_DATA_MAX_INDEX - index /
                                                                      4])->d8[index % 4]
        \
            = index;
    }

    /* prepare can ram data for tx dma */

    tx_can_ram_struct.u_CAN_RAM_ARB.CAN_RAM_ARB_BITS.b.can_ram_rtr = CAN_RTR_DATA_FRAME;
    tx_can_ram_struct.u_CAN_RAM_ARB.CAN_RAM_ARB_BITS.b.can_ram_ide = CAN_IDE_EXTEND_FORMAT;
    tx_can_ram_struct.u_CAN_RAM_ARB.CAN_RAM_ARB_BITS.b.can_ram_id = ((0x01 &
                                                                      CAN_STAND_FRAME_ID_MAX_VALUE) <<
                                                                     CAN_STD_FRAME_ID_POS) \
                                                                    | ((0x01 & CAN_EXTEND_FRAME_ID_MAX_VALUE) << CAN_EXT_FRAME_ID_POS);

    tx_can_ram_struct.u_CAN_RAM_MASK.CAN_RAM_MASK_BITS.b.can_ram_ide_mask = 0;
    tx_can_ram_struct.u_CAN_RAM_MASK.CAN_RAM_MASK_BITS.b.can_ram_rtr_mask = 0;
    tx_can_ram_struct.u_CAN_RAM_MASK.CAN_RAM_MASK_BITS.b.can_ram_id_mask = 0;

    tx_can_ram_struct.u_CAN_RAM_CS.CAN_RAM_CS_BITS.b.can_ram_rxtx = SET;
    tx_can_ram_struct.u_CAN_RAM_CS.CAN_RAM_CS_BITS.b.reset = 0;
    tx_can_ram_struct.u_CAN_RAM_CS.CAN_RAM_CS_BITS.b.can_ram_dlc = CAN_DLC_BYTES_8;

    tx_can_ram_struct.u_CAN_RAM_CMD.CAN_RAM_CMD_BITS.b.can_ram_start = SET;
    tx_can_ram_struct.u_CAN_RAM_CMD.CAN_RAM_CMD_BITS.b.can_ram_dir = SET;
    tx_can_ram_struct.u_CAN_RAM_CMD.CAN_RAM_CMD_BITS.b.can_ram_buffer_en = SET;
    tx_can_ram_struct.u_CAN_RAM_CMD.CAN_RAM_CMD_BITS.b.can_ram_acc_data = 0xFFFF;
    tx_can_ram_struct.u_CAN_RAM_CMD.CAN_RAM_CMD_BITS.b.can_ram_acc_arb = SET;
    tx_can_ram_struct.u_CAN_RAM_CMD.CAN_RAM_CMD_BITS.b.can_ram_acc_cs = SET;
    tx_can_ram_struct.u_CAN_RAM_CMD.CAN_RAM_CMD_BITS.b.can_ram_acc_mask = SET;
    tx_can_ram_struct.u_CAN_RAM_CMD.CAN_RAM_CMD_BITS.b.can_ram_acc_num = 0;
}

void can_start_tx_dma(CAN_RAM_TypeDef *p_can_ram_data)
{
    GDMA_SetSourceAddress(TX_GDMA_Channel, (uint32_t)p_can_ram_data);
    GDMA_INTConfig(TX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
    GDMA_Cmd(TX_GDMA_CHANNEL_NUM, ENABLE);
}

void can_start_rx_dma(uint32_t des_addr, uint32_t buffer_size)
{
    GDMA_SetBufferSize(RX_GDMA_Channel, buffer_size);
    GDMA_SetDestinationAddress(RX_GDMA_Channel, des_addr);
    GDMA_INTConfig(RX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
    GDMA_Cmd(RX_GDMA_CHANNEL_NUM, ENABLE);
}

void can_demo(void)
{
    DBG_DIRECT("[CAN]  start can trx_gdma demo!");

    can_board_init();

    can_driver_init();
    rx_gdma_driver_init();
    tx_gdma_driver_init();
    can_dma_tx();
    can_start_tx_dma(&tx_can_ram_struct);
    can_dma_rx();

    /* Waiting for rx data to generate interrupt. */
}

void CAN_Handler(void)
{
    if (SET == CAN_GetINTStatus(CAN_RAM_MOVE_DONE_INT_FLAG))
    {
        CAN_ClearINTPendingBit(CAN_RAM_MOVE_DONE_INT_FLAG);

        /* Add user code. */
    }

    if (SET == CAN_GetINTStatus(CAN_BUS_OFF_INT_FLAG))
    {
        DBG_DIRECT("[CAN HANDLER]  CAN BUS OFF");
        CAN_ClearINTPendingBit(CAN_BUS_OFF_INT_FLAG);

        /* Add user code. */
    }

    if (SET == CAN_GetINTStatus(CAN_WAKE_UP_INT_FLAG))
    {
        DBG_DIRECT("[CAN HANDLER]  CAN WAKE UP");
        CAN_ClearINTPendingBit(CAN_WAKE_UP_INT_FLAG);

        /* Add user code. */
    }

    if (SET == CAN_GetINTStatus(CAN_ERROR_INT_FLAG))
    {
        DBG_DIRECT("[CAN HANDLER]  CAN ERROR");
        CAN_ClearINTPendingBit(CAN_ERROR_INT_FLAG);

        if (SET == CAN_GetErrorStatus(CAN_ERROR_RX))
        {
            DBG_DIRECT("[CAN HANDLER]  CAN ERROR RX");
            CAN_CLearErrorStatus(CAN_ERROR_RX);

            /* Add user code. */
        }

        if (SET == CAN_GetErrorStatus(CAN_ERROR_TX))
        {
            DBG_DIRECT("[CAN HANDLER]  CAN ERROR TX");
            CAN_CLearErrorStatus(CAN_ERROR_TX);

            for (uint8_t index = 0; index < CAN_MESSAGE_BUFFER_MAX_CNT; index++)
            {
                if (SET == CAN_GetMBnTxErrorFlag(index))
                {
                    DBG_DIRECT("[CAN HANDLER]  CAN ERROR TX MB_%d", index);
                    CAN_ClearMBnTxErrorFlag(index);

                    /* Add user code. */
                }
            }
        }

        if (SET == CAN_GetErrorStatus(CAN_ERROR_ACK))
        {
            DBG_DIRECT("[CAN HANDLER]  CAN ERROR ACK");
            CAN_CLearErrorStatus(CAN_ERROR_ACK);
            /* Add user code. */
        }

        if (SET == CAN_GetErrorStatus(CAN_ERROR_STUFF))
        {
            DBG_DIRECT("[CAN HANDLER]  CAN ERROR STUFF");
            CAN_CLearErrorStatus(CAN_ERROR_STUFF);

            /* Add user code. */
        }

        if (SET == CAN_GetErrorStatus(CAN_ERROR_CRC))
        {
            DBG_DIRECT("[CAN HANDLER]  CAN_ERROR_CRC");
            CAN_CLearErrorStatus(CAN_ERROR_CRC);

            /* Add user code. */
        }

        if (SET == CAN_GetErrorStatus(CAN_ERROR_FORM))
        {
            DBG_DIRECT("[CAN HANDLER]  CAN ERROR FORM");
            CAN_CLearErrorStatus(CAN_ERROR_FORM);

            /* Add user code. */
        }

        if (SET == CAN_GetErrorStatus(CAN_ERROR_BIT1))
        {
            DBG_DIRECT("[CAN HANDLER]  CAN ERROR BIT1");
            CAN_CLearErrorStatus(CAN_ERROR_BIT1);

            /* Add user code. */
        }

        if (SET == CAN_GetErrorStatus(CAN_ERROR_BIT0))
        {
            DBG_DIRECT("[CAN HANDLER]  CAN ERROR BIT0");
            CAN_CLearErrorStatus(CAN_ERROR_BIT0);

            /* Add user code. */
        }
    }

    if (SET == CAN_GetINTStatus(CAN_RX_INT_FLAG))
    {
        DBG_DIRECT("[CAN HANDLER]  CAN RX");
        CAN_ClearINTPendingBit(CAN_RX_INT_FLAG);

        for (uint8_t index = 0; index < CAN_MESSAGE_BUFFER_MAX_CNT; index++)
        {
            if (SET == CAN_GetMBnRxDoneFlag(index))
            {
                DBG_DIRECT("[CAN HANDLER]  MB_%d rx done", index);
                CAN_ClearMBnRxDoneFlag(index);

                FlagStatus dma_en_flag = CAN_GetMBnRxDmaEnFlag(index);
                if (dma_en_flag == ENABLE)
                {
                    uint32_t dma_buffer_size = CAN_GetRxDmaMsize();
                    can_start_rx_dma((uint32_t)&rx_dma_data_struct, dma_buffer_size);
                }
                else
                {
                    CANError_TypeDef get_error;
                    CANMsgBufInfo_TypeDef mb_info;
                    get_error = CAN_GetMsgBufInfo(index, &mb_info);

                    uint8_t rx_data[8];
                    memset(rx_data, 0, 8);
                    CAN_GetRamData(mb_info.data_length, rx_data);

                    CANDataFrameSel_TypeDef frame_type = CAN_CheckFrameType(mb_info.rtr_bit, mb_info.ide_bit);

                    DBG_DIRECT("[CAN HANDLER]  frame_type %d, frame_id = 0x%03x, ext_frame_id = 0x%05x", \
                               frame_type, mb_info.standard_frame_id, mb_info.extend_frame_id);

                    for (uint8_t index = 0; index < mb_info.data_length; index++)
                    {
                        DBG_DIRECT("[CAN HANDLER]  rx_data [%d] 0x%02x", index, rx_data[index]);
                    }

                    /* Add user code. */

                }
            }
        }
    }

    if (SET == CAN_GetINTStatus(CAN_TX_INT_FLAG))
    {
        CAN_ClearINTPendingBit(CAN_TX_INT_FLAG);

        for (uint8_t index = 0; index < CAN_MESSAGE_BUFFER_MAX_CNT; index++)
        {
            if (SET == CAN_GetMBnTxDoneFlag(index))
            {
                DBG_DIRECT("[CAN HANDLER]  MB_%d tx done", index);
                CAN_ClearMBnTxDoneFlag(index);

                /* Add user code. */
            }
        }
    }
}


void RX_GDMA_Channel_Handler(void)
{
    GDMA_INTConfig(RX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, DISABLE);

    DBG_DIRECT("[RX DMA HANDLER]  GDMA_Channel_Handler");
    CAN_SetMBnRxDmaEnFlag(RX_DMA_BUF_ID, ENABLE);
    DBG_DIRECT("[RX DMA HANDLER]  std id = 0x%x, ext id = 0x%x dlc = %d", \
               (rx_dma_data_struct.can_ram_arb.b.can_ram_id & 0x7ff << 18) >> 18, \
               rx_dma_data_struct.can_ram_arb.b.can_ram_id & 0x3ffff, \
               rx_dma_data_struct.can_ram_cs.b.can_ram_dlc);

    if (rx_dma_data_struct.can_ram_cs.b.can_ram_dlc < 9)
    {
        for (uint8_t index = 0; index < rx_dma_data_struct.can_ram_cs.b.can_ram_dlc; index++)
        {
            DBG_DIRECT("[RX DMA HANDLER]  rx_data [%d] 0x%02x", index, rx_dma_data_struct.rx_dma_data[index]);
        }
    }
    else if (rx_dma_data_struct.can_ram_cs.b.can_ram_dlc < 13)
    {
        for (uint8_t index = 0; index < (rx_dma_data_struct.can_ram_cs.b.can_ram_dlc - 8) * 4 + 8; index++)
        {
            DBG_DIRECT("[RX DMA HANDLER]  rx_data [%d] 0x%02x", index, rx_dma_data_struct.rx_dma_data[index]);
        }
    }
    else if (rx_dma_data_struct.can_ram_cs.b.can_ram_dlc < 16)
    {
        for (uint8_t index = 0; index < (rx_dma_data_struct.can_ram_cs.b.can_ram_dlc - 13) * 16 + 32;
             index++)
        {
            DBG_DIRECT("[RX DMA HANDLER]  rx_data [%d] 0x%02x", index, rx_dma_data_struct.rx_dma_data[index]);
        }
    }

    /* Add user code. */

    GDMA_ClearINTPendingBit(RX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
    DBG_DIRECT("[CAN]  waiting for rx...");
}

void TX_GDMA_Channel_Handler(void)
{
    GDMA_INTConfig(TX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, DISABLE);

    DBG_DIRECT("[TX DMA HANDLER]  TX DONE!");

    GDMA_ClearINTPendingBit(TX_GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
}



