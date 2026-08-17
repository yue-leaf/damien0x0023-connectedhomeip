/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <trace.h>
#include <stdio.h>
#include <string.h>
#include "gap.h"
#include "gap_adv.h"
#include "gap_bond_le.h"
#include "profile_server.h"
#include "gap_msg.h"
#include "dfu_service.h"
#include "app_msg.h"
#include "dfu_app.h"
#include "dfu_main.h"
#include "dfu_common.h"
#include "ble_dfu_transport.h"
#include "gap_conn_le.h"
#include "os_timer.h"
#include "fmc_api.h"



#if (SUPPORT_NORMAL_OTA == 1)
/** @defgroup  DFU_APP DFU TASK Application
    * @brief This file handles BLE DFU TASK application routines.
    * @{
    */
/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @defgroup  DFU_APP_Exported_Macros DFU App Exported Macros
  * @brief
  * @{
  */
T_GAP_DEV_STATE dfu_gap_cur_state = {0, 0, 0, 0};
T_GAP_CONN_STATE dfu_gap_conn_state = GAP_CONN_STATE_DISCONNECTED;

/** End of DFU_APP_Exported_Macros
  * @}
  */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup DFU_APP_Exported_Functions DFU App Exported Functions
  * @brief
  * @{
  */
void dfu_periph_handle_gap_msg(T_IO_MSG  *p_gap_msg);
/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void dfu_handle_io_msg(T_IO_MSG io_msg)
{
    uint16_t msg_type = io_msg.type;

    switch (msg_type)
    {
    case IO_MSG_TYPE_BT_STATUS:
        {
            dfu_periph_handle_gap_msg(&io_msg);
        }
        break;
    default:
        break;
    }
}

/**
  * @brief    DFU set advertising parameter
  * @param    void
  * @return   void
  */
void dfu_set_adv_param(void)
{
    char fmt[] = "%02x%02x%02x%02x%02x%02x";
    char device_name[GAP_DEVICE_NAME_LEN];

    uint8_t bt_bd_addr[6] ;
    gap_get_param(GAP_PARAM_BD_ADDR, bt_bd_addr);
    DFU_PRINT_INFO1("dfu_set_adv_param: bt_bd_addr %s", TRACE_BDADDR(bt_bd_addr));
    sprintf(device_name, fmt,
            bt_bd_addr[5], bt_bd_addr[4], bt_bd_addr[3],
            bt_bd_addr[2], bt_bd_addr[1], bt_bd_addr[0]
           );
    DFU_PRINT_INFO1("dfu_set_adv_param: device name %s", TRACE_STRING(device_name));

    //advertising data
    uint8_t adv_data_uuid128[31] =
    {
        /* Flags */
        0x02,                           /* length     */
        GAP_ADTYPE_FLAGS,               /* type="flags" */
        GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

        /* Service */
        0x11,                           /* length     */
        GAP_ADTYPE_128BIT_COMPLETE,     /* type="Complete list of 128-bit UUIDs" */
        BLE_GATT_UUID128_DFU_SERVICE,

        9,
        0xFF,
        0x5D,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    };
    adv_data_uuid128[25] = bt_bd_addr[5];
    adv_data_uuid128[26] = bt_bd_addr[4];
    adv_data_uuid128[27] = bt_bd_addr[3];
    adv_data_uuid128[28] = bt_bd_addr[2];
    adv_data_uuid128[29] = bt_bd_addr[1];
    adv_data_uuid128[30] = bt_bd_addr[0];

    le_set_gap_param(GAP_PARAM_DEVICE_NAME, GAP_DEVICE_NAME_LEN, device_name);
    le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(adv_data_uuid128), adv_data_uuid128);
}

/**
 * @brief    Handle msg GAP_MSG_LE_DEV_STATE_CHANGE
 * @note     All the gap device state events are pre-handled in this function.
 *           Then the event handling function shall be called according to the new_state
 * @param[in] new_state  New gap device state
 * @param[in] cause GAP device state change cause
 * @return   void
 */
void dfu_periph_handle_dev_state_evt(T_GAP_DEV_STATE new_state, uint16_t cause)
{
    DFU_PRINT_INFO4("dfu_periph_handle_dev_state_evt: init state %d, adv state %d, conn state %d, cause 0x%x",
                    new_state.gap_init_state, new_state.gap_adv_state,
                    new_state.gap_conn_state, cause);
    if (dfu_gap_cur_state.gap_init_state != new_state.gap_init_state)
    {
        if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY)
        {
            APP_PRINT_INFO0("GAP stack ready");
            dfu_set_rand_addr();

            dfu_set_adv_param();

            /*stack ready*/
            le_adv_start();
        }
    }

    if (dfu_gap_cur_state.gap_adv_state != new_state.gap_adv_state)
    {
        if (new_state.gap_adv_state == GAP_ADV_STATE_IDLE)
        {
            if (new_state.gap_adv_sub_state == GAP_ADV_TO_IDLE_CAUSE_CONN)
            {
                DFU_PRINT_INFO0("GAP adv stopped: because connection created");
            }
            else
            {
                DFU_PRINT_INFO0("GAP adv stopped");
            }
        }
        else if (new_state.gap_adv_state == GAP_ADV_STATE_ADVERTISING)
        {
            DFU_PRINT_INFO0("GAP adv start");
        }
    }

    if (dfu_gap_cur_state.gap_conn_state != new_state.gap_conn_state)
    {
        DFU_PRINT_INFO2("conn state: %d -> %d",
                        dfu_gap_cur_state.gap_conn_state,
                        new_state.gap_conn_state);
    }
    dfu_gap_cur_state = new_state;
}

