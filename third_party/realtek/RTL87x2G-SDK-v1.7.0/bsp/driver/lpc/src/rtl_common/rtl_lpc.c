/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "rtl_lpc.h"

/*============================================================================*
 *                          Private Functions
 *============================================================================*/
extern void LPC_PowerOff(void);
extern void LPC_SISet(void);
extern void LPC_ChannelConfig(LPC_TypeDef *LPCx, LPC_InitTypeDef *LPC_InitStruct);
extern ITStatus LPC_GetINTStatusConfig(LPC_TypeDef *LPCx, uint32_t LPC_INT);
extern void LPC_ClearFlagConfig(LPC_TypeDef *LPCx, uint32_t LPC_FLAG) ;
extern void LPC_INTSetting(LPC_TypeDef *LPCx, uint32_t LPC_INT, FunctionalState NewState);
extern void LPC_CmdConfig(LPC_TypeDef *LPCx, FunctionalState NewState);
extern void LPC_PowerOn(LPC_InitTypeDef *LPC_InitStruct);

/*============================================================================*
 *                           Public Functions
 *============================================================================*/
/**
  * \brief  Reset LPC.
  * \param  LPCx: Select the LPC peripheral. \ref LPC_Declaration
  * \return None
  */
void LPC_DeInit(LPC_TypeDef *LPCx)
{
    LPC_CR0_TypeDef lpc_reg_0x00 = {.d32 = LPCx->LPC_CR};

    /* Stop LPC counter */
    lpc_reg_0x00.b.lpcomp0_ie = 0x0;
    lpc_reg_0x00.b.lpcomp0_single_output_en = 0x0;
    /* Disable int signal */
    lpc_reg_0x00.b.lpcomp0_src_int_en = 0x0;
    lpc_reg_0x00.b.lpcomp0_src_aon_int_en = 0x0;

    LPCx->LPC_CR = lpc_reg_0x00.d32;

#if LPC_SUPPORT_POWER_OFF
    LPC_PowerOff();
#endif
}

/**
  * \brief  Initializes LPC peripheral according to
  *    the specified parameters in the LPC_InitStruct.
  * \param  LPCx: Select the LPC peripheral. \ref LPC_Declaration
  * \param  LPC_InitStruct: pointer to a LPC_InitTypeDef
  *    structure that contains the configuration information for the
  *    specified LPC peripheral.
  * \return None
  */
void LPC_Init(LPC_TypeDef *LPCx, LPC_InitTypeDef *LPC_InitStruct)
{
    uint16_t reg_value = 0;

    /* Check the parameters */
    assert_param(IS_LPC_CHANNEL(LPC_InitStruct->LPC_Channel));
    assert_param(IS_LPC_EDGE(LPC_InitStruct->LPC_Edge));
    assert_param(IS_LPC_THRESHOLD(LPC_InitStruct->LPC_Threshold));

    LPC_CR0_TypeDef lpc_reg_0x00 = {.d32 = LPCx->LPC_CR};

    /* LPC single mode output */
    lpc_reg_0x00.b.lpcomp0_single_output_en = 0x1;

    lpc_reg_0x00.b.lpcomp0_deb_sel = 0;

    if (LPC_InitStruct->LPC_DebouncEn != DISABLE)
    {
        lpc_reg_0x00.b.lpcomp0_single_output_en = 0x0;
        lpc_reg_0x00.b.lpcomp0_deb_en = 0x1;
        lpc_reg_0x00.b.lpcomp0_deb_div = LPC_InitStruct->LPC_DebouncDiv;
        if (LPC_InitStruct->LPC_DebouncCnt > 255)
        {
            LPC_InitStruct->LPC_DebouncCnt = 255;
        }
        lpc_reg_0x00.b.lpcomp0_deb_cnt = LPC_InitStruct->LPC_DebouncCnt;

        lpc_reg_0x00.b.lpcomp0_deb_sel = 1;
    }

#if LPC_SUPPORT_NOMAL_FLOW
    /* LPC output enable */
    lpc_reg_0x00.b.lpcomp0_ie = 0x1;
#endif

#if LPC_SUPPORT_TRIGER_MODE
    /* LPC triger mode selection */
    lpc_reg_0x00.b.lpcomp0_hys_en = LPC_InitStruct->LPC_TrigerMode;
#endif

    LPCx->LPC_CR =  lpc_reg_0x00.d32;

#if LPC_SUPPORT_POWER_ON
    LPC_PowerOn(LPC_InitStruct);
#endif
}

/**
  * \brief  Fills each LPC_InitStruct member with its default value.
  * \param  LPC_InitStruct : pointer to a LPC_InitTypeDef structure which will be initialized.
  * \return None
  */
