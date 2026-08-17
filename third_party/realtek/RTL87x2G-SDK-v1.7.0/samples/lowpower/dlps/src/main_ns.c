/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <string.h>
#include <stdlib.h>

#include "trace.h"
#include "os_sched.h"
#include "io_dlps.h"
#include "io_adc.h"
#include "io_uart.h"
#include "io_rtc.h"
#include "io_aon_qdec.h"
#include "io_lpc.h"

/** @defgroup  PERIPH_DEMO_MAIN Peripheral Main
    * @brief Main file to initialize hardware and BT stack and start task scheduling
    * @{
    */

/*============================================================================*
 *                              Constants
 *============================================================================*/
#define CONFIG_ADC_GPIO_DLPS_EN         1
#define CONFIG_RTC_DLPS_EN              0
#define CONFIG_UART_DLPS_EN             0
#define CONFIG_AON_QDEC_DLPS_EN         0
#define CONFIG_LPC_DLPS_EN              0

/*============================================================================*
 *                              Variables
 *============================================================================*/

/*============================================================================*
 *                              Functions
 *============================================================================*/

#if DLPS_EN
/**
  * @brief this function will be called before enter DLPS
  *
  *  set PAD and wakeup pin config for enterring DLPS
  *
  * @param  none
  * @return none
  * @retval void
 */
void app_enter_dlps_config(void)
{
#if CONFIG_ADC_GPIO_DLPS_EN
    io_adc_gpio_dlps_enter();
#endif

#if CONFIG_RTC_DLPS_EN
    io_rtc_dlps_enter();
#endif

#if CONFIG_UART_DLPS_EN
    io_uart_dlps_enter();
#endif

#if CONFIG_QDEC_DLPS_EN
    io_aon_qdec_dlps_enter();
#endif

#if CONFIG_LPC_DLPS_EN
    io_lpc_dlps_enter();
#endif

}

/**
  * @brief this function will be called after exit DLPS
  *
  *  set PAD and wakeup pin config for enterring DLPS
  *
  * @param none
  * @return none
  * @retval void
  */
void app_exit_dlps_config(void)
{
#if CONFIG_ADC_GPIO_DLPS_EN
    io_adc_gpio_dlps_exit();
#endif

#if CONFIG_RTC_DLPS_EN
    io_rtc_dlps_exit();
#endif

#if CONFIG_UART_DLPS_EN
    io_uart_dlps_exit();
#endif

#if CONFIG_QDEC_DLPS_EN
    io_aon_qdec_dlps_exit();
#endif

#if CONFIG_LPC_DLPS_EN
    io_lpc_dlps_exit();
#endif
}

/**
  * @brief  Contains the setting about app dlps callback.
  */
POWER_CheckResult app_dlps_check_cb(void)
{
    POWER_CheckResult ret = POWER_CHECK_FAIL;

#if CONFIG_ADC_GPIO_DLPS_EN
    ret = io_adc_gpio_dlps_check();
#endif

#if CONFIG_RTC_DLPS_EN
    ret = io_rtc_dlps_check();
#endif

#if CONFIG_UART_DLPS_EN
    ret = io_uart_dlps_check();
#endif

#if CONFIG_QDEC_DLPS_EN
    ret = io_aon_qdec_dlps_check();
#endif

#if CONFIG_LPC_DLPS_EN
    ret = io_lpc_dlps_check();
#endif
    return ret;
}

#endif /* DLPS_EN */

void pwr_mgr_init(void)
{
#if DLPS_EN
    power_check_cb_register(app_dlps_check_cb);
    DLPS_IORegUserDlpsEnterCb(app_enter_dlps_config);
    DLPS_IORegUserDlpsExitCb(app_exit_dlps_config);
    DLPS_IORegister();
    bt_power_mode_set(BTPOWER_DEEP_SLEEP);
    power_mode_set(POWER_DLPS_MODE);

#else
    platform_pm_set_power_mode(PLATFORM_ACTIVE);
#endif
}

void global_data_init(void)
{
#if CONFIG_ADC_GPIO_DLPS_EN
    global_data_adc_gpio_init();
#endif

#if CONFIG_UART_DLPS_EN
    global_data_uart_init();
#endif
}

void board_init(void)
{
#if CONFIG_ADC_GPIO_DLPS_EN
    board_adc_gpio_init();
#endif

#if CONFIG_UART_DLPS_EN
    board_uart_init();
#endif

#if CONFIG_AON_QDEC_DLPS_EN
    board_qdec_init();
#endif

#if CONFIG_LPC_DLPS_EN
    board_lpc_init();
#endif
}

/**
 * @brief    Contains the initialization of peripherals
 * @note     Both new architecture driver and legacy driver initialization method can be used
 * @return   void
 */
void driver_init(void)
{
#if CONFIG_ADC_GPIO_DLPS_EN
    /* Initialize adc k value! */
    APP_PRINT_INFO0("[io_adc] global_data_adc_init");
    bool adc_k_status = false;
    adc_k_status = ADC_CalibrationInit();
    if (false == adc_k_status)
    {
        APP_PRINT_ERROR0("[io_adc] global_data_adc_init: ADC_CalibrationInit fail!");
    }

    driver_adc_init();
    driver_gpio_init();
#endif

#if CONFIG_UART_DLPS_EN
    driver_uart_init();
#endif

#if CONFIG_RTC_DLPS_EN
    driver_rtc_init();
#endif

#if CONFIG_AON_QDEC_DLPS_EN
    driver_qdec_init();
#endif

#if CONFIG_LPC_DLPS_EN
    driver_lpc_init();
#endif
}


/**
 * @brief    Entry of APP code
 * @return   int (To avoid compile warning)
 */
int main(void)
{
    DBG_DIRECT("enter main");
    global_data_init();
    board_init();
    driver_init();
    pwr_mgr_init();
    os_sched_start();

}

/**
  * @brief  System interrupt handler function, for wakeup pin.
  * @param  No parameter.
  * @return void
*/
void System_Handler(void)
{
    APP_PRINT_INFO0("[main] System_Handler");
#if CONFIG_ADC_GPIO_DLPS_EN
    if (System_WakeUpInterruptValue(ADC_GPIO_DLPS_WAKEUP_PIN) == SET)
    {
        Pad_ClearWakeupINTPendingBit(ADC_GPIO_DLPS_WAKEUP_PIN);
        System_WakeUpPinDisable(ADC_GPIO_DLPS_WAKEUP_PIN);
        IO_ADC_GPIO_DLPS_Enter_Allowed = POWER_CHECK_FAIL;
    }
#endif

#if CONFIG_UART_DLPS_EN
    if (System_WakeUpInterruptValue(UART_RX_PIN) == SET)
    {
        System_WakeUpPinDisable(UART_RX_PIN);
        Pad_ClearWakeupINTPendingBit(UART_RX_PIN);
        IO_UART_DLPS_Enter_Allowed = POWER_CHECK_FAIL;
    }
#endif

}

/** @} */ /* End of group PERIPH_DEMO_MAIN */
