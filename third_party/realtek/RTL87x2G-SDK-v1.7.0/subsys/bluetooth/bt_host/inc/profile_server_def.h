/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef PROFILE_SERVER_DEF_H
#define PROFILE_SERVER_DEF_H

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

/*============================================================================*
 *                        Header Files
 *============================================================================*/
/** @defgroup GATT_SERVER_API GATT Server API
  * @brief GATT Server API
  * @{
  */
/** @defgroup GATT_SERVER_COMMON_API GATT Server Common API
  * @brief GATT Server Common API
  * @{
  */
/*============================================================================*
 *                         Macros
 *============================================================================*/
/** @defgroup GATT_SERVER_COMMON_Exported_Macros GATT Server Common Exported Macros
  * @brief
  * @{
  */

/** @defgroup General_Service_ID General Service ID
  * @brief Service ID for general profile events.
  * @{
  */
#define SERVICE_PROFILE_GENERAL_ID  0xFF
/** @} */

/** End of GATT_SERVER_COMMON_Exported_Macros
  * @}
  */


/*============================================================================*
 *                         Types
 *============================================================================*/
/** @defgroup GATT_SERVER_COMMON_Exported_Types GATT Server Common Exported Types
  * @brief
  * @{
  */

typedef uint8_t T_SERVER_ID;    //!< Service ID.

/** @brief  GATT write data type*/
typedef enum
{
    WRITE_REQUEST,                      /**< Write request. */
    WRITE_WITHOUT_RESPONSE,             /**< Write without response. */
    WRITE_SIGNED_WITHOUT_RESPONSE,      /**< Signed write without response. */
    WRITE_LONG,                         /**< Write long request. */
} T_WRITE_TYPE;

/** @brief  GATT PDU type*/
typedef enum
{
    GATT_PDU_TYPE_ANY           = 0x00, /**<  Any PDU type. */
    GATT_PDU_TYPE_NOTIFICATION  = 0x01, /**<  Notification PDU type. */
    GATT_PDU_TYPE_INDICATION    = 0x02  /**<  Indication PDU type. */
} T_GATT_PDU_TYPE;

/** @brief Event type to inform APP*/
typedef enum
{
    SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION = 1,    /**< CCCD update event. */
    SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE = 2,              /**< Client read event. */
    SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE = 3,             /**< Client write event. */
} T_SERVICE_CALLBACK_TYPE;

/** @defgroup GATT_SERVER_COMMON_CB_DATA Server Common Callback Data
  * @brief Data for profile to inform application.
  * @{
  */
/** @brief Event ID */
typedef enum
{
    PROFILE_EVT_SRV_REG_COMPLETE,              /**< Services register complete event
    when application calls server_add_service before calling @ref gap_start_bt_stack. */
    PROFILE_EVT_SEND_DATA_COMPLETE,            /**< Notification or indication data send complete event. */
    PROFILE_EVT_SRV_REG_AFTER_INIT_COMPLETE,   /**< Services register complete event
    when application calls server_add_service after receiving @ref GAP_INIT_STATE_STACK_READY. */
    PROFILE_EVT_SRV_CLEAR_AFTER_INIT_COMPLETE, /**< Services clear complete event
    when application calls server_clear_service after receiving @ref GAP_INIT_STATE_STACK_READY. */
    PROFILE_EVT_SERVICE_CHANGE_STATE,          /**< Service change state event. */
} T_SERVER_CB_TYPE;

/** @brief Service reg mode, only be used when: @ref GAP_INIT_STATE_STACK_READY. */
typedef enum
{
    SERVICE_REG_MODE_ADD_TO_STACK = 0,    /**< Default mode. Add service to the Bluetooth Host.
    When transition from @ref SERVICE_REG_MODE_ADD_TO_TABLE to @ref SERVICE_REG_MODE_ADD_TO_STACK, services in table have not been registered will be registered to the Bluetooth Host.
    When in this mode, only one new service can be added to table and registered to Bluetooth Host at one time. */
    SERVICE_REG_MODE_ADD_TO_TABLE = 1,    /**< Add services to table. */
} T_SERVICE_REG_MODE;

/** @brief  The callback data of @ref PROFILE_EVT_SRV_REG_COMPLETE. */
typedef enum
{
    GATT_SERVER_SUCCESS,  /**< Success. */
    GATT_SERVER_FAIL      /**< Fail. */
} T_SERVER_RESULT;

/** @brief  The callback data of @ref PROFILE_EVT_SRV_REG_AFTER_INIT_COMPLETE. */
typedef struct
{
    T_SERVER_RESULT result;
    T_SERVER_ID     service_id;
    uint16_t        cause;
} T_SERVER_REG_AFTER_INIT_RESULT;

