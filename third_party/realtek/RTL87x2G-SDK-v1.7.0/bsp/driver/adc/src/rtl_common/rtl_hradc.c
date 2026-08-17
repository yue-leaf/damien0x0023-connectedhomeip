/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "rtl_hradc.h"
#include "rtl_rcc.h"

/*============================================================================*
 *                          Private Functions
 *============================================================================*/
extern void HRADC_AutoBootPowerConfig(HRADCMode_TypeDef HRADC_Mode);

/*============================================================================*
 *                           Public Functions
 *============================================================================*/
/**
  * \brief  Deinitializes the HRADC peripheral registers to their default reset values(turn off HRADC clock).
  * \param  None.
  * \return None.
  */
void HRADC_DeInit(void)
{
    RCC_PeriphClockCmd(APBPeriph_HRADC, APBPeriph_HRADC_CLOCK, DISABLE);
}

/**
  * \brief  Initializes the HRADC peripheral according to the specified
  *         parameters in the HRADC_InitStruct
  * \param  HRADCx: selected HRADC peripheral.
  * \param  HRADC_InitStruct: pointer to a HRADCInitTypeDef structure that
  *         contains the configuration information for the specified HRADC peripheral
  * \return None.
  */
void HRADC_Init(HRADC_TypeDef *HRADCx, HRADC_InitTypeDef *HRADC_InitStruct)
{
    /* Check the parameters */
    assert_param(IS_HRADC_PERIPH(HRADCx));
    assert_param(IS_HRADC_DATA_WRITE_TO_FIFO_TYPE(HRADC_InitStruct->HRADC_DataWriteToFifo));
    assert_param(IS_HRADC_SAMPLE_MODE(HRADC_InitStruct->HRADC_SampleMode));
    assert_param(IS_HRADC_OUTPUT_DATA_RATE_TYPE(HRADC_InitStruct->HRADC_OutPutDataRate));
    assert_param(IS_HRADC_FIFO_THRESHOLD(HRADC_InitStruct->HRADC_FifoThdLevel));
    assert_param(IS_HRADC_BURST_SIZE_CONFIG(HRADC_InitStruct->HRADC_BurstSize));
    assert_param(IS_HRADC_BITMAP(HRADC_InitStruct->HRADC_Bitmap));
    assert_param(IS_HRADC_GAIN_CTRL_TYPE(HRADC_InitStruct->HRADC_Gain_Crtol));
    assert_param(IS_HRADC_MODE(HRADC_InitStruct->HRADC_Mode));

    HRADC_REG0X_SDADC_TypeDef sdadc_reg_0x00 = {.d32 = HRADC->REG0X_SDADC};
    HRADC_REG0X_SDADC_DIG_CONFIG_TypeDef sdadc_reg_0x04 = {.d32 = HRADC->REG0X_SDADC_DIG_CONFIG};
    HRADC_REG2X_SDADC_DIG_CONFIG_TypeDef sdadc_reg_0x08 = {.d32 = HRADC->REG2X_SDADC_DIG_CONFIG};
    HRADC_REG4X_SDADC_DIG_CONFIG_TypeDef sdadc_reg_0x0c = {.d32 = HRADC->REG4X_SDADC_DIG_CONFIG};
    HRADC_REG_SDADC_POWERON_CONFIG_TypeDef sdadc_reg_0x34 = {.d32 = HRADC->REG_SDADC_POWERON_CONFIG};

    /* Config SDADC Power Mode*/
    HRADC_AutoBootPowerConfig(HRADC_InitStruct->HRADC_Mode);

    /*  Required call API provided by power manager FW for
        setting REG_ANAPAR_LDO[4:0] with reference FT
        calibrated value stored in Efuse */

    /* Enable boot ready interrupt */
    sdadc_reg_0x04.b.sdadc_boot_ready_int_en = 0x1;

    /* Not mask boot ready interrupt */
    sdadc_reg_0x04.b.sdadc_boot_ready_int_mask = 0x0;

    /* Enable power on sequence executed by HW */
    sdadc_reg_0x34.b.sdadc_auto_boot_en = 1;

    /* Configure related timing for auto boot control */
    sdadc_reg_0x34.b.sdadc_bg_ready_dly = HRADC_BG_READY_DLY_2MS;
    sdadc_reg_0x34.b.sdadc_ss_done_dly = HRADC_SS_DONE_DLY_4MS;
    sdadc_reg_0x34.b.sdadc_ldo_ready_dly = HRADC_LDO_READY_DLY_8MS;
    sdadc_reg_0x34.b.sdadc_adc_clk_ready_dly = HRADC_CLK_READY_DLY_62_5UM;
    sdadc_reg_0x34.b.sdadc_dmy_load_extend_dly = HRADC_InitStruct->HRADC_Mode == HRADC_Bypass_MODE ?
                                                 0x0 : HRADC_DMY_LOAD_EXT_DLY_500UM;
    sdadc_reg_0x34.b.sdadc_adc_ready_dly = HRADC_READY_DLY_125UM;
    HRADC->REG0X_SDADC_DIG_CONFIG = sdadc_reg_0x04.d32 ;
    HRADC->REG_SDADC_POWERON_CONFIG = sdadc_reg_0x34.d32 ;

    /* Config SDADC */

    /* Config SDADC analog performance parameters */
    sdadc_reg_0x00.b.REG_DTSDM_IB_SEL_1_0 = 0x2;
    sdadc_reg_0x00.b.REG_PGA_IB_SEL_1_0 = 0x2;
    sdadc_reg_0x00.b.REG_CHOP_DIV_1_0 = 0x3;
    sdadc_reg_0x00.b.REG_ISO_SDADC = 0x0;
    /* Configure SDADC PGA gain */
    sdadc_reg_0x00.b.REG_PGA_GSEL_2_0 = HRADC_InitStruct->HRADC_Gain_Crtol;
    HRADC->REG0X_SDADC = sdadc_reg_0x00.d32;

    /* Configure SDADC output data rate */
    if (HRADC_InitStruct->HRADC_OutPutDataRate < HRADC_6_25_SPS)
    {
        sdadc_reg_0x08.b.sdadc_adc_clk_sel = 0x0;
        sdadc_reg_0x08.b.sdadc_dr_config = HRADC_InitStruct->HRADC_OutPutDataRate & 0xF;
    }
    else
    {
        sdadc_reg_0x08.b.sdadc_adc_clk_sel = 1;
        sdadc_reg_0x08.b.sdadc_dr_config = HRADC_InitStruct->HRADC_OutPutDataRate & 0xF;
    }

    /* Configure channel setting */
    sdadc_reg_0x08.b.sdadc_ch_idx_en = HRADC_InitStruct->HRADC_Bitmap;

    /* Set schedule table */
    sdadc_reg_0x0c.b.sdadc_ch_num_idx0 =  HRADC_InitStruct->HRADC_SchIndex[0] & 0x3;
    sdadc_reg_0x0c.b.sdadc_mode_idx0 = (HRADC_InitStruct->HRADC_SchIndex[0] >> 2) & 0x1;
    sdadc_reg_0x0c.b.sdadc_ch_num_idx1 =  HRADC_InitStruct->HRADC_SchIndex[1] & 0x3;
    sdadc_reg_0x0c.b.sdadc_mode_idx1 = (HRADC_InitStruct->HRADC_SchIndex[1] >> 2) & 0x1;
    sdadc_reg_0x0c.b.sdadc_ch_num_idx2 = HRADC_InitStruct->HRADC_SchIndex[2] & 0x3;
    sdadc_reg_0x0c.b.sdadc_mode_idx2 = (HRADC_InitStruct->HRADC_SchIndex[2] >> 2) & 0x1;
    sdadc_reg_0x0c.b.sdadc_ch_num_idx3 = HRADC_InitStruct->HRADC_SchIndex[3] & 0x3;
    sdadc_reg_0x0c.b.sdadc_mode_idx3 = (HRADC_InitStruct->HRADC_SchIndex[3] >> 2) & 0x1;
    sdadc_reg_0x0c.b.sdadc_ch_num_idx4 =  HRADC_InitStruct->HRADC_SchIndex[4] & 0x3;
    sdadc_reg_0x0c.b.sdadc_mode_idx4 = (HRADC_InitStruct->HRADC_SchIndex[4] >> 2) & 0x1;
    sdadc_reg_0x0c.b.sdadc_ch_num_idx5  =  HRADC_InitStruct->HRADC_SchIndex[5] & 0x3;
    sdadc_reg_0x0c.b.sdadc_mode_idx5 = (HRADC_InitStruct->HRADC_SchIndex[5] >> 2) & 0x1;
    sdadc_reg_0x0c.b.sdadc_ch_num_idx6 = HRADC_InitStruct->HRADC_SchIndex[6] & 0x3;
    sdadc_reg_0x0c.b.sdadc_mode_idx6 = (HRADC_InitStruct->HRADC_SchIndex[6] >> 2) & 0x1;
    sdadc_reg_0x0c.b.sdadc_ch_num_idx7 = HRADC_InitStruct->HRADC_SchIndex[7] & 0x3;
    sdadc_reg_0x0c.b.sdadc_mode_idx7 = (HRADC_InitStruct->HRADC_SchIndex[7] >> 2) & 0x1;
    sdadc_reg_0x0c.b.sdadc_filter_stage2_sel_order = 0x3;
    HRADC->REG4X_SDADC_DIG_CONFIG = sdadc_reg_0x0c.d32;
    /* Configure SDADC conversion mode */
    sdadc_reg_0x08.b.sdadc_conv_mode = HRADC_InitStruct->HRADC_SampleMode;
    sdadc_reg_0x08.b.sdadc_dig_clk_sel = 0x1;
    /* In single shot or continuous conversion mode,
        only channel idx[0] configured in channel
        configuration register will be converted; */
    sdadc_reg_0x08.b.sdadc_scan_en = !(HRADC_InitStruct->HRADC_Bitmap == 0x1);
    HRADC->REG4X_SDADC_DIG_CONFIG = sdadc_reg_0x0c.d32;

    /* Disable required mask or disable interrupts */
    HRADC->REG0X_SDADC_DIG_CONFIG &= (~0xF);

    /* Configure FIFO */
    sdadc_reg_0x04.d32 = HRADC->REG0X_SDADC_DIG_CONFIG;
    sdadc_reg_0x04.b.sdadc_fifo_en = HRADC_InitStruct->HRADC_DataWriteToFifo;
    sdadc_reg_0x04.b.sdadc_fifo_thr = HRADC_InitStruct->HRADC_FifoThdLevel;

    /* Configure DMA feature */
    if (HRADC_InitStruct->HRADC_BurstSize > 0x1F)
    {
        HRADC_InitStruct->HRADC_BurstSize = 0x1F;
    }
    sdadc_reg_0x04.b.sdadc_dma_req_burst_size = HRADC_InitStruct->HRADC_BurstSize;

    /* Configure other feature */
    if (HRADC_InitStruct->HRADC_DataDiscard > 0x1F)
    {
        HRADC_InitStruct->HRADC_DataDiscard = 0x1F;
    }
    if (HRADC_InitStruct->HRADC_SwitchInterval > 0xF)
    {
        HRADC_InitStruct->HRADC_SwitchInterval = 0xF;
    }
    sdadc_reg_0x04.b.sdadc_data_discard = HRADC_InitStruct->HRADC_DataDiscard;
    sdadc_reg_0x08.b.sdadc_ch_switch_intv = HRADC_InitStruct->HRADC_SwitchInterval;
    HRADC->REG0X_SDADC_DIG_CONFIG = sdadc_reg_0x04.d32;
    HRADC->REG2X_SDADC_DIG_CONFIG = sdadc_reg_0x08.d32;

    /* SDADC power down */
    sdadc_reg_0x34.d32 = HRADC->REG_SDADC_POWERON_CONFIG;
    sdadc_reg_0x34.b.sdadc_en = 0x0;
    HRADC->REG_SDADC_POWERON_CONFIG = sdadc_reg_0x34.d32;

    /* Boot SDADC with auto boot flow */

    /* Enable auto boot power on sequence; */
    sdadc_reg_0x34.d32 = HRADC->REG_SDADC_POWERON_CONFIG;
    sdadc_reg_0x34.b.sdadc_en = 0x1;
    HRADC->REG_SDADC_POWERON_CONFIG = sdadc_reg_0x34.d32;

    /* Wait for sdadc_boot_ready */
    while ((HRADC->REG0X_SDADC_DIG_CONFIG & BIT14) == 0x0);

    /* Clear boot_ready_interrupt */
    sdadc_reg_0x04.d32 = HRADC->REG0X_SDADC_DIG_CONFIG;
    sdadc_reg_0x04.b.sdadc_boot_ready_int = 0x1;
    HRADC->REG0X_SDADC_DIG_CONFIG = sdadc_reg_0x04.d32;
}
/**
  * \brief  Fills each HRADC_InitStruct member with its default value.
  * \param  HRADC_InitStruct: pointer to an HRADC_InitTypeDef structure which will be initialized.
  * \return None
  */
