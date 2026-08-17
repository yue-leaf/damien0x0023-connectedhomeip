/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef __PM_H_
#define __PM_H_


/*============================================================================*
 *                               Header Files
*============================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include "clock.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @defgroup POWER_MANAGER Power Manager
  * @brief
  * @{
  */

/*============================================================================*
 *                              Types
*============================================================================*/
/** @defgroup POWER_MANAGER_Exported_Types Power Manager Exported Types
  * @{
  */

/** @brief This enum is used for DLPS callback check. */
typedef enum POWER_CheckResult
{
    POWER_CHECK_PEND   = 0, /**< Callback check failed, but restart DLPS check flow.  */
    POWER_CHECK_FAIL   = 1, /**< Callback check failed, and CPU enters sleep mode.  */
    POWER_CHECK_PASS   = 2, /**< Callback check passed, and continue the other check callback functions which have been registered before.  */
} POWER_CheckResult;

/** @brief BT power mode struct. */
typedef enum
{
    BTPOWER_DEEP_SLEEP        = 0,   /**< BT deep sleep mode, DSM. */
    BTPOWER_ACTIVE            = 1,   /**< BT active mode. */
} T_BT_POWER_MODE;


/** @brief Power stage struct. */
typedef enum
{
    POWER_STAGE_STORE      = 0,   /**< Stage of entering DLPS.  */
    POWER_STAGE_RESTORE    = 1,   /**< Stage of exiting DLPS.  */
} POWERStage;

/** @brief Platform power mode struct. */
typedef enum
{
//    POWER_POWEROFF_MODE    = 0,
    POWER_POWERDOWN_MODE   = 1,   /**< Power down mode, can be woken up only by PAD and LPC, PAD debounce wake-up must be disabled in power-down mode. */
    POWER_DLPS_MODE        = 2,   /**< DLPS mode, deep low power state.  */
    POWER_ACTIVE_MODE      = 3,   /**< Active mode, system will keep in active mode.  */
    POWER_MODE_MAX         = 4    /**< Max power mode number. */
} T_SYSTEM_POWER_MODE;

/** @brief Sub mode of DLPS. */
typedef enum
{
    POWER_DLPS_PFM_MODE = 0,    /**< DLPS PFM mode. */
    POWER_DLPS_RET_MODE = 1,    /**< DLPS LDO mode. */
} POWERDlpsMode;

/** @brief Wake-up reason of DLPS. */
typedef enum
{
    PM_WAKEUP_UNKNOWN              = 0x00000000,  /**< Unknown wake up from DLPS.            */
    PM_WAKEUP_PRE_SYSTEM_LEVEL     = 0x00000001,  /**< Other power module wake up from DLPS. */
    PM_WAKEUP_PF_RTC               = 0x00000100,  /**< Platform RTC wake up from DLPS.       */
    PM_WAKEUP_RTC                  = 0x00000200,  /**< User RTC wake up from DLPS.           */
    PM_WAKEUP_MAC                  = 0x00000400,  /**< MAC wake up from DLPS.                */
    PM_WAKEUP_GPIO                 = 0x00000800,  /**< GPIO wake up from DLPS.               */
    PM_WAKEUP_REASON_USB           = 0x00001000,  /**< USB wake up from DLPS.                */
    PM_WAKEUP_REASON_LPCOMP        = 0x00002000,  /**< LPC wake up from DLPS.                */
    PM_WAKEUP_REASON_AON_QUAD_DEC  = 0x00004000,  /**< AON QDecoder wake up from DLPS.       */
    PM_WAKEUP_DEF_MAX              = 0xFFFFFFFF,  /**< Max wake up reason number.             */
} PowerModeWakeupReason;

