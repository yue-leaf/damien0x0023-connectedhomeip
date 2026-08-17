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

#include "rtl_enh_tim.h"
#include "rtl_gpio.h"
#include "io_enhtim_interrupt.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Gloables define -----------------------------------------------------------*/
uint8_t LED_Status = 0;
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return None.
*/
void board_gpio_init(void)
{
    Pad_Config(OUTPUT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    Pinmux_Config(OUTPUT_PIN, DWGPIO);
}

/**
  * @brief  Initialize GPIO peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_gpio_init(void)
{
    /* GPIO peripheral clock enable */
    RCC_PeriphClockCmd(APBPeriph_GPIOA, APBPeriph_GPIOA_CLOCK, ENABLE);

    /* Configure GPIO_PIN_OUTPUT as output mode */
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_PIN;
    GPIO_InitStruct.GPIO_Dir    = GPIO_DIR_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(GPIO_PORT, &GPIO_InitStruct);
}

/**
  * @brief  Initialize TIM-Enhance peripheral.
  * @param  No parameter.
  * @return None.
*/
void driver_enhtim_init(void)
{
    /* Enable ENHTIM peripheral clock */
    RCC_PeriphClockCmd(APBPeriph_ENHTIMER, APBPeriph_ENHTIMER_CLOCK, ENABLE);

    ENHTIM_InitTypeDef ENHTIM_InitStruct;
    ENHTIM_StructInit(&ENHTIM_InitStruct);

    ENHTIM_InitStruct.ENHTIM_ClockDiv               = ENHTIM_CLOCK_DIVIDER_1;
    ENHTIM_InitStruct.ENHTIM_PWMOutputEn            = DISABLE;
    ENHTIM_InitStruct.ENHTIM_MaxCount               = ENHTIM_PERIOD;
    ENHTIM_InitStruct.ENHTIM_Mode                   = ENHTIM_MODE_PWM_MANUAL;
    ENHTIM_Init(ENHTIMER_NUM, &ENHTIM_InitStruct);

    /*  Enable TIMER IRQ  */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = ENHTIMER_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    ENHTIM_ClearINTPendingBit(ENHTIMER_NUM, ENHTIM_INT_TIM);
    ENHTIM_INTConfig(ENHTIMER_NUM, ENHTIM_INT_TIM, ENABLE);
    ENHTIM_Cmd(ENHTIMER_NUM, ENABLE);
}


/**
  * @brief  Enhance timer handler.
  * @param  None.
  * @return None.
  */
void Enhanced_Timer0_Handler()
{
    ENHTIM_ClearINTPendingBit(ENHTIMER_NUM, ENHTIM_INT_TIM);
    ENHTIM_Cmd(ENHTIMER_NUM, DISABLE);
    if (!LED_Status)
    {
        GPIO_WriteBit(GPIO_PORT, GPIO_PIN, (BitAction)(1));
        LED_Status = 1;
    }
    else
    {
        GPIO_WriteBit(GPIO_PORT, GPIO_PIN, (BitAction)(0));
        LED_Status = 0;
    }
    //Add user code here
    ENHTIM_Cmd(ENHTIMER_NUM, ENABLE);
}

