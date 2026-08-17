/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef __CFG_ITEM_API_H_
#define __CFG_ITEM_API_H_


/*============================================================================*
 *                               Header Files
 *============================================================================*/
#include <stdint.h>
#include <stdbool.h>

/** @defgroup HAL_CFG_ITEM_API Config Item
    * @brief Config item.
    * @{
    */
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                              Types
*============================================================================*/

/** @defgroup HAL_CFG_ITEM_API_EXPORTED_TYPES  Config Item Exported Types
  * @{
  */

/** @brief The result for search config item */
typedef enum
{
    CFG_SERACH_ENTRY_NOT_EXIST = -1,       //!< The config item does not exist.
    CFG_SERACH_ENTRY_SUCCESS = 0,          //!< Search config item success.
    CFG_SERACH_ENTRY_SIZE_MISMATCH = 1,    //!< The sum of config entry sizes mismatches with the module size.
    CFG_SERACH_MODULE_SIZE_MISMATCH = 2,   //!< The sum of all module sizes mismatches with the config total size.
    CFG_SERACH_MODULE_SIG_MISMATCH = 3,    //!< Mismatch the the Signature_Field in the ConfigHeaderSection_T.
    CFG_SEARCH_ENTRY_SIZE_INVALID = 4,     //!< Mismatch the data lenght in the ConfigHeaderSection_T.
} CFG_SERACH_RESULT_E;

/** @brief The result for update config item */
typedef enum
{
    CFG_UPDATE_SUCCESS = 0,                 //!< Update config flash success.
    CFG_UPDATE_UNLOCK_BP_FAIL = 1,          //!< Unlock BP fail.
    CFG_UPDATE_ERASE_FAIL = 2,              //!< Erase config flash fail.
    CFG_UPDATE_WRITE_FAIL = 3,              //!< Write config flash fail.
    CFG_UPDATE_LOCK_BP_FAIL = 4,            //!< Lock BP fail.
} CFG_UPDATE_RESULT_E;

/** @brief Read or update config item flag*/
typedef enum
{
    CFG_READ   = 1,                         //!< Read config item.
    CFG_UPDATE = 2,                         //!< Update config item.
} CFG_OP_CODE_E;

/** @brief The result for read config item */
typedef enum
{
    CFG_READ_NONE      = 0,                 //!< Don't need to read.
    CFG_READ_SUCCESS   = 1,                 //!< Read config item success.
    CFG_READ_NOT_EXIST = 2,                 //!< Config item does not exist.
    CFG_READ_FAIL      = 3,                 //!< Read config item fail.
} CFG_READ_RESULT_E;

/** @brief The tx power Structure */
typedef struct
{
    /**
      * Update (1: set the txgain_edr_LE1M, 0: don't set txgain_edr_LE1M).
        *
      * Read (1: read the txgain_edr_LE1M, 0: don't read txgain_edr_LE1M).
      * Read response (@ref CFG_READ_RESULT_E).
      */
    uint8_t txgain_br_LE1M_flag;
    union
    {
        uint8_t txgain_edr_LE1M_index; //!< The txgain_edr_LE1M with index value
        int8_t txgain_edr_LE1M_dbm;    //!< The txgain_edr_LE1M with dBm value
    } txgain_edr_LE1M;

    /**
       * Update (1: set the txgain_edr_LE2M, 0: don't set txgain_edr_LE2M).
        *
       * Read (1: read the txgain_edr_LE2M, 0: don't read txgain_edr_LE2M).
       * Read response(@ref CFG_READ_RESULT_E).
       */
    uint8_t txgain_br_LE2M_flag;
    union
    {
        uint8_t txgain_edr_LE2M_index; //!< The txgain_edr_LE2M with index value
        int8_t txgain_edr_LE2M_dbm;    //!< The txgain_edr_LE2M with dBm value
    } txgain_edr_LE2M;

    /**
       * Update (1: set the txgain_ZB, 0: don't set txgain_ZB).
        *
       * Read (1: read the txgain_ZB, 0: don't read txgain_ZB).
       * Read response(@ref CFG_READ_RESULT_E).
       */
    uint8_t txgain_ZB_flag;
    union
    {
        uint8_t txgain_ZB_index;      //!< The txgain_ZB with index value
        int8_t txpower_ZB_dbm;        //!< The txgain_ZB with dBm value
    } txgain_ZB;

} TX_POWER_CFG_t;

