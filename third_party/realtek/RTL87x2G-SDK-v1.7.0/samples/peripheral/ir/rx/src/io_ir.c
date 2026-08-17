/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include "io_ir.h"
#include "trace.h"
#include <string.h>
#include "utils.h"

/* Globals ------------------------------------------------------------------*/
IR_Data_TypeDef IR_Rx_Data;
/* Number of data which has been sent */
volatile uint16_t IR_RX_Count = 0;

/**
  * @brief  Initialization ir global data.
  * @param  No parameter.
  * @return void
  */
void global_data_ir_init(void)
{
    /* Initialize adc k value! */
    APP_PRINT_INFO0("[io_ir]global_data_ir_init");
    memset(&IR_Rx_Data, 0, sizeof(IR_Rx_Data));
    IR_RX_Count = 0;
}

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_ir_init(void)
{
    Pad_Config(IR_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);

    Pinmux_Config(IR_RX_PIN, IRDA_RX);
}

void board_pwm_init(void)
{
    Pad_Config(PWM_OUT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);

    Pinmux_Config(PWM_OUT_PIN, PWM_OUT_PIN_PINMUX);
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

    /* Initialize IR */
    IR_InitTypeDef IR_InitStruct;
    IR_StructInit(&IR_InitStruct);

    IR_InitStruct.IR_Freq               = 38000;/* IR carrier freqency is 38KHz */
    IR_InitStruct.IR_Mode               = IR_MODE_RX;/* IR receiveing mode */
    IR_InitStruct.IR_RxStartMode        = IR_RX_AUTO_MODE;
    IR_InitStruct.IR_RxFIFOThrLevel     =
        IR_RX_FIFO_THR_LEVEL; /* Configure RX FIFO threshold level to trigger IR_INT_RF_LEVEL interrupt */
    IR_InitStruct.IR_RxFIFOFullCtrl     =
        IR_RX_FIFO_FULL_DISCARD_NEWEST;/* Discard the lastest received dta if RX FIFO is full */
    IR_InitStruct.IR_RxFilterTime       =
        IR_RX_FILTER_TIME_50ns;/* If high to low or low to high transition time <= 50ns,Filter out it. */
    IR_InitStruct.IR_RxTriggerMode      = IR_RX_FALL_EDGE;/* Configure trigger type */
    IR_InitStruct.IR_RxCntThrType       = IR_RX_Count_Low_Level;
    IR_InitStruct.IR_RxCntThr           =
        0x200;/* Configure RX counter threshold.You can use it to decide to stop receiving IR data */
    IR_Init(&IR_InitStruct);
    IR_Cmd(IR_MODE_RX, ENABLE);
    IR_ClearRxFIFO();

    /* Configure NVIC */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = IR_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

}

void driver_pwm_init(void)
{
    /* TIMER peripheral clock enable */
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);

    /* Configure TIMER parameters */
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);
    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_InitStruct.TIM_PWM_En         = ENABLE;
    TIM_InitStruct.TIM_PWM_High_Count = PWM_HIGH_COUNT;
    TIM_InitStruct.TIM_PWM_Low_Count  = PWM_LOW_COUNT;
    TIM_TimeBaseInit(PWM_TIMER_NUM, &TIM_InitStruct);

}

/**
  * @brief  Demo code of ir send data.
  * @param  No parameter.
  * @return Void
*/
void ir_demo(void)
{
    /* Enable IR threshold interrupt. when RX FIFO offset >= threshold value, trigger interrupt*/
    /* Enable IR counter threshold interrupt to stop receiving data */
    IR_INTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, ENABLE);
    IR_MaskINTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, DISABLE);
    for (uint32_t i = 0; i < 50; i++)
    {
        TIM_Cmd(PWM_TIMER_NUM, ENABLE);
        platform_delay_us(500);
        TIM_Cmd(PWM_TIMER_NUM, DISABLE);
        platform_delay_us(500);
    }

}

/**
  * @brief  IR interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void IR_Handler(void)
{
//    DBG_DIRECT("INTO IR RX HANDLER");
    uint16_t len = 0;
    ITStatus int_status_rfl = IR_GetINTStatus(IR_INT_RF_LEVEL);
    ITStatus int_status_rxcnt = IR_GetINTStatus(IR_INT_RX_CNT_THR);

    /* Mask IR all interrupt */
    IR_MaskINTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, ENABLE);

    /* Receive by interrupt */
    if (int_status_rfl == SET)
    {
//        DBG_DIRECT("IR_INT_RF_LEVEL");
        len = IR_GetRxDataLen();
        IR_ReceiveBuf(IR_Rx_Data.DataBuf + IR_RX_Count, len);
        IR_Rx_Data.DataLen += len;
        IR_RX_Count += len;

        IR_ClearRxFIFO();

//        for (uint16_t i = 0; i < IR_RX_Count; i++)
//        {
////          APP_PRINT_INFO2("[io_ir]io_handle_ir_msg: IR RX data[%d] = 0x%x", i, IR_Rx_Data.DataBuf[i]);
//            DBG_DIRECT("[io_ir]io_handle_ir_msg: IR RX data[%d] = 0x%x", i, IR_Rx_Data.DataBuf[i]);
//        }

        IR_ClearINTPendingBit(IR_INT_RF_LEVEL_CLR);
    }

    /* Stop to receive IR data */
    if (int_status_rxcnt == SET)
    {
        DBG_DIRECT("IR_INT_RX_CNT_THR");
        /* Read remaining data */
        len = IR_GetRxDataLen();
        IR_ReceiveBuf(IR_Rx_Data.DataBuf + IR_RX_Count, len);
        IR_Rx_Data.DataLen += len;
        IR_RX_Count += len;

        DBG_DIRECT("len = %d", IR_RX_Count);

        for (uint16_t i = 0; i < IR_RX_Count; i++)
        {
            DBG_DIRECT("[io_ir]io_handle_ir_msg: IR RX data[%d] = 0x%x", i, IR_Rx_Data.DataBuf[i]);
        }
        memset(&IR_Rx_Data, 0, sizeof(IR_Rx_Data));
        IR_RX_Count = 0;

        IR_ClearINTPendingBit(IR_INT_RX_CNT_THR_CLR);
    }

    /* Unmask IR all interrupt */
    IR_MaskINTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, DISABLE);
}

