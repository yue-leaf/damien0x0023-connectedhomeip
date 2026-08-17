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
#include "io_latch_gpio.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Gloables define -----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialize TIM-Enhance peripheral.
  * @param  No parameter.
  * @return None.
*/
void driver_enhance_timer_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_ENHTIMER, APBPeriph_ENHTIMER_CLOCK, ENABLE);

    ENHTIM_InitTypeDef ENHTIM_InitStruct;
    ENHTIM_StructInit(&ENHTIM_InitStruct);

    ENHTIM_InitStruct.ENHTIM_ClockDiv               = ENHTIM_CLOCK_DIVIDER_1;
    ENHTIM_InitStruct.ENHTIM_Mode                   = ENHTIM_MODE_FreeRun;
    /* Only enhtim_ ENHTIM_LatchCountEn[0] latch triggered by GPIO. */
    ENHTIM_InitStruct.ENHTIM_LatchCountEn[0]        = ENABLE;
    ENHTIM_InitStruct.ENHTIM_LatchCountTrigger[0]   = ENHTIM_LATCH_TRIGGER_RISING_EDGE;
    ENHTIM_InitStruct.ENHTIM_LatchCountThd          = 3;
    ENHTIM_InitStruct.ENHTIM_LatchTriggerPad        = INPUT_PIN;

    ENHTIM_Init(ENHTIMER_NUM, &ENHTIM_InitStruct);

    /*  Enable TIMER IRQ  */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = ENHTIMER_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    ENHTIM_ClearINTPendingBit(ENHTIMER_NUM, ENHTIM_INT_TIM);
    ENHTIM_INTConfig(ENHTIMER_NUM, ENHTIM_INT_TIM, ENABLE);

    ENHTIM_ClearINTPendingBit(ENHTIMER_NUM, ENHTIM_INT_LATCH_CNT_FIFO_FULL);
    ENHTIM_INTConfig(ENHTIMER_NUM, ENHTIM_INT_LATCH_CNT_FIFO_FULL, ENABLE);

    ENHTIM_ClearINTPendingBit(ENHTIMER_NUM, ENHTIM_INT_LATCH_CNT_FIFO_THD);
    ENHTIM_INTConfig(ENHTIMER_NUM, ENHTIM_INT_LATCH_CNT_FIFO_THD, ENABLE);

    ENHTIM_Cmd(ENHTIMER_NUM, ENABLE);
}

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
    /* Initialize GPIO peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIOA, APBPeriph_GPIOA_CLOCK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin  = GPIO_PIN_OUTPUT;
    GPIO_InitStruct.GPIO_Dir  = GPIO_DIR_OUT;
    GPIO_Init(GPIO_PORT_OUTPUT, &GPIO_InitStruct);

    GPIO_WriteBit(GPIO_PORT_OUTPUT, GPIO_PIN_OUTPUT, (BitAction)(0));
}

/**
  * @brief  Enhance timer handler.
  * @param  None.
  * @return None.
  */
void Enhanced_Timer0_Handler()
{
    if (ENHTIM_GetINTStatus(ENH_TIM0, ENHTIM_INT_TIM))
    {
        APP_PRINT_INFO0("ENH_TIM0 ENHTIM_INT_TIM\r\n");
        ENHTIM_ClearINTPendingBit(ENH_TIM0, ENHTIM_INT_TIM);
    }
    if (ENHTIM_GetINTStatus(ENH_TIM0, ENHTIM_INT_LATCH_CNT_FIFO_FULL))
    {
        APP_PRINT_INFO0("ENH_TIM0 ENHTIM_INT_LATCH_CNT2_FIFO_FULL\r\n");
        ENHTIM_ClearINTPendingBit(ENH_TIM0, ENHTIM_INT_LATCH_CNT_FIFO_FULL);
    }
    if (ENHTIM_GetINTStatus(ENH_TIM0, ENHTIM_INT_LATCH_CNT_FIFO_THD))
    {
        APP_PRINT_INFO0("ENH_TIM0 ENHTIM_INT_LATCH_CNT2_FIFO_THD\r\n");
        ENHTIM_INTConfig(ENHTIMER_NUM, ENHTIM_INT_LATCH_CNT_FIFO_THD, DISABLE);
        uint8_t length = ENHTIM_GetLatchCountFIFOLength(ENH_TIM0);
        uint32_t data[4] = {0};
        ENHTIM_ReadLatchCountFIFO(ENH_TIM0, data, length);
        /* Only for debugging, removed in actual application. */
        APP_PRINT_INFO1("ENH_TIM0 fifo length = %d\r\n", length);
        for (uint8_t i = 0; i < length; i++)
        {
            /* Only for debugging, removed in actual application. */
            APP_PRINT_INFO2("ENH_TIM0 data[%d] = 0x%x\r\n", i, data[i]);
        }

        ENHTIM_ClearINTPendingBit(ENH_TIM0, ENHTIM_INT_LATCH_CNT_FIFO_THD);
        ENHTIM_INTConfig(ENH_TIM0, ENHTIM_INT_LATCH_CNT_FIFO_THD, ENABLE);
    }
}

