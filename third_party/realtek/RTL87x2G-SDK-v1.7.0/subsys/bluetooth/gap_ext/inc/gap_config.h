/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef GAP_CONFIG_H
#define GAP_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "gap_le_types.h"
#include "app_section.h"

/** @addtogroup BT_Host Bluetooth Host
  * @{
  */

/** @addtogroup GAP_LE GAP LE Module
  * @{
  */

/** @addtogroup GAP_LE_CONFIG GAP LE Config Module
  * @brief GAP LE Config Module
  * @{
  */


/*============================================================================*
 *                         Types
 *============================================================================*/
/** @defgroup GAP_LE_CONFIG_Exported_Types GAP LE Config Exported Types
  * @{
  */

/** @brief GAP GATT CCCD not check. */
typedef enum
{
    CONFIG_GATT_CCCD_CHECK,         /**< Check CCCD when server sends notification or indication. */
    CONFIG_GATT_CCCD_NOT_CHECK,     /**< Not check CCCD when server sends notification or indication. */
} T_GAP_CONFIG_GATT_CCCD_NOT_CHECK;

/** End of GAP_LE_CONFIG_Exported_Types
  * @}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** @defgroup GAP_LE_CONFIG_Exported_Functions GAP LE Config Exported Functions
  * @brief
  * @{
  */

/**
 * @brief   Configure whether to check CCCD when server sends notification or indication.
 *
 * @param[in]      cccd_not_check_flag   Whether to check CCCD.
 *                                       Value is @ref T_GAP_CONFIG_GATT_CCCD_NOT_CHECK.
 *
 * <b>Example usage</b>
 * \code{.c}
     void bt_stack_config_init(void)
    {
        gap_config_cccd_not_check(CONFIG_GATT_CCCD_NOT_CHECK);
    }
 * \endcode
 */
void gap_config_cccd_not_check(T_GAP_CONFIG_GATT_CCCD_NOT_CHECK
                               cccd_not_check_flag);

/**
 * @brief   Configure maximum server CCCD number and maximum storage CCCD number.
 *
 * @param[in]      gatt_server_ccc_bits_count    Maximum server CCCD number, default value is 16.
 * @param[in]      gatt_storage_ccc_bits_count   Maximum storage CCCD number, default value is 16.
 *
 * <b>Example usage</b>
 * \code{.c}
     void bt_stack_config_init(void)
    {
        gap_config_ccc_bits_count(18, 18);
    }
 * \endcode
 */
void gap_config_ccc_bits_count(uint8_t gatt_server_ccc_bits_count,
                               uint8_t gatt_storage_ccc_bits_count);

/**
 * @brief   Configure maximum size of Maximum Transmission Unit.
 *
 * @note Default value is 247.
 *
 * @param[in]      att_max_mtu_size    Maximum size of Maximum Transmission Unit.
 *
 * <b>Example usage</b>
 * \code{.c}
     void bt_stack_config_init(void)
    {
        gap_config_max_mtu_size(200);
    }
 * \endcode
 */
void gap_config_max_mtu_size(uint16_t att_max_mtu_size);

/**
 * @brief   Configure key storage flag to determine keys that need to be saved.
 *
 * @param[in]      le_key_storage_flag   Key storage flag to determine keys that need to be saved.
 *                                       Use the combination of macros defined in @ref GAP_LE_STORAGE_BITS.
 *                                       Default value is 0x80FF.
 *                                       Storage bit value:
 *                                       - 0: Not save.
 *                                       - 1: Save.
 *
 * <b>Example usage</b>
 * \code{.c}
     void bt_stack_config_init(void)
    {
        gap_config_le_key_storage_flag(LE_KEY_STORE_CCCD_DATA_BIT | LE_KEY_STORE_REMOTE_LTK_BIT | LE_KEY_STORE_REMOTE_BD_BIT);
    }
 * \endcode
 */
void gap_config_le_key_storage_flag(uint16_t le_key_storage_flag);

/**
 * @brief   Configure LE maximum bonded device number.
 *
 * @note Default value is 1.
 *
 * @param[in]      max_le_paired_device   LE maximum bonded device number.
 *
 * <b>Example usage</b>
 * \code{.c}
     void bt_stack_config_init(void)
    {
        gap_config_max_le_paired_device(2);
    }
 * \endcode
 */
void gap_config_max_le_paired_device(uint8_t max_le_paired_device);

/** End of GAP_LE_CONFIG_Exported_Functions
  * @}
  */

/** End of GAP_LE_CONFIG
  * @}
  */

/** End of GAP_LE
  * @}
  */

/** End of BT_Host
  * @}
  */


#endif
#ifdef __cplusplus
}

#endif /* GAP_CONFIG_H */

