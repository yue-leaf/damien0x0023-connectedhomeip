/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <string.h>

#if defined(DeviceFamily_CC27XXX10) || defined(DeviceFamily_CC27XXX20)
#include <ti/ble/stack_util/icall/app/icall.h>
#include "ti/ble/stack_util/icall/app/icall_ble_api.h"
#else
#include <icall.h>
#include <icall_ble_api.h>
#endif

#include <util.h>

#include "ti_ble_config.h"
#include "chipOBleProfile.h"

const uint8 chipOBleServUUID[ATT_BT_UUID_SIZE] = { LO_UINT16(CHIPOBLE_SERV_UUID), HI_UINT16(CHIPOBLE_SERV_UUID) };

const uint8 chipOBleProfileTxCharUUID[ATT_UUID_SIZE] = { CHIPOBLEPROFILE_CHAR_UUID_BASE128(CHIPOBLEPROFILE_TX_CHAR_UUID) };

const uint8 chipOBleProfileRxCharUUID[ATT_UUID_SIZE] = { CHIPOBLEPROFILE_CHAR_UUID_BASE128(CHIPOBLEPROFILE_RX_CHAR_UUID) };

#if CHIPOBLE_ENABLE_C3
const uint8 chipOBleProfileC3CharUUID[ATT_UUID_SIZE] = {
    0x04, 0x8f, 0x21, 0x83, 0x8a, 0x74, 0x7d, 0xb8, 0xf2, 0x45, 0x72, 0x87, 0x38, 0x02, 0x63, 0x64
};
#endif

static const gattAttrType_t chipoBleProfile = { ATT_BT_UUID_SIZE, chipOBleServUUID };

static uint8_t chipOBleProfileTxCharProps = GATT_PROP_READ | GATT_PROP_INDICATE;
static uint8_t chipOBleProfileTxCharVal[CHIPOBLEPROFILE_CHAR_LEN] = { 0x00 };
static uint8_t chipOBleProfileTxdDataUserDesp[CHIPOBLEPROFILE_MAX_DESCRIPTION_LEN] = "ChipOBLE Tx Char";
static gattCharCfg_t * chipOBleProfileTxStateDataConfig;

static uint8_t chipOBleProfileRxCharProps = GATT_PROP_WRITE;
static uint8_t chipOBleProfileRxCharVal[CHIPOBLEPROFILE_CHAR_LEN] = { 0x00 };
static uint8_t chipOBleProfileRxdDataUserDesp[CHIPOBLEPROFILE_MAX_DESCRIPTION_LEN] = "ChipOBLE Rx Char";

#if CHIPOBLE_ENABLE_C3
static uint8_t chipOBleProfileC3CharProps = GATT_PROP_READ;
static uint8_t chipOBleProfileC3CharVal[CHIPOBLEPROFILE_CHAR_LEN] = { 0x00 };
static uint8_t chipOBleProfileC3DataUserDesp[CHIPOBLEPROFILE_MAX_DESCRIPTION_LEN] = "ChipOBLE C3 Char";
#endif

static chipOBleProfileCBs_t * chipOBleProfile_AppCBs = NULL;

