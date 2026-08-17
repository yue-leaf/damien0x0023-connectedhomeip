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

#include "rtl_adc.h"
#include "io_adc.h"

/* Private define ------------------------------------------------------------*/

#define ADC_DATA_OUTPUT_TO_FIFO     0

/* ADC bypass mode or divide mode */
#define ADC_DIVIDE_MODE                     1
#define ADC_BYPASS_MODE                     0
/* Change the ADC sampling mode here! */
#define ADC_MODE_DIVIDE_OR_BYPASS           ADC_DIVIDE_MODE

#define ADC_SAMPLE_PIN                    P2_4
#define ADC_SAMPLE_CHANNEL                ADC_Channel_Index_4

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_adc_init(void)
{
    Pad_Config(ADC_SAMPLE_PIN, PAD_SW_MODE, PAD_NOT_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
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
    ADC_InitStruct.ADC_SchIndex[0]      = EXT_SINGLE_ENDED(ADC_SAMPLE_CHANNEL);
    /* Set the bitmap corresponding to schedule0*/
    ADC_InitStruct.ADC_Bitmap           = 0x01;

#if (ADC_DATA_OUTPUT_TO_FIFO)
    ADC_InitStruct.ADC_DataWriteToFifo  = ENABLE;
    ADC_InitStruct.ADC_FifoThdLevel     = 0x0A;
#endif

    /* When this parameter is set to ENABLE: Power consumption will increase in this mode,
      but the ADC sampling value will be more accurate. (When ADC_PowerOnDlyEn is set to ENABLE,
      the ADC samples are accurate; When ADC_PowerOnDlyEn is set to DISABLE, the ADC samples are accurate after 8ms error).
      When this parameter is set to DISABLE: the power consumption is low in this mode,
      but the sampling value of ADC will always have a high error of 0-10mV. */
    ADC_InitStruct.ADC_PowerAlwaysOnEn  = ENABLE;
    /* Fixed 255 in OneShot mode. */
    ADC_InitStruct.ADC_SampleTime       = 255;

    ADC_Init(ADC, &ADC_InitStruct);

#if (ADC_MODE_DIVIDE_OR_BYPASS == ADC_BYPASS_MODE)
    /* High bypass resistance mode config, please notice that the input voltage of
      adc channel using high bypass mode should not be over 0.9V */
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_0, ENABLE);
    DBG_DIRECT("[io_adc] driver_adc_init: ADC sample mode is bypass mode !");
#else
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL, DISABLE);
    DBG_DIRECT("[io_adc] driver_adc_init: ADC sample mode is divide mode !");
#endif

#if (!ADC_DATA_OUTPUT_TO_FIFO)
    ADC_INTConfig(ADC, ADC_INT_ONE_SHOT_DONE, ENABLE);
#else
    ADC_INTConfig(ADC, ADC_INT_FIFO_THD, ENABLE);
#endif

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = SAR_ADC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Demo code of operation about adc.
  * @param  No parameter.
  * @return void
  */
void adc_demo(void)
{
    /* Initialize adc k value! */
    bool adc_k_status;
    adc_k_status = ADC_CalibrationInit();
    if (false == adc_k_status)
    {
        DBG_DIRECT("[ADC]ADC_CalibrationInit fail!");
    }

    /* Configure pad and pinmux firstly! */
    board_adc_init();

    /* Initialize adc peripheral */
    driver_adc_init();

#if (ADC_DATA_OUTPUT_TO_FIFO)
    ADC_ClearFIFO(ADC);
#endif

    /* Enable adc */
    while (1)
    {
        ADC_Cmd(ADC, ADC_ONE_SHOT_MODE, ENABLE);
        for (int i = 0; i < 100000; i++);
    }

}

/**
  * @brief  ADC interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void SAR_ADC_Handler(void)
{
    DBG_DIRECT("into SAR_ADC_Handler");
    ADC_ErrorStatus error_status = NO_ERROR;

#if (!ADC_DATA_OUTPUT_TO_FIFO)
    uint16_t sample_data = 0;
    float sample_voltage = 0;

    if (ADC_GetINTStatus(ADC, ADC_INT_ONE_SHOT_DONE) == SET)
    {
        ADC_ClearINTPendingBit(ADC, ADC_INT_ONE_SHOT_DONE);

        sample_data = ADC_ReadRawData(ADC, ADC_Schedule_Index_0);
#if (ADC_MODE_DIVIDE_OR_BYPASS == ADC_BYPASS_MODE)
        sample_voltage = ADC_GetVoltage(BYPASS_SINGLE_MODE, (int32_t)sample_data, &error_status);
#else
        sample_voltage = ADC_GetVoltage(DIVIDE_SINGLE_MODE, (int32_t)sample_data, &error_status);
#endif
        if (error_status < 0)
        {
            DBG_DIRECT("[ADC]adc_sample_demo: ADC parameter or efuse data error!, error_status = %d",
                       error_status);
        }
        else
        {
            DBG_DIRECT("[ADC] adc_sample_demo: ADC sample data = %d, voltage = %dmV ",
                       sample_data, (uint32_t)sample_voltage);
        }
    }
#else
    if (ADC_GetINTStatus(ADC, ADC_INT_FIFO_THD) == SET)
    {
        uint16_t sample_data[32] = {0};
        float sample_voltage[32] = {0};
        uint16_t data_len = ADC_GetFIFODataLen(ADC);
        ADC_ReadFIFOData(ADC, sample_data, data_len);
        DBG_DIRECT("ADC_INT_FIFO_THD: len = %d", data_len);
        for (uint8_t i = 0; i < data_len; i++)
        {
#if (ADC_MODE_DIVIDE_OR_BYPASS == ADC_BYPASS_MODE)
            sample_voltage[i] = ADC_GetVoltage(BYPASS_SINGLE_MODE, (int32_t)sample_data[i], &error_status);
#else
            sample_voltage[i] = ADC_GetVoltage(DIVIDE_SINGLE_MODE, (int32_t)sample_data[i], &error_status);
#endif
            if (error_status < 0)
            {
                DBG_DIRECT("[ADC]adc_sample_demo: ADC parameter or efuse data error!, error_status = %d",
                           error_status);
            }
            else
            {
                DBG_DIRECT("[ADC] adc_sample_demo: ADC sample data_%d = %d, voltage_%d = %dmV ", i,
                           sample_data[i], i, (uint32_t)sample_voltage[i]);
            }
        }
        ADC_ClearINTPendingBit(ADC, ADC_INT_FIFO_THD);
        ADC_ClearFIFO(ADC);
    }
#endif
}