void dfu_periph_handle_conn_state_evt(uint8_t conn_id, T_GAP_CONN_STATE new_state,
                                      uint16_t disc_cause)
{
    DFU_PRINT_INFO3("dfu_periph_handle_conn_state_evt: conn_id = %d old_state = %d new_state = %d",
                    conn_id, dfu_gap_conn_state, new_state);
    switch (new_state)
    {
    case GAP_CONN_STATE_DISCONNECTED:
        {
            if ((disc_cause != (HCI_ERR | HCI_ERR_REMOTE_USER_TERMINATE))
                && (disc_cause != (HCI_ERR | HCI_ERR_LOCAL_HOST_TERMINATE)))
            {
                DFU_PRINT_ERROR1("connection lost: cause 0x%x", disc_cause);
            }

            if (dfu_active_reset_pending)
            {
                dfu_active_reset_pending = false;

                /*when multi image in temp, need goto OTA mode*/
                if (dfu_active_reset_to_ota_mode)
                {
                    DBG_DIRECT("comb Normal OTA, Reset to OTA Mode");
                    dfu_active_reset_to_ota_mode = false;
                    dfu_switch_to_ota_mode();
                }
                else
                {
#if (ENABLE_AUTO_BANK_SWITCH == 1)
                    if (is_ota_support_bank_switch())
                    {
                        uint32_t ota_addr;
                        ota_addr = get_header_addr_by_img_id(IMG_OTA);
                        DBG_DIRECT("FOR QC Test: Bank switch Erase OTA Header=0x%x", ota_addr);
                        fmc_flash_nor_erase(ota_addr, FMC_FLASH_NOR_ERASE_SECTOR);
                    }
#endif

                    //unlock_flash_bp_all();
                    chip_reset(RESET_ALL);
                }

            }
            else
            {
                dfu_fw_reboot(RESET_ALL, DFU_LINK_LOST_RESET);
            }
        }
        break;

    case GAP_CONN_STATE_CONNECTED:
        {
            uint16_t conn_interval;
            uint16_t conn_latency;
            uint16_t conn_supervision_timeout;
            uint8_t  remote_bd[6];
            T_GAP_REMOTE_ADDR_TYPE remote_bd_type;

            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_latency, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
            le_get_conn_addr(conn_id, remote_bd, (unsigned char *)&remote_bd_type);
            DFU_PRINT_INFO5("GAPSTATE_CONN_CONNECTED:remote_bd %s, remote_addr_type %d, conn_interval 0x%x, conn_latency 0x%x, conn_supervision_timeout 0x%x",
                            TRACE_BDADDR(remote_bd), remote_bd_type,
                            conn_interval, conn_latency, conn_supervision_timeout);

            os_timer_stop(&normal_ota_wait4_conn_timer_handle);
        }
        break;

    default:
        break;
    }
    dfu_gap_conn_state = new_state;
}

