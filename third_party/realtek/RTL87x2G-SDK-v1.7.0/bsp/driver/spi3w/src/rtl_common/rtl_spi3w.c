/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "rtl_spi3w.h"
#include "rtl_rcc.h"

/*============================================================================*
 *                           Public Functions
 *============================================================================*/
/**
  * \brief  Deinitializes the SPI3W peripheral registers to their default reset values.
  * \return None
  */
void SPI3W_DeInit(void)
{
    RCC_PeriphClockCmd(APBPeriph_SPI3W, APBPeriph_SPI3W_CLOCK, DISABLE);
}

/**
  * \brief  Initializes the SPI3W peripheral according to the specified
  *         parameters in the SPI3W_InitStruct
  * \param  SPI3W_InitStruct: pointer to a SPI3W_InitTypeDef structure that
  *         contains the configuration information for the specified SPI3W peripheral
  * \return None
  */
void SPI3W_Init(SPI3W_InitTypeDef *SPI3W_InitStruct)
{
    /* Check the parameters */
    assert_param(IS_SPI3W_MODE(SPI3W_InitStruct->SPI3W_Mode));
    assert_param(IS_SPI3W_OE_DELAY_CFG(SPI3W_InitStruct->SPI3W_OutputDelay));
    assert_param(IS_SPI3W_END_EXTEND_MODE(SPI3W_InitStruct->SPI3W_ExtMode));
    assert_param(IS_SPI3W_READ_CYCLE_DELAY(SPI3W_InitStruct->SPI3W_ReadDelay));

    uint32_t divValue = 0;

    divValue = (((SPI3W_InitStruct->SPI3W_SysClock) / (SPI3W_InitStruct->SPI3W_Speed * 2) - 1)) & 0xFF;

    /* Initialize the parameters */
    SPI3W_CONFIG_TypeDef spi3w_0x30 = {.d32 = SPI3W->SPI3W_CONFIG};
    spi3w_0x30.b.spi3w_cs_en = SPI3W_InitStruct->SPI3W_Mode;
    spi3w_0x30.b.spi3w_oe_dly_en = SPI3W_InitStruct->SPI3W_OutputDelay;
    spi3w_0x30.b.spi3w_end_ext_en = SPI3W_InitStruct->SPI3W_ExtMode;
    spi3w_0x30.b.spi3w_div_num = divValue;
    spi3w_0x30.b.spi3w_dly_cycle = SPI3W_InitStruct->SPI3W_ReadDelay;
    spi3w_0x30.b.spi3w_burst_read_num = 0;
    SPI3W->SPI3W_CONFIG = spi3w_0x30.d32;

    /* Clear all read data registers,  read number and interrupt */
    SPI3W->SPI3W_CLR |= 0x7;
}

/**
  * \brief  Fills each SPI3W_InitStruct member with its default value.
  * \param  SPI3W_InitStruct: pointer to an SPI3W_InitTypeDef structure which will be initialized.
  * \return None
  */
void SPI3W_StructInit(SPI3W_InitTypeDef *SPI3W_InitStruct)
{
    SPI3W_InitStruct->SPI3W_SysClock      = 20000000;
    SPI3W_InitStruct->SPI3W_Speed         = 1000000;
    SPI3W_InitStruct->SPI3W_Mode          = SPI3W_2WIRE_MODE;
    SPI3W_InitStruct->SPI3W_ReadDelay     = 0; /* Delay time = (SPI3W_ReadDelay+1)/(2*SPI3W_Speed) */
    SPI3W_InitStruct->SPI3W_OutputDelay   = SPI3W_OE_DELAY_1T;
    SPI3W_InitStruct->SPI3W_ExtMode       = SPI3W_NORMAL_MODE;
}

/**
  * \brief  Enable or disable the specified SPI3W peripheral.
  * \param  NewState: new state of the SPI3W peripheral.
  *         This parameter can be: ENABLE or DISABLE.
  * \return None
  */
void SPI3W_Cmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    SPI3W_CONFIG_TypeDef spi3w_0x30 = {.d32 = SPI3W->SPI3W_CONFIG};
    if (NewState != DISABLE)
    {
        /* Enable the selected SPI3W peripheral */
        spi3w_0x30.b.spi3w_en = 1;
    }
    else
    {
        /* Disable the selected SPI3W peripheral */
        spi3w_0x30.b.spi3w_en = 0;
    }
    SPI3W->SPI3W_CONFIG = spi3w_0x30.d32;

    return;
}

/**
  * \brief  Enable or disable the specified SPI3W interrupts.
  * \param  SPI3W_INT: specifies the SPI3W interrupts sources to be enabled or disabled.
  *         This parameter can be only be the following value:
  *         \arg SPI3W_INT_BIT: enable SPI3W interrupt.
  * \param  NewState: new state of the specified SPI3W interrupts.
  *         This parameter can be: ENABLE or DISABLE.
  * \return None
  */