static gattAttribute_t chipoBleProfileAttrTbl[] = {
    { { ATT_BT_UUID_SIZE, primaryServiceUUID }, GATT_PERMIT_READ, 0, (uint8 *) &chipoBleProfile },

    { { ATT_BT_UUID_SIZE, characterUUID }, GATT_PERMIT_READ, 0, &chipOBleProfileTxCharProps },
    { { ATT_UUID_SIZE, chipOBleProfileTxCharUUID }, GATT_PERMIT_READ, 0, chipOBleProfileTxCharVal },
    { { ATT_BT_UUID_SIZE, clientCharCfgUUID },
      GATT_PERMIT_READ | GATT_PERMIT_WRITE,
      0,
      (uint8 *) &chipOBleProfileTxStateDataConfig },
    { { ATT_BT_UUID_SIZE, charUserDescUUID }, GATT_PERMIT_READ, 0, chipOBleProfileTxdDataUserDesp },

    { { ATT_BT_UUID_SIZE, characterUUID }, GATT_PERMIT_READ, 0, &chipOBleProfileRxCharProps },
    { { ATT_UUID_SIZE, chipOBleProfileRxCharUUID }, GATT_PERMIT_WRITE, 0, chipOBleProfileRxCharVal },
    { { ATT_BT_UUID_SIZE, charUserDescUUID }, GATT_PERMIT_READ, 0, chipOBleProfileRxdDataUserDesp },

#if CHIPOBLE_ENABLE_C3
    { { ATT_BT_UUID_SIZE, characterUUID }, GATT_PERMIT_READ, 0, &chipOBleProfileC3CharProps },
    { { ATT_UUID_SIZE, chipOBleProfileC3CharUUID }, GATT_PERMIT_READ, 0, chipOBleProfileC3CharVal },
    { { ATT_BT_UUID_SIZE, charUserDescUUID }, GATT_PERMIT_READ, 0, chipOBleProfileC3DataUserDesp },
#endif
};

static bStatus_t CHIPoBLEProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t * pAttr, uint8_t * pValue, uint16_t * pLen,
                                            uint16_t offset, uint16_t maxLen, uint8_t method);
static bStatus_t CHIPoBLEProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t * pAttr, uint8_t * pValue, uint16_t len,
                                             uint16_t offset, uint8_t method);

const gattServiceCBs_t chipOBleProfileCBs = {
    CHIPoBLEProfile_ReadAttrCB,
    CHIPoBLEProfile_WriteAttrCB,
    NULL
};

bStatus_t CHIPoBLEProfile_AddService(uint32 services)
{
    uint8 status;

    chipOBleProfileTxStateDataConfig =
        (gattCharCfg_t *) ICall_malloc((uint_least16_t) (sizeof(gattCharCfg_t) * LL_MAX_NUM_BLE_CONNS));
    if (chipOBleProfileTxStateDataConfig == NULL)
    {
        return bleMemAllocError;
    }

    GATTServApp_InitCharCfg(LL_CONNHANDLE_INVALID, chipOBleProfileTxStateDataConfig);

    if (services & CHIPOBLEPROFILE_SERVICE)
    {
        status = GATTServApp_RegisterService(chipoBleProfileAttrTbl, GATT_NUM_ATTRS(chipoBleProfileAttrTbl),
                                             GATT_MAX_ENCRYPT_KEY_SIZE, &chipOBleProfileCBs);
    }
    else
    {
        status = SUCCESS;
    }

    return status;
}

bStatus_t CHIPoBLEProfile_RegisterAppCBs(chipOBleProfileCBs_t * appCallbacks)
{
    if (appCallbacks)
    {
        chipOBleProfile_AppCBs = appCallbacks;
        return SUCCESS;
    }

    return bleAlreadyInRequestedMode;
}

bStatus_t CHIPoBLEProfile_SetParameter(uint8 param, uint8 len, void * value, uint8_t taskId)
{
    switch (param)
    {
    case CHIPOBLEPROFILE_TX_CHAR:
        VOID memcpy(chipOBleProfileTxCharVal, value, len);
        return GATTServApp_ProcessCharCfg(chipOBleProfileTxStateDataConfig, chipOBleProfileTxCharVal, FALSE, chipoBleProfileAttrTbl,
                                          GATT_NUM_ATTRS(chipoBleProfileAttrTbl), taskId, CHIPoBLEProfile_ReadAttrCB);

    default:
        return INVALIDPARAMETER;
    }
}

