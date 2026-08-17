/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __FMC_API_H_
#define __FMC_API_H_
#include <stdbool.h>
#include <stdint.h>

/** @defgroup FMC  Flexible Memory Controller
  * @brief
  * @{
  */


/** @defgroup FMC_API_Exported_Types  FMC API Exported Types
  * @brief
  * @{
  */


/**
 * @brief FMC index
 *
 */
typedef enum
{
    FMC_FLASH_NOR_IDX0, /**< FMC index0*/
    FMC_FLASH_NOR_IDX1, /**< FMC index1 */
    FMC_FLASH_NOR_IDX2, /**< FMC index2 */
    FMC_FLASH_NOR_IDX3 /**< FMC index3 */
} FMC_FLASH_NOR_IDX_TYPE;


/**
 * @brief Nor flash erase mode
 *
 */
typedef enum
{
    FMC_FLASH_NOR_ERASE_CHIP   = 1, /**< Chip erase*/
    FMC_FLASH_NOR_ERASE_SECTOR = 2, /**< Sector erase*/
    FMC_FLASH_NOR_ERASE_BLOCK  = 4, /**< Block erase*/
} FMC_FLASH_NOR_ERASE_MODE;

/**
 * @brief Size of PSRAM refresh area in half sleep mode
 *
 */
typedef enum
{
    FMC_PSRAM_WB_REFRESH_FULL       = 0, /**< Refresh full area*/
    FMC_PSRAM_WB_REFRESH_BOTTOM_1_2 = 1, /**< Refresh bottom 1/2 area*/
    FMC_PSRAM_WB_REFRESH_BOTTOM_1_4 = 2, /**< Refresh bottom 1/4 area*/
    FMC_PSRAM_WB_REFRESH_BOTTOM_1_8 = 3, /**< Refresh bottom 1/8 area*/
    FMC_PSRAM_WB_REFRESH_NONE       = 4, /**< Refresh none area*/
    FMC_PSRAM_WB_REFRESH_TOP_1_2    = 5, /**< Refresh top 1/2 area*/
    FMC_PSRAM_WB_REFRESH_TOP_1_4    = 6, /**< Refresh top 1/4 area*/
    FMC_PSRAM_WB_REFRESH_TOP_1_8    = 7, /**< Refresh top 1/8 area*/
} FMC_PSRAM_WB_PARTIAL_ARRAY_REFRESH;

/**
 * @brief PSRAM low power mode
 *
 */
typedef enum
{
    FMC_PSRAM_LPM_STANDBY_MODE, /**< Standby mode*/
    FMC_PSRAM_LPM_HALF_SLEEP_MODE, /**< Half sleep mode*/
    FMC_PSRAM_LPM_DEEP_POWER_DOWN_MODE, /**< Deep power down mode*/
} FMC_PSRAM_LPM_TYPE;


/**
  * \cond     private
  * \defgroup FMC_API_Private_Type
  * \{
  */


/** @defgroup FMC_API_Flash_Callback_Definition  FMC API Flash Callback Definition
  * @ingroup FMC_API_Exported_Types
  * @brief
  * @{
  */
typedef void (*FMC_FLASH_NOR_ASYNC_CB)(void);

typedef enum
{
    PARTITION_FLASH_OCCD,
    PARTITION_FLASH_BOOT_PATCH0,
    PARTITION_FLASH_BOOT_PATCH1,
    PARTITION_FLASH_OTA_BANK_0,
    PARTITION_FLASH_OTA_BANK_1,
    PARTITION_FLASH_SECURE_APP0,
    PARTITION_FLASH_SECURE_APP1,
    PARTITION_FLASH_SECURE_APP_DATA0,
    PARTITION_FLASH_SECURE_APP_DATA1,
    PARTITION_FLASH_RO_DATA1,
    PARTITION_FLASH_RO_DATA2,
    PARTITION_FLASH_RO_DATA3,
    PARTITION_FLASH_RO_DATA4,
    PARTITION_FLASH_RO_DATA5,
    PARTITION_FLASH_RO_DATA6,
    PARTITION_FLASH_BKP_DATA1,
    PARTITION_FLASH_BKP_DATA2,
    PARTITION_FLASH_OTA_TMP,
    PARTITION_FLASH_FTL,

} T_FLASH_PARTITION_NAME;

typedef enum _FLASH_IMG_ID
{
    FLASH_IMG_OTA                  = 0, /* OTA header */
    FLASH_IMG_MCUPATCH             = 1,
    FLASH_IMG_MCUAPP               = 2,
    FLASH_IMG_MCUCONFIG            = 3,
    FLASH_IMG_MCUAPPDATA1          = 4,
    FLASH_IMG_MCUAPPDATA2          = 5,
    FLASH_IMG_MCUAPPDATA3          = 6,
    FLASH_IMG_MCUAPPDATA4          = 7,
    FLASH_IMG_MCUAPPDATA5          = 8,
    FLASH_IMG_MCUAPPDATA6          = 9,
    FLASH_IMG_UPPERSTACK           = 10,
    FLASH_IMG_MAX,
} FLASH_IMG_ID;

typedef enum
{
    PSRAM_WB_PARTIAL_ARRAY_SET,
    PSRAM_WB_ENTER_LMP_MODE,
    PSRAM_WB_EXIT_LMP_MODE,
} PSRAM_LMP_CTRL_FUNCTION_CALL;


/** End of FMC_API_Flash_Callback_Definition
 * @}
 */

/**
*  End of FMC_Private_type
* \}
* \endcond
*/


/** End of FMC_API_Exported_Types
 * @}
 */


/** @defgroup FMC_API_Exported_Functions  FMC API Exported Functions
  * @brief
  * @{
  */

/**
 * @brief               Task-safe nor flash read
 * @param[in] addr      The ram address mapping of nor flash going to be read
 * @param[in] data      Data buffer to be read into
 * @param[in] len       Read data length
 * @return          Read result
 * @retval          True Success.
 * @retval          False Failed.
 */
bool fmc_flash_nor_read(uint32_t addr, void *data, uint32_t len);

/**
 * @brief               Task-safe nor flash write
 * @param[in] addr      The ram address mapping of nor flash going to be written
 * @param[in] data      Data buffer to be written into
 * @param[in] len       Write data length
 * @return          Write result
 * @retval          True Success.
 * @retval          False Failed.
 */
bool fmc_flash_nor_write(uint32_t addr, void *data, uint32_t len);

/**
 * @brief           Task-safe nor flash erase
 * @param[in] addr      The ram address mapping of nor flash going to be erased
 * @param[in] mode      Erase mode defined as @ref FMC_FLASH_NOR_ERASE_MODE
 * @return          Erase result
 * @retval          True Success.
 * @retval          False Failed.
 */
bool fmc_flash_nor_erase(uint32_t addr, FMC_FLASH_NOR_ERASE_MODE mode);

/**
 * @brief           Set block protect level with lock
 * @param addr      Nor flash addr
 * @param bp_lv     Nor flash BP level to be set
 * @return          Set BP result
 * @retval          True Success.
 * @retval          False Failed.
 */
bool fmc_flash_nor_set_bp_lv(uint32_t addr, uint8_t bp_lv);

/**
 * @brief           Get block protect level with lock
 * @param addr      Nor flash addr
 * @param bp_lv     Nor flash BP level to be retrieved.
 * @return          Get BP result
 * @retval          True Success.
 * @retval          False Failed.
 */
bool fmc_flash_nor_get_bp_lv(uint32_t addr, uint8_t *bp_lv);


//bool fmc_flash_nor_clock_switch(FMC_FLASH_NOR_IDX_TYPE idx, uint32_t required_mhz,
//                                uint32_t *actual_mhz);

/**
 * @brief           Get psram power status
 * @return          Power status
 * @retval          True power on.
 * @retval          False power off.
 */
bool fmc_get_psram_power_status(void);

/**
 * @brief           Ctrl fmc pad in lps mode
 * @param[in] id        FMC idx number
 * @param[in] enable    True if enter lps mode, otherwise false
 */