void SPI3W_INTConfig(uint32_t SPI3W_INT, FunctionalState newState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(newState));
    assert_param(IS_SPI3W_INT(SPI3W_INT));

    if (newState == ENABLE)
    {
        /* Enable the selected SPI_3W interrupts */
        SPI3W->SPI3W_CONFIG |= SPI3W_INT;
    }
    else
    {
        /* Disable the selected SPI_3W interrupts */
        SPI3W->SPI3W_CONFIG &= (uint32_t)(~SPI3W_INT);
    }
}

/**
  * \brief  Configure resync signal time value.
  * \param  value: Resync signal time value whose uint is 1/(2*SPI3W_Speed).
  *         This parameter can be only be the following value: 0x0 to 0xf.
  * \return None
  */
void SPI3W_SetResyncTime(uint32_t value)
{
    SPI3W_CONFIG_TypeDef spi3w_0x30 = {.d32 = SPI3W->SPI3W_CONFIG};
    spi3w_0x30.b.spi3w_resync_timer = value & 0xF;
    SPI3W->SPI3W_CONFIG = spi3w_0x30.d32;

    return;
}

/**
  * \brief  Send resync signal or not .Must send when SPI3W is disable.
  * \param  NewState: new state of the SPI3W peripheral.
  *         This parameter can be only be the following value:
  * \param  ENABLE: trigger resync signal.
  * \param  ENABLE: disable resync signal.
  * \return None
  */
void SPI3W_ResyncSignalCmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));
    SPI3W_CLR_TypeDef spi3w_0x38 = {.d32 = SPI3W->SPI3W_CLR};

    if (NewState != DISABLE)
    {
        spi3w_0x38.b.spi3w_resync_en = 1;
    }
    else
    {
        spi3w_0x38.b.spi3w_resync_en = 0;
    }
    SPI3W->SPI3W_CLR = spi3w_0x38.d32;
}

/**
  * \brief  Get total number of data byte in each SPI3W reading.
  * \param  None.
  * \return The total number of data byte in each SPI3W reading.
  */
uint8_t SPI3W_GetRxDataLen(void)
{
    return (uint8_t)(SPI3W->SPI3W_STS & 0xF);
}

/**
  * \brief  Clear read data number status.
  * \param  None.
  * \return None
  */
void SPI3W_ClearRxDataLen(void)
{
    SPI3W_CLR_TypeDef spi3w_0x38 = {.d32 = SPI3W->SPI3W_CLR};
    spi3w_0x38.b.spi3w_rd_num_clr = 1;
    SPI3W->SPI3W_CLR = spi3w_0x38.d32;
}

/**
  * \brief  Clear all read data registers.
  * \param  None.
  * \return None.
  */
void SPI3W_ClearRxFIFO(void)
{
    SPI3W_CLR_TypeDef spi3w_0x38 = {.d32 = SPI3W->SPI3W_CLR};
    spi3w_0x38.b.spi3w_rd_data_clr = 1;
    SPI3W->SPI3W_CLR = spi3w_0x38.d32;
}

/**
  * \brief  start to write data.
  * \param  address: write address.
  * \param  data: write data.
  * \return None
  */
void SPI3W_StartWrite(uint8_t address, uint8_t data)
{
    SPI3W_WDATA_TypeDef spi3w_0x34 = {.d32 = 0};
    spi3w_0x34.b.spi3w_data_mode = 1;
    spi3w_0x34.b.spi3w_data_addr = address & 0x7F;
    spi3w_0x34.b.spi3w_data_data = data & 0xFF;
    SPI3W->SPI3W_WDATA = spi3w_0x34.d32;
}

/**
  * \brief  start read.
  * \param  address: read address.
  * \param  readNum: number of data to read.This value can be 0x1 to 0xf.
  * \return None.
  */
void SPI3W_StartRead(uint8_t address, uint32_t len)
{
    SPI3W_CONFIG_TypeDef spi3w_0x30 = {.d32 = SPI3W->SPI3W_CONFIG};
    SPI3W_WDATA_TypeDef spi3w_0x34 = {.d32 = 0};
    if (len == 1)
    {
        /* Disable burst read */
        spi3w_0x30.b.spi3w_burst_read_en = 0;
        SPI3W->SPI3W_CONFIG = spi3w_0x30.d32;

        /* Read address */
        spi3w_0x34.b.spi3w_data_addr = address;
        SPI3W->SPI3W_WDATA = spi3w_0x34.d32;
    }
    else
    {
        /* Enable burst read */
        spi3w_0x30.b.spi3w_burst_read_en = 1;
        SPI3W->SPI3W_CONFIG = spi3w_0x30.d32;

        /* Read num */
        spi3w_0x30.b.spi3w_burst_read_num = 0;
        SPI3W->SPI3W_CONFIG = spi3w_0x30.d32;
        spi3w_0x30.b.spi3w_burst_read_num = len;
        SPI3W->SPI3W_CONFIG = spi3w_0x30.d32;

        /* Set read command and read address */
        spi3w_0x34.b.spi3w_data_addr = address;
        SPI3W->SPI3W_WDATA = spi3w_0x34.d32;
    }
}