bStatus_t CHIPoBLEProfile_GetParameter(uint8 param, void * value, uint16_t len)
{
    switch (param)
    {
    case CHIPOBLEPROFILE_RX_CHAR:
        VOID memcpy(value, chipOBleProfileRxCharVal, len < CHIPOBLEPROFILE_CHAR_LEN ? len : CHIPOBLEPROFILE_CHAR_LEN);
        break;

    case CHIPOBLEPROFILE_CCCWrite:
        *((uint8 *) value) = chipOBleProfileTxStateDataConfig->value;
        break;

    default:
        return INVALIDPARAMETER;
    }

    return SUCCESS;
}

static bStatus_t CHIPoBLEProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t * pAttr, uint8_t * pValue, uint16_t * pLen,
                                            uint16_t offset, uint16_t maxLen, uint8_t method)
{
    bStatus_t status = SUCCESS;
    uint16_t len     = 0;

#if CHIPOBLE_ENABLE_C3
    if (!memcmp(pAttr->type.uuid, chipOBleProfileC3CharUUID, pAttr->type.len))
    {
        if ((chipOBleProfile_AppCBs == NULL) || (chipOBleProfile_AppCBs->pfnchipOBleProfileReadAdditionalData == NULL))
        {
            return ATT_ERR_UNLIKELY;
        }

        status = chipOBleProfile_AppCBs->pfnchipOBleProfileReadAdditionalData(chipOBleProfileC3CharVal, &len,
                                                                               CHIPOBLEPROFILE_CHAR_LEN);
        if (status != SUCCESS)
        {
            return status;
        }
    }
    else
#endif
        if (!memcmp(pAttr->type.uuid, chipOBleProfileTxCharUUID, pAttr->type.len))
    {
        len = CHIPOBLEPROFILE_CHAR_LEN;
    }
    else if (pAttr->pValue == chipOBleProfileTxdDataUserDesp)
    {
        len = (uint16_t) strlen((const char *) chipOBleProfileTxdDataUserDesp);
    }
    else if (pAttr->pValue == chipOBleProfileRxdDataUserDesp)
    {
        len = (uint16_t) strlen((const char *) chipOBleProfileRxdDataUserDesp);
    }
#if CHIPOBLE_ENABLE_C3
    else if (pAttr->pValue == chipOBleProfileC3DataUserDesp)
    {
        len = (uint16_t) strlen((const char *) chipOBleProfileC3DataUserDesp);
    }
#endif
    else
    {
        return ATT_ERR_ATTR_NOT_FOUND;
    }

    if (offset > len)
    {
        return ATT_ERR_INVALID_OFFSET;
    }

    len -= offset;
    if (len > maxLen)
    {
        len = maxLen;
    }

    *pLen = len;
    VOID memcpy(pValue, pAttr->pValue + offset, len);
    return status;
}

static bStatus_t CHIPoBLEProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t * pAttr, uint8_t * pValue, uint16_t len,
                                             uint16_t offset, uint8_t method)
{
    bStatus_t status = SUCCESS;
    uint8 notifyApp  = 0xFF;

    if (pAttr->type.len == ATT_UUID_SIZE)
    {
        if (!memcmp(pAttr->type.uuid, chipOBleProfileRxCharUUID, pAttr->type.len))
        {
            VOID memcpy(pAttr->pValue, pValue, len);
            notifyApp = CHIPOBLEPROFILE_RX_CHAR;
        }
        else
        {
            status = ATT_ERR_ATTR_NOT_FOUND;
        }
    }
    else if ((pAttr->type.len == ATT_BT_UUID_SIZE) && (BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]) == GATT_CLIENT_CHAR_CFG_UUID))
    {
        status    = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len, offset, GATT_CLIENT_CFG_INDICATE);
        notifyApp = CHIPOBLEPROFILE_CCCWrite;
    }
    else
    {
        status = ATT_ERR_INVALID_HANDLE;
    }

    if ((notifyApp != 0xFF) && chipOBleProfile_AppCBs && chipOBleProfile_AppCBs->pfnchipOBleProfileChange)
    {
        chipOBleProfile_AppCBs->pfnchipOBleProfileChange(notifyApp, len, connHandle);
    }

    return status;
}
