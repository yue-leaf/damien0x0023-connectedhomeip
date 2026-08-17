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
#include "io_can.h"
#include "trace.h"

/*============================================================================*
 *                              Variables
 *============================================================================*/

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

    DBG_DIRECT("[CAN] BUS state: %d, waiting...", CAN_GetBusState());

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

    DBG_DIRECT("[CAN] BUS ON %d", CAN_GetBusState());
}

void can_tx_auto_reply(void)
{
    CANError_TypeDef tx_error;
    CANTxFrame_TypeDef tx_frame_type;

    uint8_t tx_data[8];
    for (uint8_t index = 0; index < 8; index++)
    {
        tx_data[index] = index;
    }

    tx_frame_type.msg_buf_id = 0;
    tx_frame_type.frame_type = CAN_STD_DATA_FRAME;
    tx_frame_type.standard_frame_id = 0x01;
    tx_frame_type.extend_frame_id = 0x01;
    tx_frame_type.auto_reply_bit = SET;
    CAN_MBTxINTConfig(tx_frame_type.msg_buf_id, ENABLE);
    tx_error = CAN_SetMsgBufTxMode(&tx_frame_type, tx_data, 8);

    while (CAN_GetRamState() != CAN_RAM_STATE_IDLE)
    {
        __asm volatile
        (
            "nop    \n"
        );
    }

    if (tx_error != CAN_NO_ERR)
    {
        DBG_DIRECT("[CAN]  tx error %d, line %d", tx_error, __LINE__);
    }

    DBG_DIRECT("[CAN]  tx auto reply remote frame id: 0x%x", tx_frame_type.standard_frame_id);
}

void can_rx_auto_reply(void)
{
    CANError_TypeDef rx_error;
    CANRxFrame_TypeDef rx_frame_type;

    rx_frame_type.msg_buf_id = 10;
    rx_frame_type.extend_frame_id = 0;
    rx_frame_type.standard_frame_id = 0x10;
    rx_frame_type.frame_ide_bit = CAN_IDE_STANDARD_FORMAT;
    rx_frame_type.frame_rtr_bit = CAN_RTR_DATA_FRAME;
    rx_frame_type.frame_rtr_mask = RESET;
    rx_frame_type.frame_ide_mask = RESET;
    rx_frame_type.frame_id_mask = 0;
    rx_frame_type.rx_dma_en = RESET;
    rx_frame_type.auto_reply_bit = SET;
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

    DBG_DIRECT("[CAN]  rx auto receive data frame id: 0x%x", rx_frame_type.standard_frame_id);
}

void can_demo(void)
{
    DBG_DIRECT("[CAN]  start can trx autoreply demo!");

    can_board_init();

    can_driver_init();

    uint8_t tx_data[8];
    for (uint8_t i = 0; i < 8; ++i)
    {
        tx_data[i] = i;
    }

    can_tx_auto_reply();
    can_rx_auto_reply();

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
        DBG_DIRECT("[CAN HANDLER]  CAN RX INT");
        CAN_ClearINTPendingBit(CAN_RX_INT_FLAG);

        for (uint8_t index = 0; index < CAN_MESSAGE_BUFFER_MAX_CNT; index++)
        {
            if (SET == CAN_GetMBnRxDoneFlag(index))
            {
                DBG_DIRECT("[CAN HANDLER]  MB_%d rx done", index);
                CAN_ClearMBnRxDoneFlag(index);

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


            }
        }
    }

    if (SET == CAN_GetINTStatus(CAN_TX_INT_FLAG))
    {
        CAN_ClearINTPendingBit(CAN_TX_INT_FLAG);
        DBG_DIRECT("[CAN HANDLER]  CAN TX INT");

        /* Enable autoreply next time. */

        can_tx_auto_reply();

        /* Add user code. */
    }
}