void HRADC_StructInit(HRADC_InitTypeDef *HRADC_InitStruct)
{
    HRADC_InitStruct->HRADC_DataWriteToFifo = HRADC_DATA_WRITE_TO_FIFO_ENABLE;
    HRADC_InitStruct->HRADC_FifoThdLevel = 0x0;
    HRADC_InitStruct->HRADC_BurstSize = 0x0;
    HRADC_InitStruct->HRADC_SampleMode = HRADC_ONE_SHOT_MODE;
    HRADC_InitStruct->HRADC_OutPutDataRate = HRADC_10_SPS;
    HRADC_InitStruct->HRADC_DataDiscard = 0x0;
    HRADC_InitStruct->HRADC_SwitchInterval = 0x3;
    HRADC_InitStruct->HRADC_SchIndex[0] = 0x0;
    HRADC_InitStruct->HRADC_SchIndex[1] = 0x0;
    HRADC_InitStruct->HRADC_SchIndex[2] = 0x0;
    HRADC_InitStruct->HRADC_SchIndex[3] = 0x0;
    HRADC_InitStruct->HRADC_SchIndex[4] = 0x0;
    HRADC_InitStruct->HRADC_SchIndex[5] = 0x0;
    HRADC_InitStruct->HRADC_SchIndex[6] = 0x0;
    HRADC_InitStruct->HRADC_SchIndex[7] = 0x0;
    HRADC_InitStruct->HRADC_Bitmap = 0x0;
    HRADC_InitStruct->HRADC_Gain_Crtol  = HRADC_GAIN_1X;
    HRADC_InitStruct->HRADC_Mode = HRADC_28_LDO_MODE;
}
/**
  * \brief  Enables or disables the HRADC peripheral.
  * \param  HRADCMode: HRADC mode select.
  *         This parameter can be one of the following values:
  *         \arg HRADC_ONE_SHOT_MODE: one shot mode.
  *         \arg HRADC_CONTINUOUS_MODE: continuous mode.
  * \param  NewState: new state of the HRADC peripheral.
  *         This parameter can be: ENABLE or DISABLE.
  * \return None
  */
