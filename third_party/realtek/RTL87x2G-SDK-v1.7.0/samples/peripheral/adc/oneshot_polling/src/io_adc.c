/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "rtl_rcc.h"
#include "rtl_pinmux.h"
#include "trace.h"

#include "rtl_adc.h"
#include "io_adc.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* ADC Demo Mode*/
#define ADC_DEMO_POLLING                    0
#define ADC_DEMO_AVERAGE                    1
/*Change ADC Demo Mode Here!*/
#define ADC_DEMO_MODE                       ADC_DEMO_POLLING

/* ADC bypass mode or divide mode */
#define ADC_DIVIDE_MODE                     1
#define ADC_BYPASS_MODE                     0
/* Change the ADC sampling mode here! */
#define ADC_MODE_DIVIDE_OR_BYPASS           ADC_DIVIDE_MODE

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
    ADC_InitStruct.ADC_SampleTime       = 255;

#if (ADC_DEMO_MODE == ADC_DEMO_POLLING)
    for (uint8_t i = 0; i < 16; i++)
    {
        ADC_InitStruct.ADC_SchIndex[i]  = EXT_SINGLE_ENDED(ADC_SAMPLE_CHANNEL);
    }
    ADC_InitStruct.ADC_Bitmap           = 0xFFFF;
#elif (ADC_DEMO_MODE == ADC_DEMO_AVERAGE)
    ADC_InitStruct.ADC_SchIndex[0]      = EXT_SINGLE_ENDED(ADC_SAMPLE_CHANNEL);
    ADC_InitStruct.ADC_DataAvgEn        = ENABLE;
    ADC_InitStruct.ADC_DataAvgSel       = ADC_DATA_AVERAGE_OF_4;
    ADC_InitStruct.ADC_Bitmap           = 0x01;
#endif

    /* When this parameter is set to ENABLE: Power consumption will increase in this mode,
      but the ADC sampling value will be more accurate. (When ADC_PowerOnDlyEn is set to ENABLE,
      the ADC samples are accurate; When ADC_PowerOnDlyEn is set to DISABLE, the ADC samples are accurate after 8ms error).
      When this parameter is set to DISABLE: the power consumption is low in this mode,
      but the sampling value of ADC will always have a high error of 0-10mV. */
    ADC_InitStruct.ADC_PowerAlwaysOnEn  = ENABLE;
    ADC_Init(ADC, &ADC_InitStruct);

#if (ADC_MODE_DIVIDE_OR_BYPASS == ADC_BYPASS_MODE)
    DBG_DIRECT("[ADC]ADC sample mode is bypass mode !");
    /* High bypass resistance mode config, please notice that the input voltage of
      adc channel using high bypass mode should not be over 0.9V */
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL, ENABLE);
#else
    DBG_DIRECT("[ADC]ADC sample mode is divide mode !");
#endif
    ADC_INTConfig(ADC, ADC_INT_ONE_SHOT_DONE, ENABLE);
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

    /* Enable adc */
    ADC_Cmd(ADC, ADC_ONE_SHOT_MODE, ENABLE);
}

/**
  * @brief  Calculate adc sample voltage.
  * @param  No parameter.
  * @return void
  */
/**
  * @brief  ADC polling sample demo code.
  * @param  No parameter.
  * @return void
  */
void adc_sample_demo(void)
{
    ADC_ErrorStatus error_status = NO_ERROR;
    while (ADC_GetINTStatus(ADC, ADC_INT_ONE_SHOT_DONE) == RESET) {}
    ADC_ClearINTPendingBit(ADC, ADC_INT_ONE_SHOT_DONE);

#if (ADC_DEMO_MODE == ADC_DEMO_POLLING)
    uint16_t sample_data[16] = {0};
    float sample_voltage[16] = {0};

    for (uint8_t i = 0; i < 16; i++)
    {
        sample_data[i] = ADC_ReadRawData(ADC, ADC_Schedule_Index_0 + i);
    }

    for (uint8_t i = 0; i < 16; i++)
    {
#if (ADC_MODE_DIVIDE_OR_BYPASS == ADC_BYPASS_MODE)
        sample_voltage[i] = ADC_GetVoltage(BYPASS_SINGLE_MODE, (int32_t)sample_data[i], &error_status);
#else
        sample_voltage[i] = ADC_GetVoltage(DIVIDE_SINGLE_MODE, (int32_t)sample_data[i], &error_status);
#endif
        if (error_status < 0)
        {
            DBG_DIRECT("[ADC]adc_sample_demo: ADC parameter or efuse data error! i = %d, error_status = %d", i,
                       error_status);
        }
        else
        {
            DBG_DIRECT("[ADC] adc_sample_demo: ADC one shot mode sample data_%d = %d, voltage_%d = %dmV ", i,
                       sample_data[i], i, (uint32_t)sample_voltage[i]);
        }
    }
#elif (ADC_DEMO_MODE == ADC_DEMO_AVERAGE)
    uint16_t sample_data = 0;
    float sample_voltage = 0;
    uint16_t sample_data_integer = 0;
    sample_data = ADC_ReadRawData(ADC, ADC_Schedule_Index_0);
    sample_data_integer = (sample_data & 0x3FFC) >> 2;
    sample_voltage = ADC_GetVoltage(DIVIDE_SINGLE_MODE, (int32_t)sample_data_integer, &error_status);
    if (error_status < 0)
    {
        DBG_DIRECT("[ADC]adc_sample_demo: ADC parameter or efuse data error!, error_status = %d",
                   error_status);
    }
    else
    {
        DBG_DIRECT("[ADC] adc_sample_demo: ADC sample data_integer = %d, voltage = %dmV ",
                   sample_data_integer, (uint32_t)sample_voltage);
    }
#endif
}
