/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "io_adc.h"
#include "pm.h"
#include "board.h"
#include "trace.h"


/* Private define ------------------------------------------------------------*/


/* Globals ------------------------------------------------------------------*/
POWER_CheckResult IO_ADC_GPIO_DLPS_Enter_Allowed = POWER_CHECK_FAIL;

/**
  * @brief  Initialize uart global data.
  * @param  No parameter.
  * @return void
  */
void global_data_adc_gpio_init(void)
{
    IO_ADC_GPIO_DLPS_Enter_Allowed = POWER_CHECK_PASS;
}


/**
  * @brief  IO enter dlps check function.
  * @param  No parameter.
  * @return void
  */
POWER_CheckResult io_adc_gpio_dlps_check(void)
{
    return IO_ADC_GPIO_DLPS_Enter_Allowed;
}

/**
  * @brief  IO enter dlps call back function.
  * @param  No parameter.
  * @return void
  */
void io_adc_gpio_dlps_enter(void)
{
    /* Switch pad to Software mode */
    Pad_ControlSelectValue(ADC_GPIO_DLPS_WAKEUP_PIN, PAD_SW_MODE);
    System_WakeUpPinEnable(ADC_GPIO_DLPS_WAKEUP_PIN, PAD_WAKEUP_POL_LOW, 0);

    DBG_DIRECT("io_adc_dlps_enter");
}

/**
  * @brief  IO exit dlps call back function.
  * @param  No parameter.
  * @return void
  */
void io_adc_gpio_dlps_exit(void)
{
    /* Switch pad to Pinmux mode */
    Pad_ControlSelectValue(ADC_GPIO_DLPS_WAKEUP_PIN, PAD_PINMUX_MODE);
    DBG_DIRECT("io_adc_dlps_exit");

    DBG_DIRECT("STATUS = %d", GPIO_GetINTStatus(GPIOA, GPIO_PIN_INPUT));

}

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_adc_gpio_init(void)
{
    Pad_Config(ADC_GPIO_DLPS_WAKEUP_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_HIGH);

    Pinmux_Config(ADC_GPIO_DLPS_WAKEUP_PIN, DWGPIO);
}


/**
  * @brief  Initialize RTC peripheral.
  * @param   No parameter.
  * @return  void
  */
void driver_adc_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

    ADC_InitTypeDef ADC_InitStruct;
    ADC_StructInit(&ADC_InitStruct);

    ADC_InitStruct.ADC_SampleTime       = 255;  /* (n + 1) cycle of 10MHz,n = 0~255 or n = 2048~14591 */

    ADC_InitStruct.ADC_SchIndex[0]      = INTERNAL_VBAT_MODE;
    ADC_InitStruct.ADC_Bitmap           = 0x01;

    /* When this parameter is set to ENABLE: Power consumption will increase in this mode,
      but the ADC sampling value will be more accurate. (When ADC_PowerOnDlyEn is set to ENABLE,
      the ADC samples are accurate; When ADC_PowerOnDlyEn is set to DISABLE, the ADC samples are accurate after 8ms error).
      When this parameter is set to DISABLE: the power consumption is low in this mode,
      but the sampling value of ADC will always have a high error of 0-10mV. */
    ADC_InitStruct.ADC_PowerAlwaysOnEn  = ENABLE;
    ADC_Init(ADC, &ADC_InitStruct);

#if (ADC_MODE_DIVIDE_OR_BYPASS == ADC_BYPASS_MODE)
    /* High bypass resistance mode config, please notice that the input voltage of
      adc channel using high bypass mode should not be over 0.9V */
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_0, ENABLE);
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_1, ENABLE);
#endif

    ADC_INTConfig(ADC, ADC_INT_ONE_SHOT_DONE, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = SAR_ADC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Initialize GPIO peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_gpio_init(void)
{
    /* Initialize GPIO peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIOB, APBPeriph_GPIOB_CLOCK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin        = GPIO_PIN_INPUT;
    GPIO_InitStruct.GPIO_Mode       = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd      = ENABLE;
    GPIO_InitStruct.GPIO_ITTrigger  = GPIO_INT_Trigger_EDGE;
    GPIO_InitStruct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_LOW;

    /* Enable GPIO doubonce function, Set doubonce time is 2ms,
     * debounce time = (CntLimit + 1) * DEB_CLK = 64 / 32000 = 1 */
    GPIO_InitStruct.GPIO_ITDebounce = GPIO_INT_DEBOUNCE_ENABLE;
    GPIO_InitStruct.GPIO_DebounceClkSource = GPIO_DEBOUNCE_32K;
    GPIO_InitStruct.GPIO_DebounceClkDiv    = GPIO_DEBOUNCE_DIVIDER_1;
    GPIO_InitStruct.GPIO_DebounceCntLimit  = 32 - 1;
    GPIO_Init(GPIO_PORT, &GPIO_InitStruct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GPIO_PIN_INPUT_IRQN;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    GPIO_MaskINTConfig(GPIO_PORT, GPIO_PIN_INPUT, DISABLE);
    GPIO_INTConfig(GPIO_PORT, GPIO_PIN_INPUT, ENABLE);
}

/**
  * @brief  ADC interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void SAR_ADC_Handler(void)
{
    uint16_t sample_data;
    float sample_voltage = 0;
    ADC_ErrorStatus error_status = NO_ERROR;

    if (ADC_GetINTStatus(ADC, ADC_INT_ONE_SHOT_DONE) == SET)
    {
        ADC_ClearINTPendingBit(ADC, ADC_INT_ONE_SHOT_DONE);

        sample_data = ADC_ReadRawData(ADC, ADC_Schedule_Index_0);
        sample_voltage = ADC_GetVoltage(DIVIDE_SINGLE_MODE, (int32_t)sample_data, &error_status);

        if (error_status < 0)
        {
            DBG_DIRECT("[ADC]adc_sample_demo: ADC parameter or efuse data error!, error_status = %d",
                       error_status);
        }
        else
        {
            DBG_DIRECT("[ADC] adc_sample_demo: ADC one shot mode sample data = %d, voltage = %dmV ",
                       sample_data, (uint32_t)sample_voltage);
        }

    }
}

/**
  * @brief  GPIO interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void GPIO_Input_Handler(void)
{
    DBG_DIRECT("GPIO_Input_Handler");
    GPIO_INTConfig(GPIO_PORT, GPIO_PIN_INPUT, DISABLE);
    GPIO_MaskINTConfig(GPIO_PORT, GPIO_PIN_INPUT, ENABLE);

    ADC_Cmd(ADC, ADC_ONE_SHOT_MODE, ENABLE);

    GPIO_ClearINTPendingBit(GPIO_PORT, GPIO_PIN_INPUT);
    GPIO_MaskINTConfig(GPIO_PORT, GPIO_PIN_INPUT, DISABLE);
    GPIO_INTConfig(GPIO_PORT, GPIO_PIN_INPUT, ENABLE);
}