void HRADC_Cmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    HRADC_REG2X_SDADC_DIG_CONFIG_TypeDef sdadc_reg_0x08 = {.d32 = HRADC->REG2X_SDADC_DIG_CONFIG};
    sdadc_reg_0x08.b.sdadc_sample_start_en = NewState;
    HRADC->REG2X_SDADC_DIG_CONFIG = sdadc_reg_0x08.d32;

}
/**
  * \brief  Mask the specified HRADC interrupts.
  * \param  HRADCx: selected HRADC peripheral.
  * \param  HRADC_INT: specifies the HRADC interrupts sources to be enabled or disabled.
  *         This parameter can be any combination of the following values:
  *         \arg HRADC_INT_FIFO_THR :HRADC FIFO size > thd interrupt.
  *         \arg HRADC_INT_FIFO_OVERFLOW :HRADC FIFO overflow interrupt.
  *         \arg HRADC_INT_SCAN_ONEROUND_DOWN :HRADC one round done interrupt.
  *         \arg HRADC_INT_DATA_READY :HRADC data ready interrupt.
  *         \arg HRADC_INT_BOOT_READY :HRADC boot ready interrupt
  * \param  NewState: new state of the specified HRADC interrupts.
  *         This parameter can be: ENABLE or DISABLE.
  * \return None
  */
