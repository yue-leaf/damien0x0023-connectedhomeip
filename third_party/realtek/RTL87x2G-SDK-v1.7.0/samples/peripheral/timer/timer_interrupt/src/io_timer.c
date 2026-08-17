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

#include "rtl_tim.h"
#include "rtl_gpio.h"
#include "io_timer.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define TIMER_NUM       TIM2
#define TIMER_IRQN      Timer2_IRQn
#define TIMER_Handler   Timer2_Handler

/* Timer timing config */
#define TIME_UINT       40 /* 40M source clock is divided to get unit: 1us */
#define TIME_TIMING     1000000
#define TIMER_PERIOD    (TIME_TIMING * TIME_UINT - 1)

/* P0_0 is connected to LED0 on EVB board */
#define OUTPUT_PIN          P0_0
#define GPIO_PIN            GPIO_GetPin(OUTPUT_PIN)
#define GPIO_PORT           GPIO_GetPort(OUTPUT_PIN)

/* Gloables define -----------------------------------------------------------*/
uint8_t LED_Status = 0;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_gpio_init(void)
{
    Pad_Config(OUTPUT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    Pinmux_Config(OUTPUT_PIN, DWGPIO);
}

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
  * @brief  Initialize TIMER peripheral.
  * @param  None.
  * @retval None.
  */
void driver_timer_init(void)
{
    /* Enable TIMER peripheral clock */
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);

    /* Configure TIMER init parameters */
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);
    TIM_InitStruct.TIM_PWM_En = DISABLE;
    TIM_InitStruct.TIM_Period = TIMER_PERIOD ;
    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_TimeBaseInit(TIMER_NUM, &TIM_InitStruct);

    /* Enable NVIC */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIMER_IRQN;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    TIM_ClearINT(TIMER_NUM);
    TIM_INTConfig(TIMER_NUM, ENABLE);
    TIM_Cmd(TIMER_NUM, ENABLE);

}

void TIMER_Handler(void)
{
    TIM_ClearINT(TIMER_NUM);
    TIM_Cmd(TIMER_NUM, DISABLE);
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
    TIM_Cmd(TIMER_NUM, ENABLE);
}