/** @brief  The callback data of @ref PROFILE_EVT_SRV_CLEAR_AFTER_INIT_COMPLETE */
typedef struct
{
    uint16_t        cause; /**< Cause. */
    uint16_t        svc_changed_char_cccd_handle; /**< 0x0000: Invalid handle. */
} T_SERVER_CLEAR_SERVICE_AFTER_INIT_RESULT;

/** @brief  The callback data of @ref PROFILE_EVT_SERVICE_CHANGE_STATE. */
typedef struct
{
    uint8_t service_change;             /**< Whether service is changed.
                                          *  - 0:  Service is not changed.
                                          *  - 1:  Service is changed. */
    uint8_t service_change_state;       /**< @ref GATT_SERVER_SERVICE_CHANGE_STATE_BIT_Def. */
    uint16_t conn_handle;               /**< Ignore if service_change is 1. */
} T_SERVER_SERVICE_CHANGE_STATE;

/** @} End of GATT_SERVER_COMMON_CB_DATA */

/** End of GATT_SERVER_COMMON_Exported_Types
  * @}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** @defgroup GATT_SERVER_COMMON_Exported_Functions GATT Server Common Exported Functions
  * @brief
  * @{
  */

/**
 * @brief Initialize parameters of GATT Server.
 *
 * @param[in] service_num Set the number of services that need to register.
 *
 * <b>Example usage</b>
 * \code{.c}
    void app_le_profile_init(void)
    {
        server_init(service_num);
    }
 * \endcode
 */
void server_init(uint8_t service_num);

/**
 * @brief Register built-in services including GAP and GATT services.
 *
 * If the application does not need to register GAP and GATT services,
 * the application shall call @ref server_builtin_service_reg (false) before @ref server_init.
 *
 * @param[in] reg Whether to register built-in services. The default value is true.
 *
 * <b>Example usage</b>
 * \code{.c}
    void app_le_profile_init(void)
    {
        server_builtin_service_reg(false);
        server_init(service_num);
        simp_srv_id = simp_ble_service_add_service(app_profile_callback);
        server_register_app_cb(app_profile_callback);
    }
 * \endcode
 */
void server_builtin_service_reg(bool reg);

/**
 * @brief Get the start handle of the service.
 *
 * @param[in] service_id Service ID.
 * @return Start handle.
 * @retval 0 Operation failure.
 * @retval Others Start handle.
 *
 * <b>Example usage</b>
 * \code{.c}
    void test(void)
    {
        uint16_t start_handle = server_get_start_handle(service_id);
    }
 * \endcode
 */
uint16_t server_get_start_handle(T_SERVER_ID service_id);

/**
 * @brief Configure the server interface.
 *
 * @param[in] use_ext Whether to use the extension API. The default value is false.
 *                    - true: Use the API in @ref GATT_SERVER_EXT_API.
 *                    - false: Use the API in @ref GATT_SERVER_LEGACY_API.
 * @return Operation result.
 * @retval true  Operation success.
 * @retval false Operation failure.
 *
 * <b>Example usage</b>
 * \code{.c}
    void test(void)
    {
        bool ret = server_cfg_use_ext_api(true);
    }
 * \endcode
 */
bool server_cfg_use_ext_api(bool use_ext);

