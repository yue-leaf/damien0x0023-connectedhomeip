/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "io_lpc.h"
#include "pm.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
POWER_CheckResult IO_LPC_DLPS_Enter_Allowed = POWER_CHECK_PASS;
static uint32_t allow_count = 0;

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  IO enter dlps check function.
  * @param  No parameter.
  * @return void
  */
POWER_CheckResult io_lpc_dlps_check(void)
{

    return IO_LPC_DLPS_Enter_Allowed;
}

/**
  * @brief  IO enter dlps call back function.
  * @param  No parameter.
  * @return void
  */
void io_lpc_dlps_enter(void)
{
    DBG_DIRECT("DLPS ENTER");

    LPC_Cmd(LPC0, ENABLE);
}

/**
  * @brief  IO exit dlps call back function.
  * @param  No parameter.
  * @return void
  */
void io_lpc_dlps_exit(void)
{
    allow_count ++;
    DBG_DIRECT("DLPS EXIT, wake up reason 0x%x", power_get_wakeup_reason());
}

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_lpc_init(void)
{
    Pad_Config(P2_4, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_HIGH);
    Pinmux_Config(P2_4, IDLE_MODE);
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
    LPC_WKCmd(LPC0, ENABLE);

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
//        LPC_INTConfig(LPC0, LPC_INT_LPCOMP, DISABLE);
        DBG_DIRECT("LPC COMP");
        LPC_ClearFlag(LPC0, LPC_FLAG_LPCOMP);

    }

    if (LPC_GetFlagStatus(LPC0, LPC_FLAG_LPCOMP_AON) == SET)
    {
        DBG_DIRECT("LPC AON");
    }

    LPC_Cmd(LPC0, DISABLE);
}



