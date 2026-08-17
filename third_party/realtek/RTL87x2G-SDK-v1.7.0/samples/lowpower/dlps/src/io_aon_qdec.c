/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "rtl_rcc.h"
#include "rtl_nvic.h"
#include "rtl_pinmux.h"
#include "trace.h"
#include "pm.h"
#include "rtl_aon_qdec.h"
#include "io_aon_qdec.h"

/* Private typedef -----------------------------------------------------------*/
/* phase A */
#define QDEC_X_PHA_PIN          P1_3
/* phase B */
#define QDEC_X_PHB_PIN          P1_4

/* Private define ------------------------------------------------------------*/

/* Globals -------------------------------------------------------------------*/
QDEC_Data_TypeDef   X_Axis_Data;

/* Private function prototypes -----------------------------------------------*/
POWER_CheckResult IO_AON_QDEC_DLPS_Enter_Allowed = POWER_CHECK_PASS;
static uint32_t allow_count = 0;
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  IO enter dlps check function.
  * @param  No parameter.
  * @return void
  */
POWER_CheckResult io_aon_qdec_dlps_check(void)
{
    if (allow_count >= 20)
    {
        IO_AON_QDEC_DLPS_Enter_Allowed = POWER_CHECK_FAIL;
    }
    return IO_AON_QDEC_DLPS_Enter_Allowed;
}

/**
  * @brief  IO enter dlps call back function.
  * @param  No parameter.
  * @return void
  */
void io_aon_qdec_dlps_enter(void)
{
    DBG_DIRECT("DLPS ENTER");

}

/**
  * @brief  IO exit dlps call back function.
  * @param  No parameter.
  * @return void
  */
void io_aon_qdec_dlps_exit(void)
{
    allow_count ++;
    DBG_DIRECT("DLPS EXIT, wake up reason 0x%x", power_get_wakeup_reason());
}


/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return Void
  */
void board_qdec_init(void)
{
    /* AON QDEC pad config */
    Pad_Config(QDEC_X_PHA_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(QDEC_X_PHB_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);

    /* AON QDEC pinmux config */
    Pinmux_AON_Config(QDPH0_IN_P1_3_P1_4);
}

/**
  * @brief  Initialize Qdecoder peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_qdec_init(void)
{

    AON_QDEC_InitTypeDef QDEC_InitStruct;
    AON_QDEC_StructInit(&QDEC_InitStruct);
    QDEC_InitStruct.axisConfigX       = ENABLE;
    QDEC_InitStruct.debounceEnableX   = ENABLE;
    QDEC_InitStruct.manualLoadInitPhase = DISABLE;
    AON_QDEC_Init(AON_QDEC, &QDEC_InitStruct);

    AON_QDEC_INTMask(AON_QDEC, AON_QDEC_X_INT_MASK, DISABLE);
    AON_QDEC_INTMask(AON_QDEC, AON_QDEC_X_CT_INT_MASK, DISABLE);
    AON_QDEC_INTMask(AON_QDEC, AON_QDEC_X_ILLEAGE_INT_MASK, DISABLE);
    AON_QDEC_INTMask(AON_QDEC, AON_QDEC_X_WAKE_AON_MASK, DISABLE);

    AON_QDEC_INTConfig(AON_QDEC, AON_QDEC_X_INT_NEW_DATA, ENABLE);
    AON_QDEC_INTConfig(AON_QDEC, AON_QDEC_X_INT_ILLEAGE, ENABLE);

    AON_QDEC_Cmd(AON_QDEC, AON_QDEC_AXIS_X, ENABLE);

    NVIC_InitTypeDef nvic_init_struct;
    nvic_init_struct.NVIC_IRQChannel         = AON_QDEC_IRQn;
    nvic_init_struct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    nvic_init_struct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&nvic_init_struct);

}

/**
  * @brief  Qdecode interrupt handler function.
  * @param  No parameter.
  * @return Void
  */
void AON_QDEC_Handler(void)
{
    DBG_DIRECT("Enter AON_QDEC Handler");
    if (AON_QDEC_GetFlagState(AON_QDEC, AON_QDEC_FLAG_NEW_CT_STATUS_X) == SET)
    {

        /* Read direction & count */
        X_Axis_Data.AxisDirection = AON_QDEC_GetAxisDirection(AON_QDEC, AON_QDEC_AXIS_X);
        X_Axis_Data.AxisCount = AON_QDEC_GetAxisCount(AON_QDEC, AON_QDEC_AXIS_X);

        DBG_DIRECT("Qdecode_Handler: X_Axis_Direction = %d, X_Axis_Count = %d",
                   X_Axis_Data.AxisDirection, X_Axis_Data.AxisCount);

        /* Clear qdec interrupt flags */
        AON_QDEC_ClearINTPendingBit(AON_QDEC, AON_QDEC_CLR_NEW_CT_X);

    }
    if (AON_QDEC_GetFlagState(AON_QDEC, AON_QDEC_FLAG_ILLEGAL_STATUS_X) == SET)
    {
        DBG_DIRECT("Before clear, illegal_INT_STA is %d\r\n", AON_QDEC_GetFlagState(AON_QDEC,
                                                                                    AON_QDEC_FLAG_ILLEGAL_STATUS_X));

        AON_QDEC_ClearINTPendingBit(AON_QDEC, AON_QDEC_CLR_ILLEGAL_INT_X);
        AON_QDEC_ClearINTPendingBit(AON_QDEC, AON_QDEC_CLR_ILLEGAL_CT_X);

        DBG_DIRECT("After clear, illegal_INT_STA is %d\r\n", AON_QDEC_GetFlagState(AON_QDEC,
                                                                                   AON_QDEC_FLAG_ILLEGAL_STATUS_X));
    }
}

