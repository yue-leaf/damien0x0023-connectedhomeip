/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "rtl_rcc.h"
#include "rtl_pinmux.h"
#include "rtl_gdma.h"
#include "rtl_enh_tim.h"
#include "io_pwm_dma.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define PWM_OUT_P_PIN                     P0_0
#define PWM_PINMUX_OUT_P                  ENPWM0_P
#define PWM_OUT_N_PIN                     P0_1
#define PWM_PINMUX_OUT_N                  ENPWM0_N

#define Enhance_Timer_Num                 0
#define Enhance_Timer                     ENH_TIM0

#define PWM_MAXCNT                        100     /*< Setting this macro to 100 means the PWM period is 100 * 25ns. */

/* Gloables define -----------------------------------------------------------*/
#define Enhance_Timer_GDMA_CH_NUM      GDMA_CH_NUM0
#define Enhance_Timer_GDMA_Handshake   GDMA_Handshake_ENH_TIM0
#define Enhance_Timer_GDMA_CH_INDEX    GDMA_Channel0
/* Private function prototypes -----------------------------------------------*/
static uint32_t ccr_fifo_buf[] = {0x0A, 0x14, 0x1E, 0x32};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  None.
  * @retval None.
  */
void board_enhance_pwm_init(void)
{
    Pad_Config(PWM_OUT_P_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(PWM_OUT_N_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);

    Pinmux_Config(PWM_OUT_P_PIN, PWM_PINMUX_OUT_P);
    Pinmux_Config(PWM_OUT_N_PIN, PWM_PINMUX_OUT_N);
}

/**
  * @brief  Initialize GPIO peripheral.
  * @param  None.
  * @retval None.
  */
void enhance_timer_init(void)
{
    /* open enhtimer clock */
    RCC_PeriphClockCmd(APBPeriph_ENHTIMER, APBPeriph_ENHTIMER_CLOCK, ENABLE);

    ENHTIM_InitTypeDef ENHTIM_InitStruct;
    ENHTIM_StructInit(&ENHTIM_InitStruct);
    ENHTIM_InitStruct.ENHTIM_ClockDiv = ENHTIM_CLOCK_DIVIDER_1;

    ENHTIM_InitStruct.ENHTIM_Mode               = ENHTIM_MODE_PWM_AUTO;
    ENHTIM_InitStruct.ENHTIM_PWMOutputEn        = ENABLE;
    ENHTIM_InitStruct.ENHTIM_PWMStartPolarity   = ENHTIM_PWM_START_WITH_HIGH;
    ENHTIM_InitStruct.ENHTIM_MaxCount           = PWM_MAXCNT;

    ENHTIM_InitStruct.ENHTIM_PWMDeadZoneClockSource = ENHTIM_PWM_DZCLKSRCE_32K;
    ENHTIM_InitStruct.ENHTIM_PWMDeadZoneEn = ENABLE;
    ENHTIM_InitStruct.ENHTIM_PWMStopStateP = ENHTIM_PWM_STOP_AT_HIGH;
    ENHTIM_InitStruct.ENHTIM_PWMStopStateN = ENHTIM_PWM_STOP_AT_LOW;
    ENHTIM_InitStruct.ENHTIM_DeadZoneSize = 0x0;
    ENHTIM_InitStruct.ENHTIM_DmaEn = ENABLE;
    ENHTIM_InitStruct.ENHTIM_DmaTragget = ENHTIM_DMA_CCR_FIFO;

    ENHTIM_Init(Enhance_Timer, &ENHTIM_InitStruct);

    ENHTIM_Cmd(Enhance_Timer, ENABLE);
}

void driver_gdma_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = Enhance_Timer_GDMA_CH_NUM;

    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Word;

    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestHandshake       = Enhance_Timer_GDMA_Handshake;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_MemoryToPeripheral;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)ccr_fifo_buf;
    GDMA_InitStruct.GDMA_BufferSize          = 4;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t) & (Enhance_Timer->ENHTIM_CCR_FIFO_ENTRY);
    GDMA_InitStruct.GDMA_Secure_En           = ENABLE;
    GDMA_Init(Enhance_Timer_GDMA_CH_INDEX, &GDMA_InitStruct);

    GDMA_Cmd(Enhance_Timer_GDMA_CH_NUM, ENABLE);//FIRST open DMA ,then open timer
}