void HRADC_MaskINTConfig(HRADC_TypeDef *HRADCx, uint32_t HRADC_INT,
                         FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_HRADC_PERIPH(HRADCx));
    assert_param(IS_HRADC_INT(HRADC_INT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        /* Enable the selected HRADC interrupts */
        HRADCx->REG0X_SDADC_DIG_CONFIG |= HRADC_INT << 5;
    }
    else
    {
        /* Disable the selected HRADC interrupts */
        HRADCx->REG0X_SDADC_DIG_CONFIG &= ~(HRADC_INT << 5) ;
    }
}

/**
  * \brief  Enable or disable the specified HRADC interrupts.
  * \param  HRADCx: selected HRADC peripheral.
  * \param  HRADC_INT: specifies the HRADC interrupts sources to be enabled or disabled.
  *         This parameter can be any combination of the following values:
  *         \arg HRADC_INT_FIFO_THR :HRADC FIFO size > thd interrupt.
  *         \arg HRADC_INT_FIFO_OVERFLOW :HRADC FIFO overflow interrupt.
  *         \arg HRADC_INT_SCAN_ONEROUND_DOWN :HRADC one round done interrupt.
  *         \arg HRADC_INT_DATA_READY :HRADC data ready interrupt.
  *         \arg HRADC_INT_BOOT_READY :HRADC boot ready interrupt
  * \param  NewState: new state of the specified HRADC interrupts.
  *         This parameter can be: ENABLE or DISABLE.
  * \return None
  */