/** End of HAL_CFG_ITEM_API_EXPORTED_TYPES
  * @}
  */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @addtogroup HAL_CFG_ITEM_EXT_EXPORTED_FUNCTIONS Config Item Exported Functions
  * @{
  */

/**
 * @brief Get the config payload length.
 * \param[in] p_cfg_payload   Set null to get the config module total length on flash OEM Configuration Data.
 *                            Set a valid RAM address to get the config module total length on the RAM buffer.
 *
 * @return The total config module length.
 */
uint32_t cfg_get_size(void *p_cfg_payload);

/**
 * @brief Read back the config data on the flash to the ram buffer.
 * \param[in] address   Specify the system config read address, default set as OEM Configuration Data.
 * \param[in] backup_len  Specify the backup config data length.
 *
 * @return The config buffer backup pointer on the heap.
 */
void *cfg_backup(uint32_t address, uint32_t backup_len);

/**
 * @brief Write the config data on the RAM to the flash.
 * \param[in] p_new_cfg_buf   Pointer to the occd config payload data backup on RAM.
 * \param[in] backup_len      Specify the config buffer length.
 *
 * @return The result of config buffer writing operation.
 * @retval true              Success.
 * @retval false             Fail.
 */
bool cfg_write_to_flash(void *p_new_cfg_buf, uint32_t backup_len);

/**
 * @brief Update the config entry item in the config data buffer.
 * \param[in] p_new_cfg_buf   Pointer to the occd config payload data backup on RAM.
 * \param[in] module_id       Specify the config item module id.
 * \param[in] p_cfg_entry     Pointer to the config entry item to update.
 *
 * @return The result of updating the config item in p_new_cfg_buf.
 * @retval true              Success.
 * @retval false             Fail.
 */
bool cfg_update_item_in_store(uint8_t *p_new_cfg_buf, uint16_t module_id,
                              void *p_cfg_entry);

/**
 * @brief Update the config entry item on the occd flash.
 * \param[in] module_id       Specify the config item module id.
 * \param[in] p_cfg_entry     Pointer to the config entry item to update.
 *
 * @return The result of updating the config item on the flash of system config.
 * @retval true              Success.
 * @retval false             Fail.
 */
bool cfg_add_item(uint16_t module_id, void *p_cfg_entry);

/**
  * @brief Write MAC address to config, this is mainly used on production line.
  * @param[in] p_mac_addr  The buffer hold MAC address (6 bytes).
  *
  * @return Write MAC to config fail or success.
  * @retval true    Write MAC to config success.
  * @retval false   Write MAC to config fail.
 */
bool cfg_update_mac(uint8_t *p_mac_addr);

/**
  * @brief Write 40M XTAL calibration data to config sc_xi_40m and sc_xo_40m.
  *        This is mainly used on production line.
  * @param[in] xtal           The value of 40M XTAL calibration data.
    *
  * @return Write calibration data to config fail or success.
  * @retval true              Success.
  * @retval false             Fail.
  */
bool cfg_update_xtal(uint8_t xtal);

/**
  * @brief Write Max Tx Power config data into the system config.
  * @param[in] tx_power_cfg    The pointer to @ref TX_POWER_CFG_t structure data.
    *
  * @return The result of writing Max Tx Power config data to system config.
  * @retval true              Success.
  * @retval false             Fail.
  */
bool cfg_update_txpower(TX_POWER_CFG_t *tx_power_cfg);

/**
  * @brief Write Default Tx Power config data into the system config.
  * @param[in] tx_power_cfg    The pointer to @ref TX_POWER_CFG_t structure data.
    *
  * @return The result of writing Default Tx Power config data to system config.
  * @retval true              Success.
  * @retval false             Fail.
  */
bool cfg_update_default_txpower(TX_POWER_CFG_t *tx_power_cfg);

/**
  * @brief Read the config entry item on the occd flash
  * \param[in] module_id       Specify the config item module id.
  * \param[in,out] p_cfg_entry     Pointer to the config entry item to read, input the item offset, len and mask, output data.
    *
  * @return  @ref CFG_SERACH_RESULT_E The result of search the config item.
  */
CFG_SERACH_RESULT_E cfg_read_item(uint16_t module_id, void *p_cfg_entry);
/**
  * @brief Read MAC address.
  * @param[out] p_mac_addr  Specify the buffer to read back the MAC address (6 bytes).
    *
  * @return Read MAC result.
  * @retval true    Success.
  * @retval false   Fail.
 */
bool cfg_read_mac(uint8_t *p_mac_addr);

/**
  * @brief Read 40M XTAL XI(sc_xi_40m) and XO(sc_xo_40m) data.
  * @param[out] p_xtal_xi    Get the value of 40M XTAL XI calibration data.
  * @param[out] p_xtal_xo    Get the value of 40M XTAL XO calibration data.
    *
  * @return The result of reading 40M XTAL calibration data.
  * @retval true              Success.
  * @retval false             Fail.
  */
bool cfg_read_xtal(uint8_t *p_xtal_xi, uint8_t *p_xtal_xo);

/**
  * @brief Read Max Tx Power config data.
  * @param[out] tx_power_cfg   The pointer to @ref TX_POWER_CFG_t structure data.
    *
  * @return The result of reading Max Tx Power config data.
  * @retval true              Success.
  * @retval false             Fail.
  */
bool cfg_read_txpower(TX_POWER_CFG_t *tx_power_cfg);

/**
  * @brief Read Default Tx Power config data.
  * @param[out] tx_power_cfg   The pointer to @ref TX_POWER_CFG_t structure data.
    *
  * @return The result of reading Default Tx Power config data.
  * @retval true              Success.
  * @retval false             Fail.
  */
bool cfg_read_default_txpower(TX_POWER_CFG_t *tx_power_cfg);

/**
  * @brief Read LE Master Link Num and LE Slave Link Num form config data.
  * @param[out] master_num   Get LE Master Link Num.
  * @param[out] slave_num    Get LE Slave Link Num.
    *
  * @return The result of reading LE Master and Slave Link Num.
  * @retval true              Success.
  * @retval false             Fail.
  */
bool cfg_read_le_linknum(uint8_t *master_num, uint8_t *slave_num);

/** End of HAL_CFG_ITEM_EXT_EXPORTED_FUNCTIONS
  * @}
  */
#ifdef __cplusplus
}
#endif
/** @} */ /* End of group HAL_CFG_ITEM_API */
#endif /* __CFG_ITEM_API_H_ */
