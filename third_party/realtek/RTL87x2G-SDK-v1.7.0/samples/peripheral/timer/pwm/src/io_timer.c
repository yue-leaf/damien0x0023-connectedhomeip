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
#include "utils.h"

#include "rtl_tim.h"
#include "io_timer.h"
#include "clock.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define PWM_OUT_PIN             P0_1

#define PWM_TIMER_NUM           TIM2
#define PWM_OUT_PIN_PINMUX      TIMER_PWM2

#define PWM_PERIOD              100000 //uint:us
#define PWM_DUTY_CYCLE          50     //uint:percent
#define PWM_HIGH_COUNT          ((((PWM_PERIOD)*(PWM_DUTY_CYCLE*40))/100)-1)
#define PWM_LOW_COUNT           ((((PWM_PERIOD)*((100-PWM_DUTY_CYCLE)*40))/100)-1)

#define CONFIG_TIM_CHANGE_SRC_CLK       1

/* Gloables define -----------------------------------------------------------*/
uint8_t LED_Status = 0;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  None.
  * @retval None.
  */
void board_pwm_init(void)
{
    Pad_Config(PWM_OUT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);

    Pinmux_Config(PWM_OUT_PIN, PWM_OUT_PIN_PINMUX);
}

/**
  * @brief  Initialize GPIO peripheral.
  * @param  None.
  * @retval None.
  */
void driver_pwm_init(void)
{
    /* TIMER peripheral clock enable */
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);

    /* Configure TIMER parameters */
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);
    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_InitStruct.TIM_PWM_En         = ENABLE;
    TIM_InitStruct.TIM_PWM_High_Count = PWM_HIGH_COUNT;
    TIM_InitStruct.TIM_PWM_Low_Count  = PWM_LOW_COUNT;
    TIM_TimeBaseInit(PWM_TIMER_NUM, &TIM_InitStruct);

    TIM_Cmd(PWM_TIMER_NUM, ENABLE);
}

void pwm_demo(void)
{
#if (CONFIG_TIM_CHANGE_SRC_CLK == 1)
    platform_delay_ms(1000);
    TIM_Cmd(PWM_TIMER_NUM, DISABLE);
    /* Change TIMER SRC CLK to PLL1(100MHz) */
    int32_t ret = pm_timer_freq_set(CLK_PLL1_SRC, 100, 100);
    DBG_DIRECT("ret = %d", ret);
    /* Set timer clock src pll1 to 8 divider (12.5MHz) */
    TIM_ClkConfig(PWM_TIMER_NUM, TIM_CLOCK_SRC_PLL1, TIM_CLOCK_DIVIDER_8);
    TIM_Cmd(PWM_TIMER_NUM, ENABLE);

    /* Get timer clock src */
    TIMClockDiv_TypeDef tim_clk_div = 10;
    TIMClockSrc_TypeDef tim_clk_src = 10;
    uint32_t actual_clock_src_freq = 0;
    TIM_ClkGet(PWM_TIMER_NUM, &tim_clk_src, &tim_clk_div);
    DBG_DIRECT("clk src = %d, clk div = %d", tim_clk_src, tim_clk_div);

    /* If clk src = PPL clock, it can use pm_clock_src_freq_get() to get PLL clock freq */
    if (tim_clk_src == TIM_CLOCK_SRC_PLL1)
    {
        actual_clock_src_freq = pm_clock_src_freq_get(CLK_PLL1_PERI);
    }
    else if (tim_clk_src == TIM_CLOCK_SRC_PLL2)
    {
        actual_clock_src_freq = pm_clock_src_freq_get(CLK_PLL2);
    }

    DBG_DIRECT("actual_clock_src_freq = %d", actual_clock_src_freq);

    /* After get clock src freq, it can calculate actual peripheral clock.
       It can refer to the definition of the enumerated variables in TIMClockDiv_TypeDef
       to find the correspondence between the enumerated values and the actual frequency.
       For example, if clock src = 100MHz(PLL1), clock div = 0x3, which means TIM_CLOCK_DIVIDER_8,
       so the TIM freq = 100MHz / 8 = 12.5MHz.
     */
#endif
}