void HRADC_INTConfig(HRADC_TypeDef *HRADCx, uint32_t HRADC_INT,
                     FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_HRADC_PERIPH(HRADCx));
    assert_param(IS_HRADC_INT(HRADC_INT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        /* Enable the selected HRADC interrupts */
        HRADCx->REG0X_SDADC_DIG_CONFIG |= HRADC_INT;
    }
    else
    {
        /* Disable the selected HRADC interrupts */
        HRADCx->REG0X_SDADC_DIG_CONFIG &= (uint32_t)~HRADC_INT;
    }
}

/**
  * \brief  Read HRADC data according to specific channel.
  * \param  HRADCx: selected HRADC peripheral.
  * \param  index: can be 0 to 8
  * \return The 10-bit converted HRADC data.
  */
uint32_t HRADC_ReadRawData(HRADC_TypeDef *HRADCx, uint8_t Index)
{
    /* Check the parameters */
    assert_param(IS_HRADC_PERIPH(HRADCx));
    assert_param(Index < 8);

    return (*(uint32_t *)((uint32_t *)(&HRADCx->REG_CH_IDX0_SDADC_OUTPUT_DATA) + (Index)));
}
/**
  * \brief  Get one data from HRADC FIFO.
  * \param  HRADCx: selected HRADC peripheral.
  * \return HRADC FIFO data.
  */
