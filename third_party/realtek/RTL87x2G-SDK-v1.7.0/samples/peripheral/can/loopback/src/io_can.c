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
#include "io_can.h"
#include "trace.h"

/*============================================================================*
 *                              Variables
 *============================================================================*/

/*============================================================================*
 *                              Functions
 *============================================================================*/
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

    init_struct.CAN_TestModeSel = CAN_TEST_MODE_INT_LOOPBACK;
    CAN_Init(&init_struct);

    /* CAN enable */

    CAN_Cmd(ENABLE);

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

void can_loopback(uint32_t buf_id, uint8_t frame_type, \
                  uint16_t frame_id, uint32_t ext_id, uint8_t *tx_data, uint8_t data_len)
{
    /* Set rx message buffer. */

    CANError_TypeDef rx_error;
    CANRxFrame_TypeDef rx_frame_type;

    rx_frame_type.msg_buf_id = 15;

    /* Set 0 to filter related bit, and set 1 to mask related filter. */

    rx_frame_type.frame_rtr_mask = SET;
    rx_frame_type.frame_ide_mask = SET;
    rx_frame_type.frame_id_mask = CAN_FRAME_ID_MASK_MAX_VALUE;
    rx_frame_type.rx_dma_en = RESET;
    rx_frame_type.auto_reply_bit = RESET;
    rx_frame_type.rx_msg_buf_enable = SET;

    rx_error = CAN_SetMsgBufRxMode(&rx_frame_type);

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

    /* Set tx message buffer. */

    CANError_TypeDef tx_error = CAN_NO_ERR;

    CANTxFrame_TypeDef tx_frame_type;

    tx_frame_type.msg_buf_id = buf_id;
    tx_frame_type.frame_type = frame_type;
    tx_frame_type.standard_frame_id = frame_id;
    tx_frame_type.extend_frame_id = 0;
    tx_frame_type.auto_reply_bit = DISABLE;

    switch (frame_type)
    {
    case CAN_EXT_DATA_FRAME:
    case CAN_EXT_REMOTE_FRAME:
        tx_frame_type.extend_frame_id = ext_id;
    case CAN_STD_DATA_FRAME:
    case CAN_STD_REMOTE_FRAME:
        break;
    }

    tx_error = CAN_SetMsgBufTxMode(&tx_frame_type, tx_data, data_len);

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

    /* Polling tx done. */

    while (SET != CAN_GetMBnTxDoneFlag(tx_frame_type.msg_buf_id))
    {
        __asm volatile
        (
            "nop    \n"
        );
    }

    DBG_DIRECT("[CAN]  BUS TX done");

    /* Polling rx done. */

    while (SET != CAN_GetMBnRxDoneFlag(rx_frame_type.msg_buf_id))
    {
        __asm volatile
        (
            "nop    \n"
        );
    }

    /* Receive rx data. */

    CANError_TypeDef get_error;
    CANMsgBufInfo_TypeDef mb_info;
    get_error = CAN_GetMsgBufInfo(rx_frame_type.msg_buf_id, &mb_info);

    uint8_t rx_data[8];
    memset(rx_data, 0, 8);
    CAN_GetRamData(mb_info.data_length, rx_data);

    CANDataFrameSel_TypeDef get_frame_type = CAN_CheckFrameType(mb_info.rtr_bit, mb_info.ide_bit);

    DBG_DIRECT("[CAN]  rx frame_type %d", get_frame_type);
    DBG_DIRECT("[CAN]  rx frame_id = 0x%x, ext_frame_id = 0x%x", mb_info.standard_frame_id,
               mb_info.extend_frame_id);

    for (uint8_t index = 0; index < mb_info.data_length; index++)
    {
        DBG_DIRECT("[CAN]  rx_data [%d] 0x%02X", index, rx_data[index]);
    }

    DBG_DIRECT("[CAN]  BUS RX done");
}

void can_demo(void)
{
    DBG_DIRECT("[CAN]  start can loopback demo!");

    can_driver_init();

    /* Send tx messages. */
    /* Set CAN tx message buffer */

    uint8_t tx_data[8];
    for (uint8_t i = 0; i < 8; ++i)
    {
        tx_data[i] = i;
    }

    /* Send standard data frame. */

    DBG_DIRECT("[CAN]  send standard data frame");
    can_loopback(0, CAN_STD_DATA_FRAME, 0x123, 0, tx_data, 8);

    /* Send extend data frame. */

    DBG_DIRECT("[CAN]  send extend data frame");
    can_loopback(1, CAN_EXT_DATA_FRAME, 0x123, 0x4567, tx_data, 8);

    /* Send standard remote frame. */

    DBG_DIRECT("[CAN]  send standard remote frame");
    can_loopback(2, CAN_STD_REMOTE_FRAME, 0x7ff, 0, tx_data, 0);

    /* Send extend remote frame. */

    DBG_DIRECT("[CAN]  send extend remote frame");
    can_loopback(3, CAN_EXT_REMOTE_FRAME, 0x7ff, 0x3ffff, tx_data, 0);

    /* Waiting for rx data to generate interrupt. */

    DBG_DIRECT("[CAN]  end of can loopback demo");
}