/**
 * @brief    Handle msg GAP_MSG_LE_AUTHEN_STATE_CHANGE
 * @note     All the gap authentication state events are pre-handled in this function.
 *           Then the event handling function shall be called according to the new_state
 * @param[in] conn_id Connection ID
 * @param[in] new_state  New authentication state
 * @param[in] cause Use this cause when new_state is GAP_AUTHEN_STATE_COMPLETE
 * @return   void
 */
void dfu_periph_handle_authen_state_evt(uint8_t conn_id, uint8_t new_state, uint16_t cause)
{
    DFU_PRINT_INFO2("dfu_periph_handle_authen_state_evt:conn_id %d, cause 0x%x", conn_id, cause);

    switch (new_state)
    {
    case GAP_AUTHEN_STATE_STARTED:
        {
            DFU_PRINT_INFO0("dfu_periph_handle_authen_state_evt: GAP_AUTHEN_STATE_STARTED");
        }
        break;

    case GAP_AUTHEN_STATE_COMPLETE:
        {
            if (cause == GAP_SUCCESS)
            {
                DFU_PRINT_INFO0("dfu_periph_handle_authen_state_evt: GAP_AUTHEN_STATE_COMPLETE pair success");
            }
            else
            {
                DFU_PRINT_INFO0("dfu_periph_handle_authen_state_evt: GAP_AUTHEN_STATE_COMPLETE pair failed");
            }
        }
        break;

    default:
        {
            DFU_PRINT_ERROR1("dfu_periph_handle_authen_state_evt: unknown newstate %d", new_state);
        }
        break;
    }
}

/**
 * @brief    Handle msg GAP_MSG_LE_CONN_MTU_INFO
 * @note     This msg is used to inform APP that exchange mtu procedure is completed.
 * @param[in] conn_id Connection ID
 * @param[in] mtu_size  New mtu size
 * @return   void
 */
void dfu_periph_handle_conn_mtu_info_evt(uint8_t conn_id, uint16_t mtu_size)
{
    DFU_PRINT_INFO2("app_handle_conn_mtu_info_evt: conn_id %d, mtu_size %d", conn_id, mtu_size);
}

/**
 * @brief    Handle msg GAP_MSG_LE_CONN_PARAM_UPDATE
 * @note     All the connection parameter update change  events are pre-handled in this function.
 * @param[in] conn_id Connection ID
 * @param[in] status  New update state
 * @param[in] cause Use this cause when status is GAP_CONN_PARAM_UPDATE_STATUS_FAIL
 * @return   void
 */
