/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include "io_ir.h"
#include "trace.h"
#include <string.h>


/* Globals ------------------------------------------------------------------*/
uint32_t GDMA_Recv_Buf[4095];
uint16_t IR_GDMA_Rev_Data_Len = 0;

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_ir_init(void)
{
    Pad_Config(IR_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_PullConfigValue(IR_RX_PIN, PAD_PULL_STRONG);

    Pinmux_Config(IR_RX_PIN, IRDA_RX);
}

void board_pwm_init(void)
{
    Pad_Config(PWM_OUT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    /* Normal mode */
    Pinmux_Config(PWM_OUT_PIN, PWM_OUT_PINMUX);
}


/**
  * @brief  Initialize ir peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_ir_init(void)
{
    /* Enable IR clock */
    RCC_PeriphClockCmd(APBPeriph_IR, APBPeriph_IR_CLOCK, ENABLE);
//    RCC_IRClockConfig(ENABLE);

    /* Initialize IR */
    IR_InitTypeDef IR_InitStruct;
    IR_StructInit(&IR_InitStruct);

    IR_InitStruct.IR_Clock              = IR_CLOCK_40M;
    IR_InitStruct.IR_Freq               = 40000000;
//    IR_InitStruct.IR_Freq               = 38000;
    IR_InitStruct.IR_Mode               = IR_MODE_RX;
    IR_InitStruct.IR_RxStartMode        = IR_RX_AUTO_MODE;
    IR_InitStruct.IR_RxFIFOThrLevel     = IR_RX_FIFO_THR_LEVEL;
    IR_InitStruct.IR_RxFIFOFullCtrl     = IR_RX_FIFO_FULL_DISCARD_NEWEST;
    IR_InitStruct.IR_RxFilterTime       = IR_RX_FILTER_TIME_50ns;
    IR_InitStruct.IR_RxTriggerMode      = IR_RX_FALL_EDGE;
    IR_InitStruct.IR_RxCntThrType       = IR_RX_Count_Low_Level;
    IR_InitStruct.IR_RxCntThr           = 0x0ED8;
    IR_InitStruct.IR_RxDmaEn            = ENABLE;
    IR_InitStruct.IR_RxWaterLevel       = 4;

    IR_Init(&IR_InitStruct);
    IR_Cmd(IR_MODE_RX, ENABLE);
    IR_ClearRxFIFO();
}

void driver_pwm_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);

    TIM_InitStruct.TIM_PWM_En = ENABLE;
    TIM_InitStruct.TIM_PWM_High_Count = PWM_HIGH_COUNT;
    TIM_InitStruct.TIM_PWM_Low_Count  = PWM_LOW_COUNT;
    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_TimeBaseInit(PWM_TIMER_NUM, &TIM_InitStruct);

    TIM_Cmd(PWM_TIMER_NUM, ENABLE);
}

/**
  * @brief  Initialize GDMA peripheral.
  * @param   No parameter.
  * @return  void
  */
void driver_ir_gdma_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);
    GDMA_InitTypeDef GDMA_InitStruct;

    memset(GDMA_Recv_Buf, 0, sizeof(GDMA_Recv_Buf));

    /*--------------GDMA init-----------------------------*/
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum         = IO_TEST_GDMA_CHANNEL_MUM;
    GDMA_InitStruct.GDMA_BufferSize         = IO_TEST_GDMA_TRANSFER_SIZE;//determine total transfer size
    GDMA_InitStruct.GDMA_DIR                = GDMA_DIR_PeripheralToMemory;//GDMA_DIR_PeripheralToMemory
    GDMA_InitStruct.GDMA_SourceDataSize     = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_SourceMsize        = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_DestinationMsize   = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_SourceInc          = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc     = DMA_DestinationInc_Inc;
    GDMA_InitStruct.GDMA_SourceAddr         = (uint32_t)(&IR->IR_RX_FIFO);
    GDMA_InitStruct.GDMA_DestinationAddr    = (uint32_t)(GDMA_Recv_Buf);
    GDMA_InitStruct.GDMA_SourceHandshake    = GDMA_Handshake_IR_RX;
    GDMA_InitStruct.GDMA_Secure_En          = ENABLE;
    GDMA_Init(IO_TEST_GDMA_Channel, &GDMA_InitStruct);

    GDMA_INTConfig(IO_TEST_GDMA_CHANNEL_MUM, GDMA_INT_Transfer, ENABLE);

    /*-----------------GDMA IRQ init-------------------*/
    NVIC_InitTypeDef nvic_init_struct;
    nvic_init_struct.NVIC_IRQChannel         = IO_TEST_GDMA_Channel_IRQn;
    nvic_init_struct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    nvic_init_struct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&nvic_init_struct);

    GDMA_Cmd(IO_TEST_GDMA_CHANNEL_MUM, ENABLE);
}

/**
  * @brief  Handle gdma data function.
  * @param  No parameter.
  * @return void
  */
void io_handle_gdma_msg(void)
{
    DBG_DIRECT("io_handle_gdma_msg: IR_GDMA_Rev_Data_Len = %d \r\n", IR_GDMA_Rev_Data_Len);
    for (uint32_t i = 0; i < IR_GDMA_Rev_Data_Len; i++)
    {
        DBG_DIRECT("io_handle_gdma_msg: GDMA_Recv_Buf[%d] = 0x%x \r\n", i, GDMA_Recv_Buf[i]);
    }
}

/**
  * @brief  GDMA channel interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void IO_TEST_GDMA_Channel_Handler(void)
{
    GDMA_INTConfig(IO_TEST_GDMA_CHANNEL_MUM, GDMA_INT_Transfer, DISABLE);
    GDMA_Cmd(IO_TEST_GDMA_CHANNEL_MUM, DISABLE);
    IR_GDMA_Rev_Data_Len = IO_TEST_GDMA_TRANSFER_SIZE;
    io_handle_gdma_msg();
    GDMA_ClearINTPendingBit(IO_TEST_GDMA_CHANNEL_MUM, GDMA_INT_Transfer);
    GDMA_Cmd(IO_TEST_GDMA_CHANNEL_MUM, ENABLE);
    GDMA_INTConfig(IO_TEST_GDMA_CHANNEL_MUM, GDMA_INT_Transfer, ENABLE);
}
