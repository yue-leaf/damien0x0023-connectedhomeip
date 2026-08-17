/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                      Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _DFU_COMMON_H_
#define _DFU_COMMON_H_

#include "patch_header_check.h"
#include "rtl876x.h"
#include "wdt.h"

/** @defgroup  DFU_COMMON DFU Common
  * @brief
  * @{
  */
/*============================================================================*
 *                              Macros
 *============================================================================*/
/** @defgroup DFU_COMMON_Exported_Macros DFU Common Exported Macros
  * @brief
  * @{
  */
/** @brief  Flash sector size. */
#define FLASH_SECTOR_SIZE                          0x1000

/** @brief  Compile-time assertion for static checking. */
#define PLATFORM_STATIC_ASSERT(condition, identifier) typedef char PALStaticAssert_##identifier[(condition) ? 1 : -1]

/** @brief  Set the valid image bitmap for a given image ID. */
#define SET_VALID_BITMAP(image_id)      (valid_bitmap |= BIT(image_id - IMG_DFU_FIRST))

/** @brief  Get the valid image bitmap status for a given image ID. */
#define GET_VALID_BITMAP(image_id)      (valid_bitmap >> (image_id - IMG_DFU_FIRST) & BIT0)

/** @brief  Set the valid user data image bitmap for a given image ID. */
#define SET_USER_DATA_VALID_BITMAP(image_id)      (user_data_valid_bitmap |= BIT(image_id - IMG_USER_DATA_FIRST))

/** @brief  Get the valid user data image bitmap for a given image ID. */
#define GET_USER_DATA_VALID_BITMAP(image_id)      (user_data_valid_bitmap >> (image_id - IMG_USER_DATA_FIRST) & BIT0)


/** End of DFU_COMMON_Exported_Macros
  * @}
  */


