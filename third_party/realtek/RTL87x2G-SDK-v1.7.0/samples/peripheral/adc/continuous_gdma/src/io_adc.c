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
#include "rtl_gdma.h"
#include "rtl_adc.h"
#include "io_adc.h"
#include "utils.h"
#include <string.h>

/* Globals ------------------------------------------------------------------*/
uint16_t ADC_Recv_Buffer[GDMA_TRANSFER_SIZE];
/* Private define ------------------------------------------------------------*/

#define ADC_CONFIG_GDMA_MODE_EN             1

/* ADC bypass mode or divide mode */
#define ADC_DIVIDE_MODE                     1
#define ADC_BYPASS_MODE                     0
/* Change the ADC sampling mode here! */
#define ADC_MODE_DIVIDE_OR_BYPASS           ADC_DIVIDE_MODE

/* Private function prototypes -----------------------------------------------*/
void global_adc_init(void)
{
    bool adc_k_status = false;
    adc_k_status = ADC_CalibrationInit();
    if (false == adc_k_status)
    {
        DBG_DIRECT("[io_adc] global_data_adc_init: ADC_CalibrationInit fail!");
    }
    memset(&ADC_Recv_Buffer, 0, sizeof(ADC_Recv_Buffer));
}

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_adc_init(void)
{
    Pad_Config(ADC_SAMPLE_PIN_0, PAD_SW_MODE, PAD_NOT_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
}

/**
  * @brief  Initialize ADC peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_adc_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

    ADC_InitTypeDef ADC_InitStruct;
    ADC_StructInit(&ADC_InitStruct);

    /* Configure the ADC sampling schedule0 */
    ADC_InitStruct.ADC_SchIndex[0]      = EXT_SINGLE_ENDED(ADC_SAMPLE_CHANNEL_0);
    /* Set the bitmap corresponding to schedule0*/
    ADC_InitStruct.ADC_Bitmap           = 0x01;
#if ADC_CONFIG_GDMA_MODE_EN
    ADC_InitStruct.ADC_WaterLevel       = 4;
#endif
    ADC_InitStruct.ADC_DataWriteToFifo  = ENABLE;

    /* When this parameter is set to ENABLE: Power consumption will increase in this mode,
      but the ADC sampling value will be more accurate. (When ADC_PowerOnDlyEn is set to ENABLE,
      the ADC samples are accurate; When ADC_PowerOnDlyEn is set to DISABLE, the ADC samples are accurate after 8ms error).
      When this parameter is set to DISABLE: the power consumption is low in this mode,
      but the sampling value of ADC will always have a high error of 0-10mV. */
    ADC_InitStruct.ADC_PowerAlwaysOnEn  = ENABLE;
    /* Fixed 255 in OneShot mode. */
    ADC_InitStruct.ADC_SampleTime       = ADC_CONTINUOUS_SAMPLE_PERIOD;
    ADC_InitStruct.ADC_FifoThdLevel = 16;

    ADC_Init(ADC, &ADC_InitStruct);

#if (ADC_MODE_DIVIDE_OR_BYPASS == ADC_BYPASS_MODE)
    /* High bypass resistance mode config, please notice that the input voltage of
      adc channel using high bypass mode should not be over 0.9V */
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_0, ENABLE);
    DBG_DIRECT("[ADC]ADC sample mode is bypass mode !");
#else
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_0, DISABLE);
    DBG_DIRECT("[ADC]ADC sample mode is divide mode !");
#endif

#if ADC_CONFIG_GDMA_MODE_EN
    ADC_INTConfig(ADC, ADC_INT_FIFO_RD_ERR, ENABLE);
    ADC_INTConfig(ADC, ADC_INT_FIFO_OVERFLOW, ENABLE);
#else
    ADC_INTConfig(ADC, ADC_INT_FIFO_THD, ENABLE);
#endif

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = SAR_ADC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

#if ADC_CONFIG_GDMA_MODE_EN
void driver_gdma_adc_init(void)
{
    /* GDMA init */
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);

    GDMA_InitStruct.GDMA_ChannelNum         = ADC_GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR                = GDMA_DIR_PeripheralToMemory;
    GDMA_InitStruct.GDMA_BufferSize         = GDMA_TRANSFER_SIZE;//Determine total transfer size
    GDMA_InitStruct.GDMA_SourceInc          = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc     = DMA_DestinationInc_Inc;
    GDMA_InitStruct.GDMA_SourceDataSize     = GDMA_DataSize_HalfWord;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_HalfWord;
    GDMA_InitStruct.GDMA_SourceMsize        = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_DestinationMsize   = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_SourceAddr         = (uint32_t)(&(ADC->ADC_FIFO_READ));
    GDMA_InitStruct.GDMA_DestinationAddr    = (uint32_t)ADC_Recv_Buffer;
    GDMA_InitStruct.GDMA_SourceHandshake    = GDMA_Handshake_ADC_RX;


    GDMA_Init(ADC_GDMA_Channel, &GDMA_InitStruct);

    GDMA_INTConfig(ADC_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    /* GDMA irq init */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = ADC_GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

}
#endif

