/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _FTL_H_
#define _FTL_H_
#include <stdint.h>
#include <stdbool.h>
#ifdef  __cplusplus
extern  "C" {
#endif  // __cplusplus
/** @defgroup  FTL    Flash Transport Layer
  * @brief simple implementation of file system for flash
  * @{
  */
/*============================================================================*
  *                                   Macros
  *============================================================================*/
/** @defgroup FTL_Exported_Macros Flash Transport Layer Exported Macros
    * @brief
    * @{
    */

#define FTL_WRITE_SUCCESS                   (0x00)  /**< FTL write success */
#define FTL_WRITE_ERROR_IN_INTR             (0x01)  /**< FTL write error: interrupted */
#define FTL_WRITE_ERROR_INVALID_ADDR        (0x02)  /**< FTL write error: invalid address */
#define FTL_WRITE_ERROR_OUT_OF_SPACE        (0x03)  /**< FTL write error: out of space */
#define FTL_WRITE_ERROR_READ_BACK           (0x04)  /**< FTL write error: read back failed */
#define FTL_WRITE_ERROR_VERIFY              (0x05)  /**< FTL write error: verification failed */
#define FTL_WRITE_ERROR_INVALID_PARAMETER   (0x06)  /**< FTL write error: invalid parameter */
#define FTL_WRITE_ERROR_ERASE_FAIL          (0x07)  /**< FTL write error: erase failed */
#define FTL_WRITE_ERROR_NOT_INIT            (0x08)  /**< FTL write error: FTL not initialized */
#define FTL_READ_SUCCESS                    (0x00)  /**< FTL read success */
#define FTL_READ_ERROR_INVALID_LOGICAL_ADDR (0x01)  /**< FTL read error: invalid logical address */
#define FTL_READ_ERROR_READ_NOT_FOUND       (0x02)  /**< FTL read error: data not found */
#define FTL_READ_ERROR_PARSE_ERROR          (0x03)  /**< FTL read error: parse error */
#define FTL_READ_ERROR_INVALID_PARAMETER    (0x04)  /**< FTL read error: invalid parameter */
#define FTL_READ_ERROR_NOT_INIT             (0x05)  /**< FTL read error: FTL not initialized */
#define FTL_INIT_ERROR_ERASE_FAIL           (0x01)  /**< FTL init error: erase failed */

/** End of FTL_Exported_Macros
  * @}
  */
/*============================================================================*
  *                                   Types
  *============================================================================*/
/** @defgroup FTL_Exported_Types Flash Transport Layer Exported Types
  * @brief
  * @{
  */
enum
{
    FTL_IOCTL_DEBUG = 0,  /**< IO code for FTL debug. */
    FTL_IOCTL_CLEAR_ALL = 2,  /**< IO code for clear FTL section. */
    FTL_IOCTL_ERASE_INVALID_PAGE = 3,  /**< IO code to erase invalid page. */
    FTL_IOCTL_ENABLE_GC_IN_IDLE = 4,  /**< IO code to enable garbage collection in idle task. */
    FTL_IOCTL_DISABLE_GC_IN_IDLE = 5,  /**< IO code to disable garbage collection in idle task.*/
    FTL_IOCTL_DO_GC_IN_APP = 6,  /**< IO code to do garbage collection in app. */
    FTL_IOCTL_GET_PAGE_SIZE, /**< Not supported. */
    FTL_IOCTL_SAVE_TO_STORAGE = 0x1000, /**< IO code to test FTL save function. */
    FTL_IOCTL_LOAD_FROM_STORAGE, /**< IO code to test FTL load function. */
};
/** End of FTL_Exported_Types
  * @}
  */
/*============================================================================*
  *                                Functions
  *============================================================================*/
/** @defgroup FTL_Exported_Functions Flash Transport Layer Exported Functions
  * @brief
  * @{
  */
/**
    * @brief    Save specified value to specified FTL offset.
    * @param[in]    pdata  Specify data buffer.
    * @param[in]    offset Specify FTL offset to store.
    * @param[in]    size   Size to store.
    * @return   Status. @ref FTL_Exported_Macros
    * @retval   0 Successful.
    * @retval   otherwise Fail.
    * @note     FTL offset is pre-defined and no confict with ROM.
    */
extern uint32_t(*ftl_save_to_storage)(void *pdata_tmp, uint16_t offset, uint16_t size);
/**
    * @brief    Load specified FTL offset parameter to specified buffer.
    * @param[out]   pdata  Specify data buffer.
    * @param[in]    offset Specify FTL offset to load.
    * @param[in]    size   Size to load.
    * @return   Status. @ref FTL_Exported_Macros
    * @retval   0  Successful.
    * @retval   otherwise Fail.
    * @note     FTL offset is pre-defined and no confict with ROM.
    */
extern uint32_t(*ftl_load_from_storage)(void *pdata_tmp, uint16_t offset, uint16_t size);
/**
    * @brief    Save specified value to specified FTL offset(APP region).
    * @note     FTL offset is pre-defined and no conflict. @ref FTL_Page
    * @param[in]    pdata  Specify data buffer.
    * @param[in]    offset Specify FTL offset to store.
    * @param[in]    size   Size to store.
    * @return   Status. @ref FTL_Exported_Macros
    * @retval   0  Successful.
    * @retval   otherwise Fail.
    */
uint32_t ftl_save(void *pdata, uint16_t offset, uint16_t size);
/**
    * @brief    Load specified FTL offset(APP region) parameter to specified buffer.
    * @note     FTL offset is pre-defined and no conflict. @ref FTL_Page
    * @param[out]     pdata  Specify data buffer.
    * @param[in]      offset Specify FTL offset to load.
    * @param[in]      size   Size to load.
    * @return   Status. @ref FTL_Exported_Macros
    * @retval   0 Successful.
    * @retval   otherwise Fail.
    */
uint32_t ftl_load(void *pdata, uint16_t offset, uint16_t size);
/**
    * @brief    Control function entry for FTL.
    * @param[in]    cmd    Command code for different operation.
    * @param[in]    p1     Command parameter. @ref FTL_IO_CTL_CODE
    * @param[in]    p2     Extended command parameters.
                FTL_IOCTL_DEBUG/FTL_IOCTL_CLEAR_ALL: p1: 0, p2: 0
                FTL_IOCTL_ERASE_INVALID_PAGE/FTL_IOCTL_DISABLE_GC_IN_IDLE: p1: 0, p2: 0
                FTL_IOCTL_DO_GC_IN_APP/FTL_IOCTL_ENABLE_GC_IN_IDLE:
                    p1: free page count, from 1 to 4; p2: free cell index from 0 to 0x3ff
    * @return    Status. @ref FTL_Exported_Macros
    * @retval    0 Successful.
    * @retval    otherwise Fail.
    */
extern uint32_t (*ftl_ioctl)(uint32_t cmd, uint32_t p1, uint32_t p2);
/** End of FTL_Exported_Functions
  * @}
  */
/** End of FTL
  * @}
  */
#ifdef  __cplusplus
}
#endif // __cplusplus
#endif // _FTL_H_
