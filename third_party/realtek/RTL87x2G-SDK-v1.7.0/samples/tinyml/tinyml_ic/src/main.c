/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <stdlib.h>
#include <os_sched.h>
#include <trace.h>
#include "board.h"
#include "mem_config.h"
#include "pm.h"
#include "rtl_nvic.h"
#include "io_dlps.h"
#include "app_section.h"
#include "fmc_api.h"

POWER_CheckResult dlps_allow = POWER_CHECK_PASS;

/* ============================================================
 * Bounded _sbrk override (GCC newlib).
 * After malloc_override.c redirects all malloc/free to ucHeap,
 * _sbrk should receive zero calls.  Any remaining call is from
 * code that bypasses malloc (e.g. internal newlib init).  The
 * bounded version prevents those calls from conflicting with
 * ROM's pvPortMalloc which also manages NS_HEAP from 0x126400.
 * ============================================================ */
#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
#include <errno.h>
#include <sys/types.h>
extern char _sheap;    /* defined by app.ld: start of .heap region */
extern char _eheap;    /* defined by app.ld: end   of .heap region */
static char *g_heap_end = 0;

caddr_t _sbrk(int incr)
{
    char *prev;
    if (g_heap_end == 0) { g_heap_end = &_sheap; }
    if (g_heap_end + incr > &_eheap)
    {
        errno = 12; /* ENOMEM */
        return (caddr_t) - 1;
    }
    prev = g_heap_end;
    g_heap_end += incr;
    return (caddr_t)prev;
}
#endif

/** @defgroup  PERIPH_DEMO_MAIN Peripheral Main
    * @brief Main file to initialize hardware and BT stack and start task scheduling
    * @{
    */


/*============================================================================*
 *                              Variables
 *============================================================================*/


/**
 * @brief    Contains the initialization of pinmux settings and pad settings
 * @note     All the pinmux settings and pad settings shall be initiated in this function,
 *           but if legacy driver is used, the initialization of pinmux setting and pad setting
 *           should be peformed with the IO initializing.
 * @return   void
 */
void board_init(void)
{

}

/**
 * @brief    Contains the initialization of peripherals
 * @note     Both new architecture driver and legacy driver initialization method can be used
 * @return   void
 */
void driver_init(void)
{

}

/**
 * @brief    System_Handler
 * @note     system handle to judge which pin is wake source
 * @return   void
 */
RAM_FUNCTION
void System_Handler(void)
{
//    DBG_DIRECT("SYSTEM_HANDLER 0x%x", HAL_READ32(SOC_VENDOR2_REG_BASE, 0x0058));

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

//    HAL_WRITE32(SOC_VENDOR2_REG_BASE, 0x0058, 0x000001FF);
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
//    DBG_DIRECT("DLPS ENTER");

#if USE_PSRAM
#if USE_PSRAM_DEEP_POWER_HALF_SLEEP_MODE
    fmc_psram_wb_set_partial_refresh(FMC_FLASH_NOR_IDX1, FMC_PSRAM_WB_REFRESH_TOP_1_2);
    fmc_psram_enter_lpm(FMC_FLASH_NOR_IDX1, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#else
    fmc_psram_enter_lpm(FMC_FLASH_NOR_IDX1, FMC_PSRAM_LPM_DEEP_POWER_DOWN_MODE);
#endif
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
RAM_FUNCTION
void app_exit_dlps_config(void)
{
//    DBG_DIRECT("DLPS EXIT");
#if USE_PSRAM
    //fmc_pad_ctrl_in_lps_mode(FMC_FLASH_NOR_IDX1, false);
#if USE_PSRAM_DEEP_POWER_HALF_SLEEP_MODE
    fmc_psram_exit_lpm(FMC_FLASH_NOR_IDX1, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#else
    fmc_psram_exit_lpm(FMC_FLASH_NOR_IDX1, FMC_PSRAM_LPM_DEEP_POWER_DOWN_MODE);
#endif
#endif
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
    return false;
    //return dlps_allow;
}

/**
 * @brief    Contains the power mode settings
 * @return   void
 */
void pwr_mgr_init(void)
{
#if DLPS_EN
    power_check_cb_register(app_dlps_check_cb);
    DLPS_IORegUserDlpsEnterCb(app_enter_dlps_config);
    DLPS_IORegUserDlpsExitCb(app_exit_dlps_config);
    DLPS_IORegister();
    bt_power_mode_set(BTPOWER_DEEP_SLEEP);
    power_mode_set(POWER_DLPS_MODE);
#endif
}

/**
 * @brief    Contains the initialization of all tasks
 * @note     There is only one task in BLE Peripheral APP, thus only one APP task is init here
 * @return   void
 */
void task_init(void)
{
}

/**
 * @brief    Entry of APP code
 * @return   int (To avoid compile warning)
 */
int main(void)
{
    /* Clear CCR.UNALIGN_TRP (bit 3). ROM strlen uses LDM/LDRD on
     * word boundaries; UNALIGN_TRP set causes faults in ROM code. */
    {
        volatile uint32_t *p_ccr = (volatile uint32_t *)0xE000ED14;
        *p_ccr &= ~(1UL << 3);   /* clear UNALIGN_TRP */
        __asm volatile("dsb 0xF":::"memory");
        __asm volatile("isb 0xF":::"memory");
    }

    extern uint32_t random_seed_value;
    srand(random_seed_value);

    board_init();
    pwr_mgr_init();
    extern void TinyML_task_init();
    TinyML_task_init();
    os_sched_start();

    return 0;
}
/** @} */ /* End of group PERIPH_DEMO_MAIN */


