/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef PROFILE_DEF_H
#define PROFILE_DEF_H

#ifdef __cplusplus
extern "C"
{
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <stdint.h>

/** @defgroup GATT_COMMON GATT Common
  * @{
  */

/** @defgroup GATT_COMMON_API GATT Common API
  * @{
  */

/*============================================================================*
 *                         Macros
 *============================================================================*/
/** @defgroup GATT_COMMON_Exported_Macros GATT Common Exported Macros
  * @{
  */

/** @defgroup GATT_COMMON_MSG_TYPE GATT Common Message Types
  * @{
  */
#define GATT_COMMON_MSG_CREDIT_INFO    0x0001   /**< Information msg type to notify APP about credit.
                                                     The structure of callback data is @ref T_GATT_COMMON_CB_DATA_CREDIT_INFO */

/** End of GATT_COMMON_MSG_TYPE
  * @}
  */

/** End of GATT_COMMON_Exported_Macros
* @}
*/

/*============================================================================*
 *                         Types
 *============================================================================*/
/** @defgroup GATT_COMMON_Exported_Types GATT Common Exported Types
  * @{
  */

/** @brief  Data for @ref GATT_COMMON_MSG_CREDIT_INFO */
typedef struct
{
    uint8_t          credit;
} T_GATT_COMMON_CB_DATA_CREDIT_INFO;

typedef union
{
    void                                  *p_gatt_common_cb_data;

    T_GATT_COMMON_CB_DATA_CREDIT_INFO     *p_gatt_common_cb_data_credit_info;
} T_GATT_COMMON_CB_DATA;
/** End of GATT_COMMON_Exported_Types
* @}
*/

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** @defgroup GATT_COMMON_Exported_Functions GATT Common Exported Functions
  * @{
  */

/**
 * @brief       Callback for common of GATT Server and Client.
 *
 * @param[in] cb_type    Callback message type @ref GATT_COMMON_MSG_TYPE.
 * @param[in] p_cb_data  Pointer to callback data @ref T_GATT_COMMON_CB_DATA.
 *
 * @return APP return result.
 * @retval 0 APP return success.
 * @retval Others APP return failure.
 */
typedef uint16_t(*P_FUN_GATT_COMMON_CB)(uint16_t cb_type, void *p_cb_data);

/**
 * @brief       Register callback function for common of GATT Server and Client.
 *
 * @param[in] callback   Callback function.
 *
 * @return Operation result.
 * @retval true  Operation success.
 * @retval false Operation failure.
 *
 * <b>Example usage</b>
 * \code{.c}
    void test(void)
    {
        bool ret = gatt_common_register_cb(callback);
    }

    // Callback registered by @ref gatt_common_register_cb
    uint16_t callback(uint16_t cb_type, void *p_cb_data)
    {
        uint16_t result = APP_RESULT_SUCCESS;
        T_GATT_COMMON_CB_DATA cb_data;

        memcpy(&cb_data, p_cb_data, sizeof(T_GATT_COMMON_CB_DATA));

        switch (cb_type)
        {
        case GATT_COMMON_MSG_CREDIT_INFO:
            // cb_data.p_gatt_common_cb_data_credit_info->credit
            break;
        ...
        }
 * \endcode
 */
bool gatt_common_register_cb(P_FUN_GATT_COMMON_CB callback);

/** End of GATT_COMMON_Exported_Functions
  * @}
  */

/** End of GATT_COMMON_API
  * @}
  */

/** End of GATT_COMMON
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif
