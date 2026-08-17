/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _RTL876X_LIB_PLATFORM_H_
#define _RTL876X_LIB_PLATFORM_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "mem_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup  RTL876X_LIB_PLATFORM   RTL876x LIB Platform
  * @brief
  * @{
  */
/*============================================================================*
 *                         Types
 *============================================================================*/
/** @defgroup RTL876X_LIB_PLATFORM_Exported_Types RTL876x LIB Platform Exported Types
  * @brief
  * @{
  */
typedef void (*WDG_APP_CB)(void);

/** End of RTL876X_LIB_PLATFORM_Exported_Types
  * @}
  */

/*============================================================================*
 *                       Functions
 *============================================================================*/
/** @defgroup RTL876X_LIB_PLATFORM_Exported_Functions RTL876x LIB Platform Exported Functions
  * @brief
  * @{
  */
/**
    * @brief Initialize SDK library function pointer.
    */
void patch_func_ptr_init(void);

/**
    * @brief    Get USB eFuse information.
    * @param[out]  data   Pointer to where the retrieved information will be stored.
    */
void usb_info_get(uint8_t *data);

/**
    * @brief    Determine if OTA supports dual bank merging.
    * @return   Returns true if dual bank merge is supported; otherwise, false.
    */
bool is_ota_support_dual_bank_merge(void);

/**
    * @brief    Print sdk lib version.
    */
void print_sdk_lib_version(void);

/**
    * @brief    Register watchdog app callback.
    * @param[in]    func The callback function to register.
    */
void wdg_register_app_cb(WDG_APP_CB func);

/**
 * @brief  Get ic type.
 * @return A uint8_t value representing the IC type.
 * @code
 * #define RTL8762GTP              0x03
 * #define RTL8762GKH              0x07
 * #define RTL8762GKU              0x08
 * #define RTL8762GTH              0x09
 * #define RTL8762GRH              0x0A
 * #define RTL8762GRU              0x0B
 * #define RTL8762GTU              0x0C
 * #define RTL8777GKF-VA4          0x10
 * #define RTL8772GWP              0x11
 * #define RTL8772GWP-VI           0x11
 * #define RTL8772GWP-VM           0x12
 * #define RTL8772GWP-VM           0x13
 * #define RTL8762GC               0x14
 * #define RTL8762GWF              0x15
 * #define RTL8771GUV              0x16
 * #define RTL8777GKF-VA5          0x17
* @endcode
 */
uint8_t get_ic_type(void);

/**
 * @brief  Get 14 bytes EUID.
 * @return A pointer to a copy of the EUID.
  */
uint8_t *get_ic_euid(void);

/**
   * @brief    Init I-cache/D-cache hit rate and miss rate.
   */
void init_pmu_counter(void);

/**
   * @brief    Print I-cache/D-cache hit rate and miss rate.
   */
void pmu_print_counter(void);

/**
 * @brief  Disable swd pin.
*/
void swd_pin_disable(void);

/**
 * @brief  Bypass pmu check for wfi.
 * @param[in]  enable   A boolean value indicating whether to bypass the PMU check.
*/
void bypass_pmu_check_for_wfi(bool enable);

/**
 * @brief  Enable xtal power in dlps.
 * @param[in]  enable   A boolean value indicating whether to keep xtal power on.
*/
void set_xtal_power_in_dlps(bool enable);

/**
 * @brief  Disable RXI300 interrupt for debug hardfault.
*/
void disable_RXI300_interrupt(void);

/**
 * @brief  Enable or disable XIP (eXecute In Place) access.
 * @param  set    A boolean value indicating whether to disable XIP access.
 */
void disable_xip_access(bool set);
/** End of RTL876X_LIB_PLATFORM_Exported_Functions
  * @}
  */

/** End of RTL876X_LIB_PLATFORM
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* _RTL876X_LIB_PLATFORM_H_ */