/**
 * @brief Clear service.
 *
 * @note This function can only be used when:
 *                  standby state (i.e., no link, not initiating, etc.), and @ref GAP_INIT_STATE_STACK_READY, and not add service.
 *
 * If sending request operation is successful, the result of clear service will be returned by callback depends on APP:
 * - Default or APP use @ref server_cfg_use_ext_api (false):
 *   callback registered by @ref server_register_app_cb with eventId @ref PROFILE_EVT_SRV_CLEAR_AFTER_INIT_COMPLETE.
 * - APP use @ref server_cfg_use_ext_api (true):
 *   callback registered by @ref server_ext_register_app_cb with eventId @ref PROFILE_EVT_SRV_CLEAR_AFTER_INIT_COMPLETE.
 *
 * If clear service operation is successful and APP not clear bond info:
 * - APP clear CCCD except CCCD of Service Changed characteristic.
 * - APP send Indication of Service Changed after reconnection.
 *
 * @return The result of sending request.
 * @retval true Sending request operation is successful.
 * @retval false Sending request operation is failed.
 *
 * <b>Example usage</b>
 * \code{.c}
    void test(void)
    {
        server_clear_service();
    }

    // Default or APP use server_cfg_use_ext_api(false): callback registered by @ref server_register_app_cb
    T_APP_RESULT app_profile_callback(T_SERVER_ID service_id, void *p_data)
    {
        ......
        if (service_id == SERVICE_PROFILE_GENERAL_ID)
        {
            T_SERVER_APP_CB_DATA *p_param = (T_SERVER_APP_CB_DATA *)p_data;
            switch (p_param->eventId)
            {
            case PROFILE_EVT_SRV_CLEAR_AFTER_INIT_COMPLETE:
                APP_PRINT_INFO2("app_profile_callback: PROFILE_EVT_SRV_CLEAR_AFTER_INIT_COMPLETE, cause 0x%x, svc_changed_char_cccd_handle 0x%x",
                                p_param->event_data.clear_service_after_init_result.cause,
                                p_param->event_data.clear_service_after_init_result.svc_changed_char_cccd_handle);
                ......
                break;
        ......
    }

    // APP use server_cfg_use_ext_api(true): callback registered by @ref server_ext_register_app_cb
    T_APP_RESULT app_profile_ext_callback(T_SERVER_ID service_id, void *p_data)
    {
        ......
        if (service_id == SERVICE_PROFILE_GENERAL_ID)
        {
            T_SERVER_EXT_APP_CB_DATA *p_param = (T_SERVER_EXT_APP_CB_DATA *)p_data;
            switch (p_param->eventId)
            {
            case PROFILE_EVT_SRV_CLEAR_AFTER_INIT_COMPLETE:
                APP_PRINT_INFO2("app_profile_ext_callback: PROFILE_EVT_SRV_CLEAR_AFTER_INIT_COMPLETE, cause 0x%x, svc_changed_char_cccd_handle 0x%x",
                                p_param->event_data.clear_service_after_init_result.cause,
                                p_param->event_data.clear_service_after_init_result.svc_changed_char_cccd_handle);
                ......
                break;
            ......
    }
 * \endcode
 */
bool server_clear_service(void);

/**
 * @brief Set service reg mode.
 *
 * @note This function can only be used when:
 *                  @ref GAP_INIT_STATE_STACK_READY, and not add service.
 *
 * If mode is @ref SERVICE_REG_MODE_ADD_TO_TABLE, setting operation result will be returned.
 *
 * If mode is @ref SERVICE_REG_MODE_ADD_TO_STACK, setting operation result will be returned.
 *
 * If result is true, and p_num is not zero, the result of registering services will be returned by callback depends on APP:
 * - callback registered by @ref server_register_app_cb with eventId @ref PROFILE_EVT_SRV_REG_AFTER_INIT_COMPLETE,
 *   when register single or multiple services complete, return the last service ID.
 *
 * @param[in]     mode  Service reg mode, @ref T_SERVICE_REG_MODE.
 * @param[in,out] p_num Pointer to the num of services in table have not been registered to the Bluetooth Host.
                        Only when mode is @ref SERVICE_REG_MODE_ADD_TO_STACK and result is true, the value is valid.
 * @return Set mode result.
 * @retval true  Set mode success.
 * @retval false Set mode failure.
 *
 * <b>Example usage</b>
 * \code{.c}
    void test(void)
    {
      bool ret = server_set_service_reg_mode(SERVICE_REG_MODE_ADD_TO_TABLE, p_num);

      //add multiple service to table
      bas_srv_id = bas_add_service(app_profile_callback);
      hid_srv_id = hids_add_service(app_profile_callback);
      ......
      ret = server_set_service_reg_mode(SERVICE_REG_MODE_ADD_TO_STACK, p_num);
    }

    T_APP_RESULT app_profile_callback(T_SERVER_ID service_id, void *p_data)
    {
        ......
        if (service_id == SERVICE_PROFILE_GENERAL_ID)
        {
          T_SERVER_APP_CB_DATA *p_param = (T_SERVER_APP_CB_DATA *)p_data;

          switch (p_param->eventId)
          {
            //srv register result event.
            case PROFILE_EVT_SRV_REG_AFTER_INIT_COMPLETE:
            APP_PRINT_INFO3("app_profile_callback: PROFILE_EVT_SRV_REG_AFTER_INIT_COMPLETE: result %d, service id 0x%x, cause 0x%x",
                            p_param->event_data.server_reg_after_init_result.result,
                            //When register multiple services complete, return the last service id.
                            p_param->event_data.server_reg_after_init_result.service_id,
                            p_param->event_data.server_reg_after_init_result.cause);
              ......
              break;
          ......
    }
 * \endcode
 */
bool server_set_service_reg_mode(uint8_t mode, uint8_t *p_num);

/** @} End of GATT_SERVER_COMMON_Exported_Functions */

/** @} End of GATT_SERVER_COMMON_API */

/** @} End of GATT_SERVER_API */

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif /* PROFILE_SERVER_DEF_H */
