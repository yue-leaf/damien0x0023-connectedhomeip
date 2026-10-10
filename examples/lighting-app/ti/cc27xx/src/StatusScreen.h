/*
 * Copyright (c) 2026 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

enum class DisplayNetworkStatus : uint8_t
{
    kUnknown,
    kConnecting,
    kConnected,
    kDisconnected,
};

struct DeviceDisplayInfo
{
    char serialNumber[33]           = {};
    char softwareVersion[65]        = {};
    uint32_t softwareVersionNumber  = 0;
    uint16_t vendorId               = 0;
    uint16_t productId              = 0;
    bool vendorIdValid              = false;
    bool productIdValid             = false;
    bool serialNumberValid          = false;
    bool softwareVersionValid       = false;
    bool softwareVersionNumberValid = false;
};

struct DisplayStatus
{
    DisplayNetworkStatus network = DisplayNetworkStatus::kUnknown;
    bool lightOn                 = false;
    bool lightKnown              = false;
    bool identifying             = false;

    bool operator==(const DisplayStatus & other) const
    {
        return network == other.network && lightOn == other.lightOn && lightKnown == other.lightKnown &&
            identifying == other.identifying;
    }
};

class StatusScreen
{
public:
    static constexpr uint16_t kWidth  = 240;
    static constexpr uint16_t kHeight = 240;

    void Init(const DeviceDisplayInfo & info);
    void RenderRow(uint16_t y, uint8_t * rgb565, const DisplayStatus & status) const;

private:
    char mVendorId[12]        = {};
    char mProductId[12]       = {};
    char mSerialNumber[33]    = {};
    char mSoftwareVersion[80] = {};
};
