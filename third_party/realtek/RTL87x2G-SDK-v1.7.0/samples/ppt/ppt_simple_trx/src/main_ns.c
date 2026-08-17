/**
*****************************************************************************************
*     Copyright(c) 2022, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
* @file
* @brief
* @author
* @date
* @version
**************************************************************************************
* @attention
* <h2><center>&copy; COPYRIGHT 2022 Realtek Semiconductor Corporation</center></h2>
**************************************************************************************
*/

/* Demo includes. */
#include "rtl876x.h"
#include "rtl876x_lib_platform.h"
#include "rtl_pinmux.h"
#include "app_section.h"
#include "app_task.h"
#include "pm.h"
#include "io_dlps.h"
#include "utils.h"
#include "board.h"
#include "patch_header_check.h"
#include "flash_nor_device.h"
#include "os_sched.h"
#include "trace.h"
#include "ppt_driver.h"

void non_secure_callback(void)
{
    DBG_DIRECT("Non-Secure World: callback");
}

#if DLPS_EN
DATA_RAM_FUNCTION POWER_CheckResult app_dlps_check_cb(void)
{
    DBG_DIRECT("CHECK");
    return POWER_CHECK_PASS;
}

DATA_RAM_FUNCTION void app_dlps_enter_cb(void)
{
    DBG_DIRECT("ENTER");
}

DATA_RAM_FUNCTION void app_dlps_exit_cb(void)
{
    DBG_DIRECT("EXIT");
}

DATA_RAM_FUNCTION void System_Handler(void)
{
    DBG_DIRECT("SYS");
    Pad_ClearAllWakeupINT();
}

/******************************************************************
 * @brief  pwr_mgr_init() contains the setting about power mode.
 * @param  none
 * @return none
 * @retval void
 */
void pwr_mgr_init(void)
{
    /* ppt dlps init */
    ppt_dlps_init();
    /* platform dlps init */
    power_check_cb_register(app_dlps_check_cb);
    DLPS_IORegUserDlpsEnterCb(app_dlps_enter_cb);
    DLPS_IORegUserDlpsExitCb(app_dlps_exit_cb);
    DLPS_IORegister();
    bt_power_mode_set(BTPOWER_DEEP_SLEEP);
    power_mode_set(POWER_DLPS_MODE);
}
#endif

/* Non-Secure main. */
int main(void)
{
    DBG_DIRECT("Non-Secure World: main");

#if DLPS_EN
    pwr_mgr_init();
#endif

    app_task_init();

    /* Start scheduler. */
    os_sched_start();

    /* Should not reach here as the scheduler is already started. */
    for (; ;)
    {
    }
}

