/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Demo includes. */
#include "stdlib.h"
#include "rtl876x.h"
#include "rtl876x_lib_platform.h"
#include "app_section.h"
#include "app_task.h"
#include "pm.h"
#include "board.h"
#if (USE_OSIF == 1)
#include "os_sched.h"
#else
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#endif
#include "io_dlps.h"
#include "trace.h"
#include "mem_config.h"
#include "rtl_pinmux.h"



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

    if (System_WakeUpInterruptValue(P2_7) == SET)
    {
        DBG_DIRECT("P2_7 Wake up");
        Pad_ClearWakeupINTPendingBit(P2_7);
        System_WakeUpPinDisable(P2_7);
    }

    if (System_WakeupDebounceStatus(P2_7) == SET)
    {
        DBG_DIRECT("debounce Wake up");
    }
}


/**
 * @brief this function will be called before enter DLPS
 *
 *  set PAD and wakeup pin config for entering DLPS
 *
 * @param none
 * @return none
 * @retval void
*/
RAM_FUNCTION
void app_enter_dlps_config(void)
{
    DBG_DIRECT("DLPS ENTER");

    Pad_ClearAllWakeupINT();
    System_WakeupDebounceStatus(P2_7);

//    System_WakeUpPinDisable(P2_7);
//    System_WakeUpDebounceTime(P2_7, 63);
//    Pad_Config(P2_7, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
//    System_WakeUpPinEnable(P2_7, PAD_WAKEUP_POL_LOW, PAD_WAKEUP_DEB_ENABLE);

//    Pad_Config(P2_7, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
//    System_WakeUpPinEnable(P2_7, PAD_WAKEUP_POL_LOW, PAD_WAKEUP_DEB_DISABLE);

}

/**
 * @brief this function will be called after exit DLPS
 *
 *  set PAD and wakeup pin config for exiting DLPS
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

/**
 * @brief DLPS CallBack function
 * @param none
 * @return POWER_CHECK_PASS : allow enter dlps, POWER_CHECK_FAIL : deny enter dlps
*/
RAM_FUNCTION
POWER_CheckResult app_dlps_check_cb(void)
{
    if (allow_count >= 20)
    {
        dlps_allow = POWER_CHECK_FAIL;
    }
    return dlps_allow;
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
    power_check_cb_register(app_dlps_check_cb);
    DLPS_IORegUserDlpsEnterCb(app_enter_dlps_config);
    DLPS_IORegUserDlpsExitCb(app_exit_dlps_config);
    DLPS_IORegister();
    bt_power_mode_set(BTPOWER_DEEP_SLEEP);
    power_mode_set(POWER_DLPS_MODE);
#else
#endif
}

/* Non-Secure main. */
int main(void)
{
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

    task_init();
    sw_timer_init();

    pwr_mgr_init();

    /* Start scheduler. */
#if (USE_OSIF == 1)
    os_sched_start();
#else
    vTaskStartScheduler();
#endif

    return 0;
}

#if (USE_OSIF == 0)
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that
 * is used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
     * function then they must be declared static - otherwise they will be
     * allocated on the stack and so not exist after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ] __attribute__((aligned(32)));

    /* Pass out a pointer to the StaticTask_t structure in which the Idle
     * task's state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
     * function then they must be declared static - otherwise they will be
     * allocated on the stack and so not exist after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ] __attribute__((aligned(32)));

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
/*-----------------------------------------------------------*/
#if configENABLE_MPU
void MemManage_Handler(void)
{
    __asm volatile
    (
        " tst lr, #4                                      \n"
        " ite eq                                          \n"
        " mrseq r0, msp                                       \n"
        " mrsne r0, psp                                       \n"
        " ldr r1, handler_address_const                       \n"
        " bx r1                                               \n"
        "                                                 \n"
        " handler_address_const: .word vHandleMemoryFault \n"
    );
}
#endif
#endif
/*-----------------------------------------------------------*/
