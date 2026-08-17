/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Includes ------------------------------------------------------------------*/
#include "rtl_lpc.h"
#include "utils.h"
#include "rtl_rcc.h"

/* AON register address defines */
#define LPC_AON_MBIAS                       ((AON_REG0X_MBIAS_TYPE *)0x40001858UL)
#define LPC_AON_AUXADC                      ((AON_NS_REG0X_SD_TYPE *)0x40001B98UL)
#define AUXADC_AON_REG0X1B90                ((AON_NS_REG0X_LDO_TYPE *)0x40001B90UL)
#define PERI_ON_ADC_CLOCK_CTRL                   *((volatile uint32_t *)0x4000231CUL)
#define PERI_ON_AUDIO_CLOCK_CTRL            *((volatile uint32_t *)0x40002700UL)

void LPC_PowerOn(LPC_InitTypeDef *LPC_InitStruct)
{
    /* Set LPC Threshold */
    LPC_AON_MBIAS->MBIAS_REG0X_SD_TUNE_RO =  LPC_InitStruct->LPC_Threshold & 0x3F;
    /* set to LDO mode */
    AUXADC_AON_REG0X1B90->hw_pd = 0x0;
    /* enable ADC level shifter */
    AUXADC_AON_REG0X1B90->anapar_pow_ad_1 = 0x1;

    platform_delay_us(10);

    /* Set LPC Input Channel */
    LPC_AON_AUXADC->CH_NUM_H_3_0 = LPC_InitStruct->LPC_Channel;
    /* Set LPC edge */
    LPC_AON_AUXADC->SD_POSEDGE_H = LPC_InitStruct->LPC_Edge ;
    /* turn on lpc */
    LPC_AON_AUXADC-> POW_SD_H = 0x1;

    platform_delay_us(200);
}

void LPC_PowerOff(void)
{
    /*Added to stabilize the power supply!*/
    /* set to LDO mode */
    uint32_t adc_clk_en = (PERI_ON_ADC_CLOCK_CTRL & BIT1) >> 1;
    uint32_t codec_clk_en = (PERI_ON_AUDIO_CLOCK_CTRL & BIT9) >> 9;
    if (adc_clk_en != 1 && codec_clk_en != 1)
    {
        AUXADC_AON_REG0X1B90->hw_pd = 0x1;
    }

    /* turn off LPC */
    LPC_AON_AUXADC-> POW_SD_H = 0x0;
    LPC_AON_AUXADC->SD_POSEDGE_H = 0x1;

}
void LPC_SISet(void)
{
    /* turn on lpc */
    LPC_AON_AUXADC-> POW_SD_H = 0x1;
    /* set to LDO mode */
    AUXADC_AON_REG0X1B90->hw_pd = 0x0;
    /* enable ADC level shifter */
    AUXADC_AON_REG0X1B90->anapar_pow_ad_1 = 0x1;
}

void LPC_ChannelConfig(LPC_TypeDef *LPCx, LPC_InitTypeDef *LPC_InitStruct)
{
    /* Set LPC Input Channel */

    LPC_AON_AUXADC->CH_NUM_H_3_0 = LPC_InitStruct->LPC_Channel;
    /* Set LPC edge */
    LPC_AON_AUXADC->SD_POSEDGE_H = LPC_InitStruct->LPC_Edge ;
    /* Set LPC Threshold */
    LPC_AON_MBIAS->MBIAS_REG0X_SD_TUNE_RO =  LPC_InitStruct->LPC_Threshold & 0x3F;
}

void LPC_CmdConfig(LPC_TypeDef *LPCx, FunctionalState NewState)
{
    LPC_CR0_TypeDef lpc_reg_0x00 = {.d32 = LPCx->LPC_CR};
    /* LPC output enable */
    lpc_reg_0x00.b.lpcomp0_ie = NewState;

    LPCx->LPC_CR =  lpc_reg_0x00.d32;
}

void LPC_INTSetting(LPC_TypeDef *LPCx, uint32_t LPC_INT, FunctionalState NewState)
{
    SoC_VENDOR->u_008.REG_LOW_PRI_INT_MODE &= ~BIT4;
    if (NewState != DISABLE)
    {
        SoC_VENDOR->u_00C.REG_LOW_PRI_INT_EN |= BIT4;
        /* Enable the selected LPC interrupt */
        LPCx->LPC_CR |= (LPC_INT);
    }
    else
    {
        SoC_VENDOR->u_00C.REG_LOW_PRI_INT_EN &= ~BIT4;
        /* Disable the selected LPC interrupt */
        LPCx->LPC_CR &= (~LPC_INT);
    }
}
void LPC_ClearFlagConfig(LPC_TypeDef *LPCx, uint32_t LPC_FLAG)
{
    /* Clear counter comparator interrupt */
    if (LPC_FLAG == LPC_FLAG_LPCOMP)
    {
        SoC_VENDOR->u_004.REG_LOW_PRI_INT_STATUS |= BIT4;
    }

}
ITStatus LPC_GetINTStatusConfig(LPC_TypeDef *LPCx, uint32_t LPC_INT)
{
    ITStatus int_status = RESET;

    if (LPC_INT == LPC_INT_LPCOMP)
    {
        if ((SoC_VENDOR->u_004.REG_LOW_PRI_INT_STATUS & BIT4) != (uint32_t)RESET)
        {
            int_status = SET;
        }
    }
    /* Return the LPC_INT status */
    return  int_status;
}
