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
#include "flash_nor_device.h"
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
#include "patch_header_check.h"
#include "trustzone_demo_nsc.h"

POWER_CheckResult dlps_allow = POWER_CHECK_PASS;
/*-----------------------------------------------------------*/


/**
 * @brief The mem fault handler.
 *
 * It calls a function called vHandleMemoryFault.
 */
void MemManage_Handler(void) __attribute__((naked));
/*-----------------------------------------------------------*/


void non_secure_callback(void)
{
    DBG_DIRECT("Non-Secure World: callback");
}

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

//    if (System_WakeUpInterruptValue(P2_7) == SET)
//    {
//        DBG_DIRECT("P2_7 Wake up");
//        Pad_ClearWakeupINTPendingBit(P2_7);
//        System_WakeUpPinDisable(P2_7);
//    }

//    if (WakeUpDebounceInterruptValue(P2_7) == SET)
//    {
//        DBG_DIRECT("P2_7 debounce Wake up");
//        System_WakeUpPinDisable(P2_7);
//    }

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

//    System_WakeUpDebounceTime(P2_7, PAD_DEB_FREQ_500Hz, 255);
//    System_WakeUpPinEnable(P2_7, PAD_WAKEUP_POL_HIGH, PAD_WAKEUP_DEB_ENABLE);

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
* @return true : allow enter dlps
 * @retval void
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


uint8_t get_active_bank_num(void)
{
    uint32_t addr = get_active_ota_bank_addr();
    uint32_t ota_bank0_addr = flash_nor_get_bank_addr(FLASH_OTA_BANK_0);
    uint32_t ota_bank1_addr = flash_nor_get_bank_addr(FLASH_OTA_BANK_1);
    if (addr == ota_bank0_addr)
    {
        return 0;
    }
    else if (addr == ota_bank1_addr)
    {
        return 1;
    }
    else
    {
        return 0xFF;
    }
}

/* Non-Secure main. */
int main(void)
{
    if (FEATURE_TRUSTZONE_ENABLE)
    {
        DBG_DIRECT("[NS] Non-Secure World: main, Active bank is bank%d", get_active_bank_num());
    }
    else
    {
        DBG_DIRECT("[NS] Secure World: main, Active bank is bank%d", get_active_bank_num());
    }

    /*======================================================================
     * TrustZone NSC Calling Demonstration
     *
     * Three patterns are shown in order of complexity.  Each call crosses
     * the S/NS boundary via the veneer stub in app_s_CMSE_Lib.o, which
     * contains the SG (Secure Gateway) instruction that performs the actual
     * security-state transition.
     *====================================================================*/

    /* ------------------------------------------------------------------
     * Mode 1: No-parameter NSC call
     *
     * The simplest possible NSC call.  No arguments, no return value.
     * Observe in the disassembly that the NS side calls the veneer stub
     * (BLXNS), which then executes SG and jumps into Secure World.
     * ------------------------------------------------------------------ */
    DBG_DIRECT("[NS] --- Mode 1: no-parameter NSC call ---");
    trustzone_demo_nsc();

    /* ------------------------------------------------------------------
     * Mode 2: Value-passing NSC call
     *
     * Scalar arguments (int32_t) are passed in CPU registers r0-r3.
     * The hardware automatically clears any registers not used for
     * arguments when crossing the boundary, so no explicit sanitisation
     * is needed on the Secure side.
     * ------------------------------------------------------------------ */
    DBG_DIRECT("[NS] --- Mode 2: value-passing NSC call ---");
    int32_t sum = trustzone_demo_nsc_add(10, 20);
    DBG_DIRECT("[NS] trustzone_demo_nsc_add(10, 20) = %d (expect 30)", sum);

    /* ------------------------------------------------------------------
     * Mode 3: Pointer-passing NSC call
     *
     * Buffers in NS memory are passed by pointer.  The Secure side MUST
     * validate every pointer with cmse_check_address_range() before use,
     * and copy the data onto the secure stack to prevent TOCTOU attacks.
     *
     * ns_input[i] ^ 0xAA is the transformation applied in Secure World,
     * so ns_output[0] should be 0x01 ^ 0xAA = 0xAB.
     * ------------------------------------------------------------------ */
    DBG_DIRECT("[NS] --- Mode 3: pointer-passing NSC call ---");
    uint8_t ns_input[8]  = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t ns_output[8] = {0x00};
    int32_t processed = trustzone_demo_nsc_process(
                            ns_input,  sizeof(ns_input),
                            ns_output, sizeof(ns_output));
    DBG_DIRECT("[NS] trustzone_demo_nsc_process: %d bytes processed", processed);
    DBG_DIRECT("[NS] ns_output[0]=0x%02x (expect 0xab)", ns_output[0]);
    DBG_DIRECT("[NS] ns_output[7]=0x%02x (expect 0xa2)", ns_output[7]);

    DBG_DIRECT("[NS] --- NSC demo complete, starting RTOS ---");

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

    /* Should not reach here as the scheduler is already started. */
    for (; ;)
    {
    }
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
