/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
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
#include "sd_test.h"


/* Private define ------------------------------------------------------------*/
POWER_CheckResult dlps_allow = POWER_CHECK_PASS;
uint32_t allow_count = 0;


/* Private functions ---------------------------------------------------------*/
/**
  * @brief The mem fault handler.
  * @param   none
  * @note  It calls a function called vHandleMemoryFault.
  */
void MemManage_Handler(void) __attribute__((naked));

/**
  * @brief   Contains the initialization of pinmux settings and pad settings
  * @note    All the pinmux settings and pad settings shall be initiated in this function,
  *          but if legacy driver is used, the initialization of pinmux setting and pad setting
  *          should be peformed with the IO initializing.
  * @param   none
  * @return  void
  */
void board_init(void)
{

}

/**
  * @brief   Contains the initialization of peripherals
  * @note    Both new architecture driver and legacy driver initialization method can be used
  * @param   none
  * @return  void
  */
void driver_init(void)
{
    SD_Test();
}

/**
  * @brief   System_Handler
  * @note    system handle to judge which pin is wake source
  * @param   none
  * @return  void
  */
RAM_FUNCTION
void System_Handler(void)
{
    DBG_DIRECT("SYSTEM_HANDLER");
}


/**
  * @brief   This function will be called before enter DLPS
  * @note    set PAD and wakeup pin config for enterring DLPS
  * @param   none
  * @return  void
  */
RAM_FUNCTION
void app_enter_dlps_config(void)
{
    DBG_DIRECT("DLPS ENTER");
}

/**
  * @brief   This function will be called after exit DLPS
  * @note    Set PAD and wakeup pin config for enterring DLPS
  * @param   none
  * @return  void
  */
RAM_FUNCTION
void app_exit_dlps_config(void)
{
    allow_count ++;
    DBG_DIRECT("DLPS EXIT, wake up reason 0x%x", power_get_wakeup_reason());
}

/**
  * @brief   DLPS CallBack function
  * @param   none
  * @return  PMCheckResult: allow enter dlps
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


/**
  * @brief   pwr_mgr_init() contains the setting about power mode.
  * @param   none
  * @return  void
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
#endif
}

/**
  * @brief   Contains the initialization of all tasks
  * @note    There is only one task in BLE Peripheral APP, thus only one APP task is init here
  * @param   none
  * @return  void
  */
void task_init(void)
{
    app_task_init();
}

/**
  * @brief  Non-Secure Main program.
  * @param  None.
  * @retval None.
  */
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
    DBG_DIRECT("APP main: max free block 0x%x", os_mem_peek_max_free_block(RAM_TYPE_DATA_ON));

    extern uint32_t random_seed_value;
    srand(random_seed_value);
    board_init();
    pwr_mgr_init();
    app_task_init();
    os_sched_start();

    /* Should not reach here as the scheduler is already started. */
    while (1);
}


