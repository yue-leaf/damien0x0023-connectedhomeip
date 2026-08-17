/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "io_lpc.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Globals ------------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_lpc_init(void)
{
    Pad_Config(LPC_TEST_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_HIGH);
    Pinmux_Config(LPC_TEST_PIN, IDLE_MODE);
}

/**
  * @brief  Initialize LPC peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_lpc_init(void)
{
    LPC_DeInit(LPC0);
    LPC_InitTypeDef LPC_InitStruct;
    LPC_StructInit(&LPC_InitStruct);

    LPC_InitStruct.LPC_Channel    = LPC_CAPTURE_CHANNEL;
    LPC_InitStruct.LPC_Edge       = LPC_VOLTAGE_DETECT_EDGE;
    LPC_InitStruct.LPC_Threshold  = LPC_1000_mV;
    LPC_Init(LPC0, &LPC_InitStruct);
    LPC_INTConfig(LPC0, LPC_INT_LPCOMP, ENABLE);
    LPC_Cmd(LPC0, ENABLE);

    /* Config LPC interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = LPCOMP_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}


/**
  * @brief  LPC battery detection interrupt handle function.
  * @param  None.
  * @return None.
  */
void LPCOMP_Handler(void)
{
    DBG_DIRECT("LPCOMP_Handler");

    //Add Application code here
    if (LPC_GetFlagStatus(LPC0, LPC_FLAG_LPCOMP) == SET)
    {
        LPC_INTConfig(LPC0, LPC_INT_LPCOMP, DISABLE);
        DBG_DIRECT("LPC COMP");
        LPC_ClearFlag(LPC0, LPC_FLAG_LPCOMP);

    }

    if (LPC_GetFlagStatus(LPC0, LPC_FLAG_LPCOMP_AON) == SET)
    {
        DBG_DIRECT("LPC AON");
    }

}



