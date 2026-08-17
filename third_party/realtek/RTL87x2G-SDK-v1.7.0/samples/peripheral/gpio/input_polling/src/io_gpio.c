/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "rtl_rcc.h"
#include "rtl_pinmux.h"
#include "rtl_gpio.h"
#include "trace.h"
#include "utils.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define INPUT_PIN               P1_0
#define GPIO_PIN                GPIO_GetPin(INPUT_PIN)
#define GPIO_PORT               GPIO_GetPort(INPUT_PIN)
#define APBPERIPH_GPIO          APBPeriph_GPIOA
#define APBPERIPH_GPIO_CLOCK    APBPeriph_GPIOA_CLOCK

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  None.
  * @retval None.
  */
void board_gpio_init(void)
{
    Pad_Config(INPUT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);

    Pinmux_Config(INPUT_PIN, DWGPIO);
}
/**
  * @brief  Initialize GPIO peripheral.
  * @param  None.
  * @retval None.
  */
void driver_gpio_init(void)
{
    /* GPIO peripheral clock enable */
    RCC_PeriphClockCmd(APBPERIPH_GPIO, APBPERIPH_GPIO_CLOCK, ENABLE);

    /* Configure GPIO_PIN_OUTPUT as output mode */
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode   = GPIO_DIR_IN;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(GPIO_PORT, &GPIO_InitStruct);
}

void gpio_demo(void)
{
    /* Polling GPIO input value. Add user code, e.g. print info. */
    while (1)
    {
        /* Read GPIO_PIN input value: gpio_input_data */
        uint8_t gpio_input_data = GPIO_ReadInputDataBit(GPIO_PORT, GPIO_PIN);

        /** User code here.
          * for example: print the value of gpio_input_data; */
        DBG_DIRECT("gpio_input_data = %d", gpio_input_data);
        platform_delay_ms(2000);
    }
}




