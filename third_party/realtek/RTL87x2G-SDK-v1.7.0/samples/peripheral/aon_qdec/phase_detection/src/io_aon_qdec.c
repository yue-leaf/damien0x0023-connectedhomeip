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

#include "rtl_aon_qdec.h"
#include "io_aon_qdec.h"

/* Private typedef -----------------------------------------------------------*/
/* phase A */
#define QDEC_X_PHA_PIN          P9_0
/* phase B */
#define QDEC_X_PHB_PIN          P9_1

/* Private define ------------------------------------------------------------*/

/* Globals -------------------------------------------------------------------*/
QDEC_Data_TypeDef   X_Axis_Data;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
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
    Pinmux_AON_Config(QDPH0_IN_P9_0_P9_1);
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