/**
  * @brief  Demo code of operation about adc.
  * @param  No parameter.
  * @return void
  */
void adc_demo(void)
{
    global_adc_init();

    /* Configure pad and pinmux firstly! */
    board_adc_init();

    /* Initialize adc peripheral */
    driver_adc_init();
#if ADC_CONFIG_GDMA_MODE_EN
    driver_gdma_adc_init();

    GDMA_Cmd(ADC_GDMA_CHANNEL_NUM, ENABLE);
#endif
    /* Clear adc fifo */
    ADC_ClearFIFO(ADC);

    /* Enable adc */
    ADC_Cmd(ADC, ADC_CONTINUOUS_MODE, ENABLE);
}

/**
  * @brief  ADC interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void SAR_ADC_Handler(void)
{
#if ADC_CONFIG_GDMA_MODE_EN
    if (ADC_GetINTStatus(ADC, ADC_INT_FIFO_RD_ERR) == SET)
    {
        DBG_DIRECT("ADC_INT_FIFO_RD_ERR!");
        ADC_ClearINTPendingBit(ADC, ADC_INT_FIFO_RD_ERR);
    }
    if (ADC_GetINTStatus(ADC, ADC_INT_FIFO_OVERFLOW) == SET)
    {
        ADC_WriteFIFOCmd(ADC, DISABLE);
        DBG_DIRECT("ADC_INT_FIFO_OVERFLOW");

        ADC_ClearFIFO(ADC);
        ADC_WriteFIFOCmd(ADC, ENABLE);
        ADC_ClearINTPendingBit(ADC, ADC_INT_FIFO_OVERFLOW);
    }
#else
    DBG_DIRECT("into SAR_ADC_Handler");
    uint8_t data_len = 0;
    uint16_t sample_data[ADC_FIFO_MAX] = {0};
    float sample_voltage[ADC_FIFO_MAX] = {0};
    ADC_ErrorStatus error_status = NO_ERROR;

    if (ADC_GetINTStatus(ADC, ADC_INT_FIFO_THD) == SET)
    {
        ADC_Cmd(ADC, ADC_CONTINUOUS_MODE, DISABLE);

        data_len = ADC_GetFIFODataLen(ADC);
        ADC_ReadFIFOData(ADC, sample_data, data_len);

        ADC_ClearFIFO(ADC);
        ADC_ClearINTPendingBit(ADC, ADC_INT_FIFO_THD);

        DBG_DIRECT("data_len = %d", data_len);
        for (uint8_t i = 0; i < data_len; i++)
        {
            sample_voltage[i] = ADC_GetVoltage(DIVIDE_SINGLE_MODE, (int32_t)sample_data[i], &error_status);

            if (error_status < 0)
            {
                DBG_DIRECT("[io_adc] io_adc_voltage_calculate: ADC parameter or efuse data error! i = %d, error_status = %d",
                           i, error_status);
            }
            else
            {
                DBG_DIRECT("[io_adc] io_adc_voltage_calculate: ADC rawdata_%d = %d, voltage_%d = %dmV ", i,
                           sample_data[i], i, (uint32_t)sample_voltage[i]);

            }
        }
    }
    for (uint32_t i = 0; i < 1000000; i++) {};
    ADC_Cmd(ADC, ADC_CONTINUOUS_MODE, ENABLE);
#endif
}

#if ADC_CONFIG_GDMA_MODE_EN
/**
  * @brief  GDMA channel interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void ADC_GDMA_Channel_Handler(void)
{
    ADC_Cmd(ADC, ADC_CONTINUOUS_MODE, DISABLE);
    DBG_DIRECT("into ADC_GDMA_Channel_Handler");
    float sample_voltage = 0;
    ADC_ErrorStatus error_status = NO_ERROR;

    for (uint32_t i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        uint16_t sample_data = ADC_Recv_Buffer[i];
        sample_voltage = ADC_GetVoltage(DIVIDE_SINGLE_MODE, (int32_t)sample_data, &error_status);
        if (error_status < 0)
        {
            DBG_DIRECT("[io_adc]io_adc_voltage_calculate: ADC parameter or efuse data error! i = %d, error_status = %d",
                       i, error_status);
        }
        else
        {
            DBG_DIRECT("[io_adc]io_adc_voltage_calculate: ADC rawdata_%d = %d, voltage_%d = %dmV ", i,
                       sample_data, i, (uint32_t)sample_voltage);
        }
    }

    platform_delay_ms(1000);

    /* Restart dma and adc */
    GDMA_SetSourceAddress(ADC_GDMA_Channel, (uint32_t)(&(ADC->ADC_FIFO_READ)));
    GDMA_SetDestinationAddress(ADC_GDMA_Channel, (uint32_t)(ADC_Recv_Buffer));
    GDMA_SetBufferSize(ADC_GDMA_Channel, GDMA_TRANSFER_SIZE);
    GDMA_ClearINTPendingBit(ADC_GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
    GDMA_Cmd(ADC_GDMA_CHANNEL_NUM, ENABLE);

    ADC_ClearFIFO(ADC);
    ADC_Cmd(ADC, ADC_CONTINUOUS_MODE, ENABLE);

}
#endif