void LPC_StructInit(LPC_InitTypeDef *LPC_InitStruct)
{
    LPC_InitStruct->LPC_Channel           = LPC_CHANNEL_ADC0;
    LPC_InitStruct->LPC_Edge              = LPC_Vin_Below_Vth;

    LPC_InitStruct->LPC_DebouncEn         = DISABLE;
    LPC_InitStruct->LPC_DebouncDiv        = LPC_DEBOUNCE_DIV_1;
    LPC_InitStruct->LPC_DebouncCnt        = 0;
#if LPC_SUPPORT_TRIGER_MODE
    LPC_InitStruct->LPC_Mode              = LPC_DIVIDE_MODE;
    LPC_InitStruct->LPC_Threshold         = LPC_1944_mV;
    LPC_InitStruct->LPC_ThresholdL        = LPC_1944_mV;
    LPC_InitStruct->LPC_TrigerMode        = LPC_SINGLE_OUTPUT;
#else
    LPC_InitStruct->LPC_Threshold         = LPC_2000_mV;
#endif
}

/**
  * \brief  Enable or disable LPC peripheral.
  * \param  LPCx: Select the LPC peripheral. \ref LPC_Declaration
  * \param  NewState: new state of LPC peripheral.
  *     This parameter can be: ENABLE or DISABLE.
  * \return None
  */
void LPC_Cmd(LPC_TypeDef *LPCx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    LPC_CmdConfig(LPCx, NewState);
}


/**
  * \brief  Enable or disable the specified LPC interrupts.
  * \param  LPCx: Select the LPC peripheral. \ref LPC_Declaration
  * \param  LPC_INT: specifies the LPC interrupt source to be enabled or disabled.
  *         This parameter can be one of the following values:
  *         \arg LPC_INT_LPCOMP: voltage detection interrupt.
  *         \arg LPC_INT_LPCOMP_AON: wakeup voltage detection interrupt.
  * \param  NewState: new state of the specified LPC interrupt.
  *         This parameter can be: ENABLE or DISABLE.
  * \return None
  */
void LPC_INTConfig(LPC_TypeDef *LPCx, uint32_t LPC_INT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_LPC_CONFIG_INT(LPC_INT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));
    LPC_INTSetting(LPCx, LPC_INT, NewState);
    LPC_ClearFlag(LPC0, LPC_INT_LPCOMP);
}


/**
  * \brief  Enable wakeup signal to power sequence.
  * \param  LPCx: Select the LPC peripheral. \ref LPC_Declaration
  * \param  This parameter can be: ENABLE or DISABLE.
  * \return None.
  */
void LPC_WKCmd(LPC_TypeDef *LPCx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        LPCx->LPC_CR |= LPC_INT_LPCOMP_AON;
    }
    else
    {
        LPCx->LPC_CR &= ~LPC_INT_LPCOMP_AON;
    }
}


/**
  * \brief  Checks whether the specified LPC interrupt is set or not.
  * \param  LPCx: Select the LPC peripheral. \ref LPC_Declaration
  * \param  LPC_FLAG: specifies the LPC interrupt to check.
  *         This parameter can be one of the following values:
  *         \arg LPC_FLAG_LPCOMP: LPC NVIC flag.
  *         \arg LPC_FLAG_LPCOMP_AON: LPC trigger wakeup flag.
  * \return The new state of SPI_IT (SET or RESET).
  */
FlagStatus LPC_GetFlagStatus(LPC_TypeDef *LPCx, uint32_t LPC_FLAG)
{
    ITStatus int_status = RESET;

    /* Check the parameters */
    assert_param(IS_LPC_FLAG(LPC_FLAG));

    if (((LPCx->LPC_SR) & LPC_FLAG) != (uint32_t)RESET)
    {
        int_status = SET;
    }

    /* Return the LPC_FLAG status */
    return  int_status;
}

/**
  * \brief  Clear the specified LPC interrupt.
  * \param  LPCx: Select the LPC peripheral. \ref LPC_Declaration
  * \param  LPC_FLAG: specifies the LPC interrupt to clear.
  *         This parameter can be one of the following values:
  *         \arg LPC_FLAG_LPCOMP: LPC NVIC flag.
  * \return None
  */
void LPC_ClearFlag(LPC_TypeDef *LPCx, uint32_t LPC_FLAG)
{
    /* Check the parameters */
    assert_param(IS_LPC_CLEAR_FLAG(LPC_FLAG));

    LPC_ClearFlagConfig(LPCx, LPC_FLAG);
}

/**
  * \brief  Checks whether the specified LPC interrupt is set or not.
  * \param  LPCx: Select the LPC peripheral. \ref LPC_Declaration
  * \param  LPC_INT: specifies the LPC interrupt to check.
  *         This parameter can be one of the following values:
  *         \arg LPC_INT_LPCOMP: voltage detection interrupt.
  * \return The new state of SPI_IT (SET or RESET).
  */
ITStatus LPC_GetINTStatus(LPC_TypeDef *LPCx, uint32_t LPC_INT)
{
    ITStatus int_status = RESET;
    /* Check the parameters */
    assert_param(IS_LPC_STATUS_INT(LPC_INT));
    int_status = LPC_GetINTStatusConfig(LPCx, LPC_INT);
    return  int_status;
}