uint32_t HRADC_ReadFIFO(HRADC_TypeDef *HRADCx)
{
    /* Check the parameters */
    assert_param(IS_HRADC_PERIPH(HRADCx));

    return (uint32_t)((HRADCx->REG_SDADC_OUTPUT_DATA_FIFO) & 0xFFFFFF);
}
/**
  * \brief  Read data from HRADC FIFO.
  * \param  HRADCx: selected HRADC peripheral.
  * \param[out]  outBuf: buffer to save data read from HRADC FIFO.
  * \param  num: number of data to be read.
  * \return None
  */
void HRADC_ReadFIFOData(HRADC_TypeDef *HRADCx, uint16_t *OutBuf, uint16_t Num)
{
    /* Check the parameters */
    assert_param(IS_HRADC_PERIPH(HRADCx));

    while (Num--)
    {
        *OutBuf++ = (uint32_t)HRADCx->REG_SDADC_OUTPUT_DATA_FIFO & 0xFFFFFF;
    }
}
/**
  * \brief  Get HRADC fifo data number.
  * \param  HRADCx: selected HRADC peripheral.
  * \return current data number in HRADC fifo.
  */
uint8_t HRADC_GetFIFODataLen(HRADC_TypeDef *HRADCx)
{
    /* Check the parameters */
    assert_param(IS_HRADC_PERIPH(HRADCx));

    return ((uint8_t)(HRADCx->REG_SDADC_FIFO_INFO));
}
/**
  * \brief  Config HRADC schedule table.
  * \param  HRADCx: selected HRADC peripheral.
  * \param  HRADCMode: HRADC mode.
  *         This parameter can be one of the following values:
  *         \arg EXT_SINGLE_ENDED(index)
  *         \arg EXT_DIFFERENTIAL(index)
  *         \arg INTERNAL_VBAT_MODE
  * \param  Index: Schedule table index.
  * \return None.
  */
void HRADC_SchIndexConfig(HRADC_TypeDef *HRADCx, uint8_t HRADCMode, uint16_t Index)
{
    /* Check the parameters */
    assert_param(IS_HRADC_PERIPH(HRADCx));
    assert_param(IS_HRADC_SCHEDULE_INDEX_CONFIG(HRADCMode));
    assert_param(IS_HRADC_SCH_INDEX(Index));

    *(uint32_t *)(&HRADCx->REG4X_SDADC_DIG_CONFIG + Index * 3) |= HRADCMode;
}
/**
  * \brief  Set HRADC schedule table.
  * \param  HRADCx: selected HRADC peripheral.
  * \param  channelMap: HRADC channel map.
  * \param  NewState: new state of the HRADC peripheral.
  *         This parameter can be: ENABLE or DISABLE.
  * \return None.
  */
void HRADC_BitMapConfig(HRADC_TypeDef *HRADCx, uint16_t BitMap,
                        FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_HRADC_PERIPH(HRADCx));
    assert_param(IS_HRADC_BITMAP(BitMap));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    HRADC_REG2X_SDADC_DIG_CONFIG_TypeDef sdadc_reg_0x08 = {.d32 = HRADC->REG2X_SDADC_DIG_CONFIG};
    if (NewState == ENABLE)
    {
        sdadc_reg_0x08.b.sdadc_ch_idx_en |= BitMap;
    }
    else
    {
        sdadc_reg_0x08.b.sdadc_ch_idx_en &= (~BitMap);
    }
    HRADC->REG2X_SDADC_DIG_CONFIG = sdadc_reg_0x08 .d32;
}
/**
  * \brief  Checks whether the specified HRADC interrupt status flag is set or not.
  * \param  HRADCx: selected HRADC peripheral.
  * \param  HRADC_INT_FLAG: specifies the interrupt status flag to check.
  *         This parameter can be one of the following values:
  *         \arg HRADC_INT_ONE_SHOT_DONE: HRADC once convert end interrupt.
  *         \arg HRADC_INT_FIFO_OVERFLOW: HRADC FIFO overflow interrupt.
  *         \arg HRADC_INT_FIFO_THD: fifo larger than threshold interrupt.
  *         \arg HRADC_INT_FIFO_RD_ERR: HRADC FIFO underflow interrupt.
  *         \arg HRADC_INT_FIFO_RD_REQ: HRADC FIFO size>=burst size interrupt.
  * \return The new state of HRADC_INT (SET or RESET).
  */
