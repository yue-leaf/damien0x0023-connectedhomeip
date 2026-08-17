/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _IAS_GATT_CLIENT_H_
#define _IAS_GATT_CLIENT_H_

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

/* Add Includes here */
#include <bt_gatt_client.h>

/** @defgroup IAS_GATT_CLIENT Immediate Alert Service GATT Client
  * @brief IAS service client
  * @details
     IAS Profile is a Profile. IAS LE service please refer to @ref IAS_Service .
  * @{
  */
/*============================================================================*
 *                         Macros
 *============================================================================*/
/** @defgroup IAS_GATT_Client_Exported_Macros IAS GATT Client Exported Macros
  * @brief
  * @{
  */
/** @defgroup IAS_UUIDs
  * @brief IAS UUID definitions
  * @{
  */
#define GATT_UUID_IMMEDIATE_ALERT_SERVICE       0x1802
#define GATT_UUID_CHAR_ALERT_LEVEL              0x2A06
/** @} End of IAS_UUIDs */

/** @defgroup IAS_CLT_CB_MSG
  * @brief IAS client callback messages
  * @{
  */
#define GATT_MSG_IAS_CLIENT_DIS_DONE                  0x00
#define GATT_MSG_IAS_CLIENT_WRITE_ALERT_LEVEL_RESULT  0x01
/** @} End of IAS_CLT_CB_MSG */

/** End of IAS_GATT_Client_Exported_Macros
  * @}
  */

typedef T_APP_RESULT(*P_FUN_IAS_CLIENT_APP_CB)(uint16_t conn_handle, uint8_t type, void *p_data);

/*============================================================================*
 *                         Types
 *============================================================================*/
/** @defgroup IAS_GATT_Client_Exported_Types IAS GATT Client Exported Types
  * @brief
  * @{
  */

/** @brief IAS alert level*/
typedef enum
{
    IAS_NO_ALERT = 0x00,
    IAS_MILD_ALERT,
    IAS_HIGH_ALERT
} T_IAS_ALERT_LEVEL;

/** @brief IAS client discovery result
 *         The message data for GATT_MSG_IAS_CLIENT_DIS_DONE.
*/
typedef struct
{
    bool     is_found;          /**< Whether to find service. */
    bool     load_from_ftl;     /**< Whether the service table is loaded from FTL. */
} T_IAS_CLIENT_DIS_DONE;

/** @brief IAS client write result
 *         The message data for GATT_MSG_IAS_CLIENT_WRITE_ALERT_LEVEL_RESULT.
*/
typedef struct
{
    uint16_t cause;
} T_IAS_CLIENT_WRITE_ALERT_LEVEL_RESULT;
/** End of IAS_GATT_Client_Exported_Types * @} */

/** @defgroup IAS_GATT_Client_Exported_Functions IAS GATT Client Exported Functions
  * @{
  */

/**
  * @brief      Add IAS service client to application.
  * @param[in]  app_cb Pointer of APP callback function to handle specific client module data.
  * @return Result of add the specific client module.
  * @retval true  Add client module success.
  * @retval false Add client module failed.
  *
  * <b>Example usage</b>
  * \code{.c}
    void app_le_profile_init(void)
    {
        if(!ias_client_init(app_ias_client_callback))
        {
            APP_PRINT_ERROR0("app_le_profile_init: add ias client fail!");
        }
    }
  * \endcode
  */
bool ias_client_init(P_FUN_IAS_CLIENT_APP_CB app_cb);

/**
  * @brief  Used by application, to write alert level characteristic value.
  * @param[in]  conn_handle Connection handle of the ACL link.
  * @param[in]  alert_level IAS alert level: @ref T_IAS_ALERT_LEVEL.
  * @retval true send write command to Bluetooth Host success.
  * @retval false send write command to Bluetooth Host failed.
  *
  * <b>Example usage</b>
  * \code{.c}
    void test(uint16_t conn_handle)
    {
        if(!ias_client_write_alert_level(conn_handle, IAS_NO_ALERT))
        {
            APP_PRINT_ERROR0("write alert level fail!");
        }
    }
  * \endcode
  */
bool ias_client_write_alert_level(uint16_t conn_handle, T_IAS_ALERT_LEVEL alert_level);

/** @} End of IAS_GATT_Client_Exported_Functions */

/** @} End of IAS_GATT_CLIENT */


#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif  /* _IAS_GATT_CLIENT_H_ */