/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @defgroup DFU_COMMON_Exported_Variables DFU Common Exported Variables
  * @brief
  * @{
  */
/** @brief  Bitmap representing the validity status of images. */
extern uint32_t valid_bitmap;

/** @brief  Bitmap representing the validity status of user data images. */
extern uint32_t user_data_valid_bitmap;


/** End of DFU_COMMON_Exported_Variables
  * @}
  */

/*============================================================================*
 *                              Types
 *============================================================================*/
/** @defgroup DFU_COMMON_Exported_Types DFU Common Exported Types
  * @brief
  * @{
  */
/** @brief   Active OTA Bank numbers. */
typedef enum
{
    OTA_BANK0,
    OTA_BANK1,
    OTA_BANK_MAX,
} T_ACTIVE_BANK_NUM;

/** @brief  User data error types. */
typedef enum
{
    USER_DATA_SUCCESS = 0,
    USER_DATA_NOT_SUPPORT_OTA,
    USER_DATA_TYPE_ERROR,
} T_USER_DATA_ERROR_TYPE;

/** End of DFU_COMMON_Exported_Types
  * @}
  */


/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup DFU_COMMON_Exported_Functions DFU Common Exported Functions
  * @brief
  * @{
  */
/**
 * @brief  Decrypt 16 bytes of data using AES-256.
 * @param[in]  input   Pointer to the input data to be decrypted.
 */
void dfu_aes256_decrypt_16byte(uint8_t *input);

/**
 * @brief  Get the size of the bank area for a given image.
 * @param[in]  image_id   The identifier of the image.
 * @return Size of the bank area associated with the image.
 */
uint32_t get_bank_size_by_img_id(IMG_ID image_id);

/**
 * @brief  Reset the chip for DFU operations.
 * @param[in]   reset_mode   The mode used for reset.
 * @param[in]   reason       The reason for the reset.
 */
void dfu_fw_reboot(WDTMode_TypeDef reset_mode, T_SW_RESET_REASON reason);

/**
 * @brief  Check the OTA mode flag to determine if an image update is needed.
 * @return True if an image update is needed, false otherwise.
 */
bool dfu_check_ota_mode_flag(void);

/**
 * @brief  Set the OTA mode flag.
 * @param[in]   enable   True to enable, false to disable the OTA mode flag.
 */
void dfu_set_ota_mode_flag(bool enable);

/**
 * @brief  Switch to OTA mode if supported, necessary for normal OTA applications.
 */
void dfu_switch_to_ota_mode(void);

/**
 * @brief    Get the IC type of the current firmware.
 * @return   The IC type.
 */
uint8_t dfu_get_ic_type(void);

/**
 * @brief    Get the active bank number.
 * @return   The active bank number.
 */
T_ACTIVE_BANK_NUM get_active_bank_num(void);

/**
 * @brief    Get the image address of the inactive bank.
 * @param[in]     image_id   The identifier of the image.
 * @return   The address of the image in the inactive bank.
 */
uint32_t dfu_get_temp_ota_bank_img_addr_by_img_id(IMG_ID image_id);

/**
 * @brief    Get the size of the image in the inactive bank.
 * @param[in]     image_id   The identifier of the image.
 * @return   The size of the image in the inactive bank.
 */
uint32_t dfu_get_temp_ota_bank_img_size_by_img_id(IMG_ID image_id);

/**
 * @brief Calculate the checksum of data within a buffer in flash.
 *
 * @param[in] buf        Pointer to the buffer containing the data.
 * @param[in] length     Length of the data in the buffer.
 * @param[out] mCrcVal    Variable to store the calculated CRC value.
 * @return 0 if checksum calculation is successful, else error line number.
 */
uint32_t dfu_check_bufcrc(uint8_t *buf, uint32_t length, uint16_t mCrcVal);

/**
 * @brief Erase a sector of the flash.
 * @param[in]  image_id    The identifier of the image.
 * @param[in]  offset      Flash address offset.
 * @return  0 if erase is successful, else error line number.
 */
uint32_t dfu_flash_erase(IMG_ID image_id, uint32_t offset);

/**
 * @brief    Write data to flash memory.
 * @param[in]    image_id  The identifier of the image.
 * @param[in]    offset    Offset of the current image.
 * @param[in]    total_offset  Total offset of the image in OTA TEMP area.
 * @param[in]    length    Length of the data to be written.
 * @param[in]    p_void    Pointer to the data to be written.
 * @return   0 on success, non-zero on failure.
 */
uint32_t dfu_write_data_to_flash(uint16_t image_id, uint32_t offset, uint32_t total_offset,
                                 uint32_t length, void *p_void);

/**
 * @brief    Validate the integrity of a specified image.
 * @param[in]    image_id   The identifier of the image.
 * @param[in]    offset     Address offset to start the check.
 * @return   True if image integrity is confirmed, false otherwise.
 */
bool dfu_checksum(IMG_ID image_id, uint32_t offset);

/**
 * @brief    Set the not_ready flag of a specific image.
 * @param[in]    p_header   Pointer to the image header structure.
 */
void dfu_set_ready(T_IMG_HEADER_FORMAT *p_header);

/**
 * @brief    Set the not_obsolete flag of a specific image.
 * @param[in]    p_header   Pointer to the image header structure.
 */
void dfu_set_obsolete(T_IMG_HEADER_FORMAT *p_header);

/**
 * @brief    Retrieve user data information for a specific image.
 * @param[in]     image_id   The identifier of the image.
 * @param[out]    img_info   Pointer to store the image information.
 * @param[in]     is_addr    Boolean indicating if the address is required.
 * @return   A value of type \ref T_USER_DATA_ERROR_TYPE indicating the result.
 */
T_USER_DATA_ERROR_TYPE dfu_get_user_data_info(IMG_ID image_id,
                                              uint32_t *img_info, bool is_addr);

/** End of DFU_COMMON_Exported_Functions
  * @}
  */

/** End of DFU_COMMON
  * @}
  */

#endif