/** @brief Error code of not entering DLPS. */
typedef enum
{
    PM_ERROR_UNKNOWN               = 0x0,   /**< Unknown error.                            */
    PM_ERROR_POWER_MODE            = 0x1,   /**< Power mode is not low power mode.        */
    PM_ERROR_DISABLE_DLPS_TIME     = 0x2,   /**< The number of entries and exits of DLPS has exceeded the maximum allowed limit, when max allow count is set.  */
    PM_ERROR_32K_CHECK_LOCK        = 0x3,   /**< 32k clock check fail.                    */
    PM_ERROR_LOG_DMA_NOT_IDLE      = 0x4,   /**< Log DMA is not idle.                     */
    PM_ERROR_LOG_BUFFER_NOT_EMPTY  = 0x5,   /**< Log buffer is not empty.                 */
    PM_ERROR_CALLBACK_CHECK        = 0x6,   /**< Check callback function return false.    */
    PM_ERROR_INTERRUPT_OCCURRED    = 0x7,   /**< Interrupt occurred in check flow.        */
    PM_ERROR_WAKEUP_TIME           = 0x8,   /**< Next wake-up time interval is too small. */
    PM_ERROR_ERROR_OCCUPIED_TIME   = 0x9,   /**< Power manager can not request next wake-up time interval as power manager slave is currently occupied.   */
} PowerModeErrorCode;

/** @brief Exclude type for power manager. */
typedef enum
{
    PM_EXCLUDED_TIMER,            /**< Timer exclude type for power manager. */
    PM_EXCLUDED_TASK,             /**< Task exclude type for power manager. */
    PM_EXCLUDED_TYPE_MAX,         /**< Max exclude type number for power manager. */
} PowerModeExcludedHandleType;

/** @brief This callback function is used for modules, which need to be checked before entering DLPS. */
typedef POWER_CheckResult(*POWERCheckFunc)();

/** @brief This callback function is used for modules, which need to be executed before entering or after exiting from DLPS. */
typedef void (*POWERStageFunc)();

/** @} */ /* End of group POWER_MANAGER_Exported_Types */

/**
  * \cond     private
  * \defgroup POWER_MANAGER_Private_Types
  * \{
  */

typedef enum
{
    PM_SUCCESS                         = 0x0,
    PM_DVFS_BUSY                       = -0x1,
    PM_DVFS_VOLTAGE_FAIL               = -0x2,
    PM_DVFS_CONDITION_FAIL             = -0x4,
    PM_DVFS_SRAM_FAIL                  = -0x8,
    PM_DVFS_NOT_SUPPORT                = -0x10,
} PMErrorCode;

/**
  *  End of POWER_MANAGER_Private_Types
  * \}
  * \endcond
  */

/*============================================================================*
 *                              Functions
*============================================================================*/
/** @defgroup POWER_MANAGER_Exported_Functions Power Manager Exported Functions
  * @{
  */

/**
 * @brief  Set platform power mode.
 * @param[in]  mode   Platform power mode, @ref T_SYSTEM_POWER_MODE.
 *
 * @return            Operation result, and will always return 0.
 * @retval 0          Success.
*/
int32_t power_mode_set(T_SYSTEM_POWER_MODE mode);

/**
 * @brief  Get platform power mode.
 *
 * @return Platform power mode, @ref T_SYSTEM_POWER_MODE.
*/
T_SYSTEM_POWER_MODE power_mode_get(void);

/**
 * @brief  Select DLPS sub mode, default value is @ref POWER_DLPS_PFM_MODE.
 * @param[in]  mode DLPS sub mode, @ref POWERDlpsMode.
 *
 * @return          Operation result, and will always return 0.
 * @retval 0        Success.
 * @note   Switch DLPS sub mode between @ref POWER_DLPS_PFM_MODE and @ref POWER_DLPS_RET_MODE.
*/
int32_t power_dlps_mode_select(POWERDlpsMode mode);

/**
 * @brief  Get DLPS sub mode.
 *
 * @return Current DLPS sub mode, @ref POWERDlpsMode.
*/
POWERDlpsMode power_dlps_mode_get(void);

/**
 * @brief  Set BT power mode.
 * @param[in]  mode BT power mode, @ref T_BT_POWER_MODE.
*/
void bt_power_mode_set(T_BT_POWER_MODE mode);

/**
 * @brief  Get BT power mode.
 *
 * @return BT power mode, @ref T_BT_POWER_MODE.
*/
T_BT_POWER_MODE bt_power_mode_get(void);

/**
 * @brief  Pause low power mode. API increments a control value by 1. If the control value > 0, low power mode is not allowed to enter.
 *
 * @return           Operation result, and will always return 0.
 * @retval 0         Success.
*/
int32_t power_mode_pause(void);

/**
 * @brief  Resume low power mode. API decrements a control value by 1. If the control value is reduced to 0, low power mode is allowed to enter.
 *
 * @return           Operation result, and will always return 0.
 * @retval 0         Success.
*/
int32_t power_mode_resume(void);