void dfu_periph_conn_param_update_evt(uint8_t conn_id, uint8_t status, uint16_t cause)
{
    switch (status)
    {
    case GAP_CONN_PARAM_UPDATE_STATUS_SUCCESS:
        {
            uint16_t conn_interval;
            uint16_t conn_slave_latency;
            uint16_t conn_supervision_timeout;

            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_slave_latency, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
            DFU_PRINT_INFO3("dfu_periph_handle_conn_param_update_evt update success:conn_interval 0x%x, conn_slave_latency 0x%x, conn_supervision_timeout 0x%x",
                            conn_interval, conn_slave_latency, conn_supervision_timeout);

            dfu_notify_conn_para_update_req(conn_id, DFU_ARV_SUCCESS);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_FAIL:
        {
            DFU_PRINT_ERROR1("dfu_periph_handle_conn_param_update_evt update failed: cause 0x%x", cause);
            dfu_notify_conn_para_update_req(conn_id, DFU_ARV_FAIL_OPERATION);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_PENDING:
        {
            DFU_PRINT_INFO0("dfu_periph_handle_conn_param_update_evt update pending.");
        }
        break;

    default:
        break;
    }
}

/**
 * @brief    All the BT GAP MSG are pre-handled in this function.
 * @note     Then the event handling function shall be called according to the
 *           subtype of T_IO_MSG
 * @param[in] p_gap_msg Pointer to GAP msg
 * @return   void
 */
void dfu_periph_handle_gap_msg(T_IO_MSG *p_gap_msg)
{
    T_LE_GAP_MSG gap_msg;
    uint8_t conn_id;
    memcpy(&gap_msg, &p_gap_msg->u.param, sizeof(p_gap_msg->u.param));

    DFU_PRINT_TRACE1("dfu_peripheral_handle_gap_msg: subtype %d", p_gap_msg->subtype);
    switch (p_gap_msg->subtype)
    {
    case GAP_MSG_LE_DEV_STATE_CHANGE:
        {
            dfu_periph_handle_dev_state_evt(gap_msg.msg_data.gap_dev_state_change.new_state,
                                            gap_msg.msg_data.gap_dev_state_change.cause);
        }
        break;

    case GAP_MSG_LE_CONN_STATE_CHANGE:
        {
            dfu_periph_handle_conn_state_evt(gap_msg.msg_data.gap_conn_state_change.conn_id,
                                             (T_GAP_CONN_STATE)gap_msg.msg_data.gap_conn_state_change.new_state,
                                             gap_msg.msg_data.gap_conn_state_change.disc_cause);
        }
        break;

    case GAP_MSG_LE_CONN_MTU_INFO:
        {
            dfu_periph_handle_conn_mtu_info_evt(gap_msg.msg_data.gap_conn_mtu_info.conn_id,
                                                gap_msg.msg_data.gap_conn_mtu_info.mtu_size);
        }
        break;

    case GAP_MSG_LE_CONN_PARAM_UPDATE:
        {
            dfu_periph_conn_param_update_evt(gap_msg.msg_data.gap_conn_param_update.conn_id,
                                             gap_msg.msg_data.gap_conn_param_update.status,
                                             gap_msg.msg_data.gap_conn_param_update.cause);
        }
        break;

    case GAP_MSG_LE_AUTHEN_STATE_CHANGE:
        {
            dfu_periph_handle_authen_state_evt(gap_msg.msg_data.gap_authen_state.conn_id,
                                               gap_msg.msg_data.gap_authen_state.new_state,
                                               gap_msg.msg_data.gap_authen_state.status);
        }
        break;

    case GAP_MSG_LE_BOND_JUST_WORK:
        {
            conn_id = gap_msg.msg_data.gap_bond_just_work_conf.conn_id;
            le_bond_just_work_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
            DFU_PRINT_INFO0("GAP_MSG_LE_BOND_JUST_WORK");
        }
        break;

    case GAP_MSG_LE_BOND_PASSKEY_DISPLAY:
        {
            uint32_t display_value = 0;
            conn_id = gap_msg.msg_data.gap_bond_passkey_display.conn_id;
            le_bond_get_display_key(conn_id, &display_value);
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_PASSKEY_DISPLAY:passkey %d", display_value);
            le_bond_passkey_display_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_USER_CONFIRMATION:
        {
            uint32_t display_value = 0;
            conn_id = gap_msg.msg_data.gap_bond_user_conf.conn_id;
            le_bond_get_display_key(conn_id, &display_value);
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_USER_CONFIRMATION: passkey %d", display_value);
            le_bond_user_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_PASSKEY_INPUT:
        {
            uint32_t passkey = 888888;
            conn_id = gap_msg.msg_data.gap_bond_passkey_input.conn_id;
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_PASSKEY_INPUT: conn_id %d", conn_id);
            le_bond_passkey_input_confirm(conn_id, passkey, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_OOB_INPUT:
        {
            uint8_t oob_data[GAP_OOB_LEN] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            conn_id = gap_msg.msg_data.gap_bond_oob_input.conn_id;
            APP_PRINT_INFO0("GAP_MSG_LE_BOND_OOB_INPUT");
            le_bond_set_param(GAP_PARAM_BOND_OOB_DATA, GAP_OOB_LEN, oob_data);
            le_bond_oob_input_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    default:
        APP_PRINT_ERROR1("app_handle_gap_msg: unknown subtype %d", p_gap_msg->subtype);
        break;
    }
}

/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msg type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT dfu_gap_callback(uint8_t cb_type, void *p_cb_data)
{
    T_APP_RESULT result = APP_RESULT_SUCCESS;
    T_LE_CB_DATA *p_data = (T_LE_CB_DATA *)p_cb_data;

    switch (cb_type)
    {
    case GAP_MSG_LE_DATA_LEN_CHANGE_INFO:
        DFU_PRINT_INFO3("GAP_MSG_LE_DATA_LEN_CHANGE_INFO: conn_id %d, tx octets 0x%x, max_tx_time 0x%x",
                        p_data->p_le_data_len_change_info->conn_id,
                        p_data->p_le_data_len_change_info->max_tx_octets,
                        p_data->p_le_data_len_change_info->max_tx_time);
        break;

    case GAP_MSG_LE_BOND_MODIFY_INFO:
        DFU_PRINT_INFO1("GAP_MSG_LE_BOND_MODIFY_INFO: type 0x%x",
                        p_data->p_le_bond_modify_info->type);
        break;

    case GAP_MSG_LE_MODIFY_WHITE_LIST:
        DFU_PRINT_INFO2("GAP_MSG_LE_MODIFY_WHITE_LIST: operation %d, cause 0x%x",
                        p_data->p_le_modify_white_list_rsp->operation,
                        p_data->p_le_modify_white_list_rsp->cause);
        break;
    case GAP_MSG_LE_SET_RAND_ADDR:
        DFU_PRINT_INFO1("GAP_MSG_LE_SET_RAND_ADDR: cause 0x%x",
                        p_data->p_le_set_rand_addr_rsp->cause);
        break;
    default:
        DFU_PRINT_INFO1("app_gap_callback: unhandled cb_type 0x%x", cb_type);
        break;
    }
    return result;
}


/**
    * @brief    All the BT Profile service callback events are handled in this function
    * @note     Then the event handling function shall be called according to the
    *           service_id
    * @param    service_id  Profile service ID
    * @param    p_data      Pointer to callback data
    * @return   T_APP_RESULT, which indicates the function call is successful or not
    * @retval   APP_RESULT_SUCCESS  Function run successfully
    * @retval   others              Function run failed, and return number indicates the reason
    */
T_APP_RESULT dfu_profile_callback(T_SERVER_ID service_id, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    if (service_id == SERVICE_PROFILE_GENERAL_ID)
    {
        T_SERVER_APP_CB_DATA *p_param = (T_SERVER_APP_CB_DATA *)p_data;
        switch (p_param->eventId)
        {
        case PROFILE_EVT_SRV_REG_COMPLETE:// srv register result event.
            DFU_PRINT_INFO1("PROFILE_EVT_SRV_REG_COMPLETE: result %d",
                            p_param->event_data.service_reg_result);
            break;

        case PROFILE_EVT_SEND_DATA_COMPLETE:
            DFU_PRINT_INFO5("PROFILE_EVT_SEND_DATA_COMPLETE: conn_id %d, cause 0x%x, service_id %d, attrib_idx 0x%x, credits = %d",
                            p_param->event_data.send_data_result.conn_id,
                            p_param->event_data.send_data_result.cause,
                            p_param->event_data.send_data_result.service_id,
                            p_param->event_data.send_data_result.attrib_idx,
                            p_param->event_data.send_data_result.credits);
            if (p_param->event_data.send_data_result.cause == GAP_SUCCESS)
            {
                DFU_PRINT_INFO0("PROFILE_EVT_SEND_DATA_COMPLETE success");
            }
            else
            {
                DFU_PRINT_ERROR0("PROFILE_EVT_SEND_DATA_COMPLETE failed");
            }
            break;

        default:
            break;
        }
    }
    else if (service_id == rtk_dfu_service_id)
    {
        T_DFU_CALLBACK_DATA *p_cb_data = (T_DFU_CALLBACK_DATA *)p_data;
        switch (p_cb_data->msg_type)
        {
        case SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION:
            {
                if (p_cb_data->msg_data.notification_indification_index == DFU_CP_NOTIFY_ENABLE)
                {
                    DFU_PRINT_INFO0("Normal dfu notification enable");
                }
                else if (p_cb_data->msg_data.notification_indification_index ==
                         DFU_CP_NOTIFY_DISABLE)
                {
                    DFU_PRINT_INFO0("Normal dfu notification disable");
                }
            }
            break;
        case SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE:
            {

            }
            break;
        default:
            break;
        }
    }
    else
    {
    }

    return app_result;
}
#endif

/** End of DFU_APP_Exported_Functions
  * @}
  */

/** End of DFU_APP
  * @}
  */