ITStatus HRADC_GetINTStatus(HRADC_TypeDef *HRADCx, uint32_t HRADC_INT)
{
    /* Check the parameters */
    assert_param(IS_HRADC_PERIPH(HRADCx));
    assert_param(IS_HRADC_INT(HRADC_INT));

    FlagStatus bitstatus = RESET;
    if ((HRADCx->REG0X_SDADC_DIG_CONFIG & (HRADC_INT << 10)) != 0)
    {
        bitstatus = SET;
    }
    return bitstatus;
}
/**
  * \brief  Clear the HRADC interrupt pending bit.
  * \param  HRADCx: selected HRADC peripheral.
  * \param  HRADC_INT: specifies the interrupt pending bit to clear.
  *         This parameter can be any combination of the following values:
  *         \arg HRADC_INT_ONE_SHOT_DONE: HRADC once convert end interrupt.
  *         \arg HRADC_INT_FIFO_OVERFLOW: HRADC FIFO overflow interrupt.
  *         \arg HRADC_INT_FIFO_THD: fifo larger than threshold interrupt.
  *         \arg HRADC_INT_FIFO_RD_ERR: HRADC FIFO underflow interrupt.
  *         \arg HRADC_INT_FIFO_RD_REQ: HRADC FIFO size>=burst size interrupt.
  * \return None
  */
void HRADC_ClearINTPendingBit(HRADC_TypeDef *HRADCx, uint32_t HRADC_INT)
{
    /* Check the parameters */
    assert_param(IS_HRADC_PERIPH(HRADCx));
    assert_param(IS_HRADC_INT(HRADC_INT));

    HRADCx->REG0X_SDADC_DIG_CONFIG |= (HRADC_INT << 10);
}
/**
 * \brief  Clear HRADC FIFO.
 * \param  HRADCx: Specify HRADC peripheral.
 * \return None.
 */
void HRADC_ClearFIFO(HRADC_TypeDef *HRADCx)
{
    /* Check the parameters */
    assert_param(IS_HRADC_PERIPH(HRADCx));

    HRADCx->REG0X_SDADC_DIG_CONFIG |= BIT15;
}

/**
  * \brief  Enbale or disable stop fifo from writing data.
  * \param  HRADCx: selected ADC peripheral.
  * \param  NewState: new state of the ADC fifo write.
  *         This parameter can be: ENABLE or DISABLE.
  * \return None
  */
void HRADC_WriteFIFOCmd(HRADC_TypeDef *HRADCx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_ADC_PERIPH(ADCx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    HRADC_REG0X_SDADC_DIG_CONFIG_TypeDef sdadc_reg_0x04 = {.d32 = HRADC->REG0X_SDADC_DIG_CONFIG};
    sdadc_reg_0x04.b.sdadc_fifo_en = NewState;
    HRADC->REG0X_SDADC_DIG_CONFIG = sdadc_reg_0x04.d32;
}

/**
 * \brief  Get all HRADC interrupt flag status.
 * \param  HRADCx: Specify HRADC peripheral.
 * \return All HRADC interrupt status.
 */
uint8_t HRADC_GetAllFlagStatus(HRADC_TypeDef *HRADCx)
{
    /* Check the parameters */
    assert_param(IS_HRADC_PERIPH(HRADCx));

    return ((uint8_t)(((HRADCx->REG0X_SDADC_DIG_CONFIG) & (0x1F << 10)) >> 10));
}