/**
 * @brief  Register check callback function to power manager. In idle task, system will call this function each time before entering DLPS to decide whether DLPS is allowed to enter.
 * @param[in] func  Check callback function, @ref POWERCheckFunc.
 *
 * @return           Operation result, and will always return 0.
 * @retval 0         Success.
*/
int32_t power_check_cb_register(POWERCheckFunc func);


/**
 * @brief  Register callback function to power manager which will call it in different power stages.
 * @param[in]  func  Callback function, @ref POWERStageFunc.
 * @param[in]  stage Power stage for the callback function, @ref POWERStage.
 *
 * @return           Operation result, and will always return 0.
 * @retval 0         Success.
*/
int32_t power_stage_cb_register(POWERStageFunc func, POWERStage stage);

/**
 * @brief  Get callback function which prevents the system from entering low power mode.
 *
 * @return Address of callback function which prevents the system from entering low power mode.
*/
uint32_t *power_get_refuse_reason();

/**
 * @brief  Get wake-up reason when low power mode is exited.
 *
 * @return Wake-up reason, @ref PowerModeWakeupReason.
*/
PowerModeWakeupReason power_get_wakeup_reason();

/**
 * @brief  Get error reason which prevents system from entering low power mode.
 *
 * @return Error reason which prevents the system from entering low power mode, @ref PowerModeErrorCode.
*/
PowerModeErrorCode power_get_error_code(void);

/**
 * @brief  Get debug information of DLPS mode.
 * @param[out]  wakeup_count      Total number of entries into DLPS.
 * @param[out]  last_exit_clk     Timestamp of exiting DLPS most recently, unit: 31.25us.
 * @param[out]  last_enter_clk    Timestamp of entering DLPS most recently, unit: 31.25us.
*/
void power_get_statistics(uint32_t *wakeup_count, uint32_t *last_exit_clk,
                          uint32_t *last_enter_clk);

/**
 * @brief  Stop all of the SW timers which will wake up low power mode.
 * @note   Before entering power down mode, all periodic wake-up timers need to be disabled before.
*
*/
void power_stop_all_non_excluded_timer(void);

/**
 * @brief  Exclude SW timer or task from low power wake list, SW timer needs to be one-shot timer.
 * @param[in]  handle            Handle of SW timer or task.
 * @param[in]  type              Type of SW timer or task, @ref PowerModeExcludedHandleType.
 *
 * @return Operation status.
 * @retval True    Success.
 * @retval False   Fail to exclude SW timer or task. This may happen when handle is NULL or heap malloc fail.
 *
 * @note   When a user wants to start a software timer or set a task delay but does not want to wake up DLPS, they can register this timer or task as an excluded timer or task. For example, if you want to create a debug timer that periodically prints the reasons for not entering DLPS in its callback function, you can register it as an excluded timer.
 *
*/
bool power_register_excluded_handle(void **handle, PowerModeExcludedHandleType type);

/**
 * @brief  Cancel exclude SW timer or task from low power wake list.
 * @param[in]  handle           Handle of timer or task.
 * @param[in]  type             Type of timer or task, @ref PowerModeExcludedHandleType.
 *
 * @return Operation status
 * @retval True    Success
 * @retval False   Fail to cancel exclude SW timer or task. This may happen when handle is NULL or handle is not found in exclude list.
*/
bool power_unregister_excluded_handle(void **handle, PowerModeExcludedHandleType type);

/** End of POWER_MANAGER_Exported_Functions
  * @}
  */

/**
  * \cond     private
  * \defgroup POWER_MANAGER_Private_Functions
  * \{
  */

/**
 * @brief  Shut down SPIC pin before entering DLPS
 * @param  enable Whether to shut down SPIC pin
 * @return None
*/
void shutdown_spic_pin(bool enable);

/**
 * @brief  Check whether clock allows entering low power mode
 *
 * @return Clock check result
*/
POWER_CheckResult power_clock_check_cb(void);

/**
 * @brief  Allow USB wake for low power mode
 *
 * @return None
*/
void usb_set_pon_domain(void);


/**
  *  End of POWER_MANAGER_Private_Functions
  * \}
  * \endcond
  */

/** End of POWER_MANAGER
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif
