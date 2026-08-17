/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _FTL_H_
#define _FTL_H_
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#ifdef  __cplusplus
extern  "C" {
#endif  // __cplusplus
/** @defgroup  FTL    Flash Transport Layer
    * @brief
    * @{
    */


/*============================================================================*
  *                                   Macros
  *============================================================================*/
/** @defgroup FTL_Exported_Macros Flash Transport Layer Exported Macros
    * @brief
    * @{
    */

#define FTL_POOL                1     /**< Enable FTL pool.  */

#define FTL_PARTITION_NAME      "FTL" /**< Storage partition name of FTL. */
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
typedef enum
{
    FTL_IOCTL_DEBUG                 = 0,  /**< IO code for FTL debug. */
    FTL_IOCTL_CLEAR_ALL             = 2,  /**< IO code for clear FTL section. */
    FTL_IOCTL_DO_GC_IN_APP          = 6,  /**< IO code to do garbage collection in app. */
} T_FTL_IOCTL_CODE;
/** End of FTL_Exported_Types
  * @}
  */

/**
 * \defgroup    FTL_USAGE Flash Transport Layer Usage
 *
 * \brief   Flash Transport Layer Sample
 * \details
 *  - Ext FTL module access. \n
 *   - step1: Init an ext FTL module.
 *   - step2: Save data to the ext FTL module.
 *   - step3: Load data from the ext FTL module.
 *   - Sample code:
 *   @code
        #define EXT_FTL_NAME "TEST_FTL"
        #define EXT_FTL_LOGIC_SIZE     (0x1000)
        #define EXT_FTL_BLOCK_SIZE     (64)
        void ftl_ext_module_demo(void)
        {
            // Init an ext FTL module
            ftl_init_module(EXT_FTL_NAME, EXT_FTL_LOGIC_SIZE, EXT_FTL_BLOCK_SIZE);
            // Save data
            uint8_t data_buf[EXT_FTL_BLOCK_SIZE];
            memset(data_buf, 0x5A, EXT_FTL_BLOCK_SIZE);
            uint16_t test_offset = 0x800;
            uint32_t ret = ftl_save_to_module(EXT_FTL_NAME, data_buf, test_offset, EXT_FTL_BLOCK_SIZE);
            if (ret != ESUCCESS)
            {
                //save data error
                return;
            }
            //Load data
            uint8_t read_buf[EXT_FTL_BLOCK_SIZE];
            ret = ftl_load_from_module(EXT_FTL_NAME, read_buf, test_offset, EXT_FTL_BLOCK_SIZE);
            if (ret != ESUCCESS)
            {
                //load data error
                return;
            }
        }
 *
 *   @endcode
 */


/*============================================================================*
  *                                Functions
  *============================================================================*/
/** @defgroup FTL_Exported_Functions Flash Transport Layer Exported Functions
    * @brief
    * @{
    */

/**
     * @brief    Control function entry for FTL
     *
     * @param[in]    cmd Command code for different operations (see @ref T_FTL_IOCTL_CODE).
     * @param[in]    p1  Command parameter.
     * @param[in]    p2  Extended command parameters.
     *                - FTL_IOCTL_DEBUG/FTL_IOCTL_CLEAR_ALL: p1: 0, p2: 0
     *                - FTL_IOCTL_DO_GC_IN_APP: p1: Free page count; p2: Free cell address, from 0 to 0x1000
     *
     * @return   Results of control.
     * @retval   ESUCCESS  Successful.
     * @retval   Otherwise Fail.
     */
extern uint32_t (*ftl_ioctl)(uint32_t cmd, uint32_t p1, uint32_t p2);


/**
    * @brief    Init an ext FTL module.
    * @param[in]    module_name  Specify FTL ext name, the first 4 bytes can't be the same.
    * @param[in]    malloc_size  FTL module logical size.
    * @param[in]    block_len    Minimum access unit. Must be an integral multiple of 4bytes and cannot exceed 128bytes.
    * @return   Status.
    * @retval   ESUCCESS Successful.
    * @retval   EINVAL   The block_len not satisfy 4-byte alignment or exceeds MAX_BLOCK_SIZE (128).
    * @retval   ENOSPC   No remaining space in FTL.
    * @retval   ENOBUFS  Insufficient RAM space.
    * @retval   EBADF    The maximum number of created modules has been reached.
    * @note     Up to 6 modules can be applied.
    */
int32_t ftl_init_module(char *module_name, uint16_t malloc_size, uint8_t block_len);
/**
    * @brief    Save specified value to FTL module.
    * @param[in]    module_name  Specify FTL module name.
    * @param[in]    pdata Specify data buffer.
    * @param[in]    offset Specify FTL offset to store, need to be an integer multiple of block_len in ftl_init_module.
    * @param[in]    size   Size to store, need to be an integer multiple of block_len in ftl_init_module.
    * @return   Status.
    * @retval   ESUCCESS  Successful.
    * @retval   EPERM     Attempt to write to an illegal area.
    * @retval   EIO       Flash operation failed.
    * @retval   EINVAL    Offset or size does not meet alignment or range requirements.
    */
int32_t ftl_save_to_module(char *module_name, void *pdata, uint16_t offset, uint16_t size);
/**
    * @brief    Load specified FTL module offset parameter to specified buffer.
    * @param[in]    module_name  Specify FTL module name.
    * @param[out]   pdata  Specify data buffer.
    * @param[in]    offset Specify FTL offset to load, need to be an integer multiple of block_len in ftl_init_module.
    * @param[in]    size   Size to load, need to be an integer multiple of block_len in ftl_init_module.
    * @return   Status.
    * @retval   ESUCCESS  Successful.
    * @retval   EACCES    Attempt to load from a module that hasn't been created.
    * @retval   EINVAL    Offset or size does not meet alignment or range requirements.
    * @retval   ENOF      Data corresponding to the logical address not found.
    * @retval   EVRF      Data verification error.
    * @retval   EIO       Flash operation failed.
    */
int32_t ftl_load_from_module(char *module_name, void *pdata, uint16_t offset, uint16_t size);
/**
    * @brief    Init FTL cache and set FTL cache enable.
    * @param[in]    cache_size  FTL cache size.
    * @return   Status.
    * @retval   true  FTL cache init successfully.
    * @retval   false FTL cache init fail.
    */
bool ftl_cache_init(uint16_t cache_size);
/** @} */ /* End of group FTL_Exported_Functions */

/** @} */ /* End of group FTL */
#ifdef  __cplusplus
}
#endif // __cplusplus
#endif // _FTL_H_