/**
  * \brief  write data.
  * \param  outBuf: buffer to store read datas.
  * \param  readNum: read number.
  * \return None
  */
void SPI3W_ReadBuf(uint8_t *pBuf, uint8_t readNum)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint32_t regVal = 0;
    uint8_t regIndex =  readNum / 4;

    for (i = 0; i < regIndex; i++)
    {
        regVal = *((volatile uint32_t *)(&(SPI3W->SPI3W_RDATA0) + i));

        for (j = 0; j < 4; j++)
        {
            *pBuf++ = (uint8_t)(regVal >> (8 * j));
        }
    }

    if (readNum > regIndex * 4)
    {
        regVal = *((volatile uint32_t *)(&(SPI3W->SPI3W_RDATA0) + regIndex));

        for (j = 0; j < (readNum - regIndex * 4); j++)
        {
            *pBuf++ = (uint8_t)(regVal >> (8 * j));
        }
    }
}

/**
  * \brief  Checks whether the specified SPI_3W flag is set or not.
  * \param  SPI3W_FLAG: specifies the flag to check.
  *         This parameter can be one of the following values:
  *         \arg SPI3W_FLAG_BUSY: SPI3W is busy.
  *         \arg SPI3W_FLAG_INT_IND: there is SPI3W interrupt.
  *         \arg SPI3W_FLAG_RESYNC_BUSY: resync busy or not.
  * \return The new state of SPI3W_FLAG (SET or RESET).
  */
FlagStatus SPI3W_GetFlagStatus(uint32_t SPI3W_FLAG)
{
    /* Check the parameters */
    assert_param(IS_SPI3W_FLAG(SPI3W_FLAG));

    FlagStatus bitstatus = RESET;

    if (SPI3W->SPI3W_STS & SPI3W_FLAG)
    {
        bitstatus = SET;
    }

    return bitstatus;
}

/**
  * \brief  Clears the SPI3W interrupt pending bits.
  * \param  SPI3W_INT: specifies the SPI3W interrupts sources to be enabled or disabled.
  *         This parameter can be only be the following value:
  *         \arg SPI3W_INT_BIT: enable SPI3W interrupt.
  * \return None
  */
void SPI3W_ClearINTPendingBit(uint32_t SPI3W_INT)
{
    /* Check the parameters */
    assert_param(IS_SPI3W_INT(SPI3W_INT));

    SPI3W->SPI3W_CLR |= SPI3W_INT;
}

#if (SPI3W_SUPPORT_RAP_FUNCTION ==1)

void SPI3W_RAPQactiveCtrl(uint32_t Qactive, FunctionalState NewState)
{
    SPI3W_QACTIVE_ALWAYS_HIGH_TypeDef spi3w_0x60 = {.d32 = SPI3W->SPI3W_QACTIVE_ALWAYS_HIGH};
    spi3w_0x60.b.spi3w_qactive_always_high = NewState;
    SPI3W->SPI3W_QACTIVE_ALWAYS_HIGH = spi3w_0x60.d32;
}

void SPI3W_QuickBurstStart()
{
    /* Quick Burst just work when RAP mode is disabled*/
    SPI3W_QB_START_TypeDef spi3w_0x80 = {.d32 = SPI3W->SPI3W_QB_START};
    spi3w_0x80.b.spi3w_qb_start = 1;
    SPI3W->SPI3W_QB_START = spi3w_0x80.d32;
}

void SPI3W_QuickBurstPulseWidthSet(uint8_t pulse_width)
{
    /* Quick Burst just work when RAP mode is disabled*/
    SPI3W_QB_PW_TypeDef spi3w_0x90 = {.d32 = SPI3W->SPI3W_QB_PW};
    spi3w_0x90.b.spi3w_qb_pw = pulse_width;
    SPI3W->SPI3W_QB_PW = spi3w_0x90.d32;
}

void SPI3W_RAPModeCmd(FunctionalState NewState)
{
    SPI3W_RAP_MODE_TypeDef spi3w_0x100 = {.d32 = SPI3W->SPI3W_RAP_MODE};
    spi3w_0x100.b.spi3w_rap_mode = NewState;
    SPI3W->SPI3W_RAP_MODE = spi3w_0x100.d32;
}

void SPI3W_TaskTrigger(uint32_t Task)
{
    SPI3W->SPI3W_TASKS_START |= BIT(Task);
}

#endif

