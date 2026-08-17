/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
/* Demo includes. */
#include "rtl876x.h"
#include "rtl876x_lib_platform.h"
#include "app_section.h"
#include "app_task.h"
#include "pm.h"
#include "board.h"
#include "os_mem.h"
#include "os_sched.h"
#include "io_dlps.h"
#include "trace.h"
#include "mem_config.h"
#include "rtl_gpio.h"
#include "rtl_nvic.h"
#include "rtl_pinmux.h"
#include "rtl_rcc.h"


#define CPU_125M_CLK    0
POWER_CheckResult dlps_allow = POWER_CHECK_PASS;
/*-----------------------------------------------------------*/


/**
 * @brief The mem fault handler.
 *
 * It calls a function called vHandleMemoryFault.
 */
void MemManage_Handler(void) __attribute__((naked));
/*-----------------------------------------------------------*/

#if (DLPS_EN == 1)
/**
 * @brief    System_Handler
 * @note     system handle to judge which pin is wake source
 * @return   void
 */
RAM_FUNCTION
void System_Handler(void)
{
    DBG_DIRECT("SYSTEM_HANDLER");

}


/**
 * @brief this function will be called before enter DLPS
 *
 *  set PAD and wakeup pin config for enterring DLPS
 *
 * @param none
 * @return none
 * @retval void
*/
RAM_FUNCTION
void app_enter_dlps_config(void)
{
    DBG_DIRECT("DLPS ENTER");

//    System_WakeUpDebounceTime(P2_7, PAD_DEB_FREQ_500Hz, 255);
//    System_WakeUpPinEnable(P2_7, PAD_WAKEUP_POL_HIGH, PAD_WAKEUP_DEB_ENABLE);

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
uint32_t allow_count = 0;

RAM_FUNCTION
void app_exit_dlps_config(void)
{
    allow_count ++;
    DBG_DIRECT("DLPS EXIT, wake up reason 0x%x", power_get_wakeup_reason());
}


#endif


/******************************************************************
 * @brief  pwr_mgr_init() contains the setting about power mode.
 * @param  none
 * @return none
 * @retval void
 */
void pwr_mgr_init(void)
{
#if (DLPS_EN == 1)
//    extern  POWER_CheckResult usb_dm_power_check(void);
//    power_check_cb_register(usb_dm_power_check));
    DLPS_IORegUserDlpsEnterCb(app_enter_dlps_config);
    DLPS_IORegUserDlpsExitCb(app_exit_dlps_config);
    DLPS_IORegister();
    bt_power_mode_set(BTPOWER_DEEP_SLEEP);
    power_mode_set(POWER_DLPS_MODE);
#else
#endif
}

#if (REMOTE_WAKEUP_EN == 1)
/* Private define ------------------------------------------------------------*/
#define INPUT_PIN           P0_6
#define GPIO_PIN            GPIO_GetPin(INPUT_PIN)
#define GPIO_PORT           GPIO_GetPort(INPUT_PIN)
#define GPIO_PIN_IRQN       GPIOA6_IRQn
#define GPIO_Pin_Handler    GPIOA6_Handler

void gpio_init(void)
{
    Pad_Config(INPUT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);

    Pinmux_Config(INPUT_PIN, DWGPIO);
    /* GPIO peripheral clock enable */
    RCC_PeriphClockCmd(APBPeriph_GPIOA, APBPeriph_GPIOA_CLOCK, ENABLE);

    /* Configure GPIO_PIN_OUTPUT as output mode */
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin        = GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode       = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd      = ENABLE;
    GPIO_InitStruct.GPIO_ITTrigger  = GPIO_INT_Trigger_EDGE;
    GPIO_InitStruct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_LOW;

    /* Enable NVIC*/
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GPIO_PIN_IRQN;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    /* Unmask and enable interrupt */
    GPIO_MaskINTConfig(GPIO_PORT, GPIO_PIN, DISABLE);
    GPIO_INTConfig(GPIO_PORT, GPIO_PIN, ENABLE);


    Pad_Config(P3_1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pinmux_Config(P3_1, DWGPIO);  //SWDIO

    GPIO_InitTypeDef GPIO_Wave;
    GPIO_StructInit(&GPIO_Wave);
    GPIO_Wave.GPIO_Mode   = GPIO_MODE_OUT;
    GPIO_Wave.GPIO_ITCmd  = DISABLE;
    GPIO_Wave.GPIO_Pin    = GPIO_GetPin(P3_1);
    GPIO_Init(GPIO_GetPort(P3_1), &GPIO_Wave);
    GPIO_ResetBits(GPIO_GetPort(P3_1), GPIO_GetPin(P3_1));
}

void GPIO_Pin_Handler(void)
{
    /* Mask and disable interrupt */
    GPIO_INTConfig(GPIO_PORT, GPIO_PIN, DISABLE);
    GPIO_MaskINTConfig(GPIO_PORT, GPIO_PIN, ENABLE);

    /** User code here.
      * for example: print enter handler success iofo */
    DBG_DIRECT("enter GPIO_Pin_Handler success");

    usb_hid_driver_remote_wakeup();
    extern  bool app_cfu_usb_send_data(uint8_t report_id, uint8_t *data, uint16_t len);
    uint8_t Report[7] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t ReportLen = 7;
    app_cfu_usb_send_data(0x01, Report, ReportLen);

    /* Clear int flag, unmask and enable interrupt */
    GPIO_ClearINTPendingBit(GPIO_PORT, GPIO_PIN);
    GPIO_MaskINTConfig(GPIO_PORT, GPIO_PIN, DISABLE);
//    GPIO_INTConfig(GPIO_PORT, GPIO_PIN, ENABLE);
}
#endif
/* Non-Secur? e main. */
int main(void)
{
    __enable_irq();
    if (FEATURE_TRUSTZONE_ENABLE)
    {
        DBG_DIRECT("Non-Secure World: main");
    }
    else
    {
        DBG_DIRECT("Secure World: main");
    }

    extern uint32_t random_seed_value;
    srand(random_seed_value);

    DBG_DIRECT("APP main: max free block 0x%x", os_mem_peek_max_free_block(RAM_TYPE_DATA_ON));

    task_init();
#if (REMOTE_WAKEUP_EN == 1)
    gpio_init();
#endif
#if CPU_125M_CLK
    uint32_t actual_mhz = 0;
    pm_cpu_freq_set(125, &actual_mhz);
    APP_PRINT_INFO1("CPU actual clock is %d MHz", actual_mhz);
#endif
    sw_timer_init();

#if (TEST_OS == 1)
    extern void test_os_function(void);
    test_os_function();
#endif

    pwr_mgr_init();

    /* Start scheduler. */
#if (USE_OSIF == 1)
    os_sched_start();
#else
    vTaskStartScheduler();
#endif

    /* Should not reach here as the scheduler is already started. */
    for (; ;)
    {
    }
}
/*-----------------------------------------------------------*/