void fmc_pad_ctrl_in_lps_mode(FMC_FLASH_NOR_IDX_TYPE idx, bool enable);

/**
 * @brief           Set psram retention partition
 * @param[in] idx       Specific psram
 * @param[in] partial   PSRAM  retention partition
 * @return          Set result
 * @retval          True Success.
 * @retval          False Failed.
 */
bool fmc_psram_wb_set_partial_refresh(FMC_FLASH_NOR_IDX_TYPE idx,
                                      FMC_PSRAM_WB_PARTIAL_ARRAY_REFRESH partial);

/**
 * @brief           PSRAM enter low power mode
 * @param[in] idx       Specific psram
 * @param[in] lpm_mode  PSRAM low power mode
 * @return          Result of enter power mode
 * @retval          True Success.
 * @retval          False Failed.
 */
bool fmc_psram_enter_lpm(FMC_FLASH_NOR_IDX_TYPE idx, FMC_PSRAM_LPM_TYPE lpm_mode);

/**
 * @brief           PSRAM exit low power mode
 * @param[in] idx       Specific psram
 * @param[in] lpm_mode  PSRAM low power mode
 * @return          Result of exit power mode
 * @retval          True Success.
 * @retval          False Failed.
 */
bool fmc_psram_exit_lpm(FMC_FLASH_NOR_IDX_TYPE idx, FMC_PSRAM_LPM_TYPE lpm_mode);

/**
 * @brief           Flash set 4 byte address mode
 * @param[in] idx       Specific flash
 * @param[in] enable    True if enter 4 byte address mode; otherwise, enter 3-byte address mode
 * @return          Result of enter 4 byte address mode
 * @retval          True Success.
 * @retval          False Failed.
 */
bool fmc_flash_set_4_byte_address_mode(FMC_FLASH_NOR_IDX_TYPE idx, bool enable);

//bool fmc_psram_clock_switch(CLK_FREQ_TYPE clk);

/**
 * @brief           Flash set default bp level
 * @param[in] enable    True if set default bp level; otherwise, set bp_lv to 0
 */
void fmc_flash_set_default_bp_lv(bool enable);

/** End of FMC_API_Exported_Functions
  * @}
  */


/**
 * \cond     private
 * \defgroup FMC_Private_Constants
 * \{
 */

#define FMC_SEC_SECTION_LEN     0x1000

/**
*  End of FMC_Private_Constants
* \}
* \endcond
*/

/**
 * \cond     private
 * \defgroup FMC_Private_Functions
 * \{
 */

/**
 * @brief           task-safe nor flash auto dma read with sequential transaction enabled
 * @param src       the ram address mapping of nor flash going to be read from
 * @param dst       the ram address going to be written to
 * @param len       dma data length
 * @param cb        call back function which is to be executed when dma finishing
 * @return          true if read successful, otherwise false
 */
bool fmc_flash_nor_auto_dma_read(uint32_t src, uint32_t dst, uint32_t len,
                                 FMC_FLASH_NOR_ASYNC_CB cb);

/**
 * @brief           get flash partition addr
 * @param name      flash partition name
 * @return          true if get partition successful, otherwise false
 */
uint32_t flash_partition_addr_get(T_FLASH_PARTITION_NAME name);

/**
 * @brief           get flash partition size
 * @param name      flash partition name
 * @return          true if get partition size successful, otherwise false
 */
uint32_t flash_partition_size_get(T_FLASH_PARTITION_NAME name);

/**
 * @brief           get image payload flash addr
 * @param id        flash image id number
 * @return          true if get image payload flash addr success, otherwise false
 */
uint32_t flash_cur_bank_img_payload_addr_get(FLASH_IMG_ID id);

/**
 * @brief           get image header flash addr
 * @param id        flash image id number
 * @return          true if get image header flash addr success, otherwise false
 */
uint32_t flash_cur_bank_img_header_addr_get(FLASH_IMG_ID id);

/**
*  End of FMC_Private_Functions
* \}
* \endcond
*/

/** End of FMC
 * @}
 */

#endif

