/*
 * Copyright (c) 2026 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "StatusScreen.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <vector>

namespace {
using Frame = std::vector<uint8_t>;

Frame Render(const StatusScreen & screen, const DisplayStatus & status)
{
    Frame frame(240 * 480);
    uint8_t guarded[482];
    for (uint16_t y = 0; y < 240; ++y)
    {
        memset(guarded, 0xA5, sizeof(guarded));
        screen.RenderRow(y, guarded + 1, status);
        assert(guarded[0] == 0xA5 && guarded[481] == 0xA5);
        memcpy(frame.data() + y * 480, guarded + 1, 480);
        for (int x = 0; x < 240; ++x)
            if (x < 10 || x >= 230)
                assert(guarded[x * 2 + 1] == 0xFF && guarded[x * 2 + 2] == 0xFF);
    }
    return frame;
}

void CheckChange(const Frame & before, const Frame & after, int top, int bottom)
{
    bool changed = false;
    for (int y = 0; y < 240; ++y)
    {
        const bool equal = memcmp(before.data() + y * 480, after.data() + y * 480, 480) == 0;
        if (y < top || y >= bottom)
            assert(equal);
        else
            changed |= !equal;
    }
    assert(changed);
}

bool HasInk(const Frame & frame, int top, int bottom)
{
    for (int y = top; y < bottom; ++y)
        for (int x = 10; x < 230; ++x)
            if (frame[static_cast<size_t>(y * 480 + x * 2)] != 0xFF)
                return true;
    return false;
}

void WritePreview(const Frame & frame, const char * path)
{
    FILE * file = fopen(path, "wb");
    assert(file != nullptr);
    fprintf(file, "P6\n240 240\n255\n");
    for (size_t i = 0; i < frame.size(); i += 2)
    {
        const unsigned pixel = static_cast<unsigned>(frame[i]) * 256 + frame[i + 1];
        const uint8_t rgb[]  = { static_cast<uint8_t>(((pixel >> 11) & 31) * 255 / 31),
                                 static_cast<uint8_t>(((pixel >> 5) & 63) * 255 / 63),
                                 static_cast<uint8_t>((pixel & 31) * 255 / 31) };
        assert(fwrite(rgb, 1, 3, file) == 3);
    }
    assert(fclose(file) == 0);
}
} // namespace

int main(int argc, char ** argv)
{
    static_assert(sizeof(StatusScreen) <= 160, "Status renderer must remain small");
    DeviceDisplayInfo info;
    info.vendorId      = 0xFFF1;
    info.productId     = 0x8001;
    info.vendorIdValid = info.productIdValid = info.serialNumberValid = true;
    info.softwareVersionValid = info.softwareVersionNumberValid = true;
    strcpy(info.serialNumber, "TI2755R10-20261011-0001");
    strcpy(info.softwareVersion, "1.0.0");
    info.softwareVersionNumber = 1;
    StatusScreen screen;
    screen.Init(info);
    DisplayStatus status;
    status.network    = DisplayNetworkStatus::kConnected;
    status.lightKnown = status.lightOn = true;
    Frame connected                    = Render(screen, status);
    assert(HasInk(connected, 10, 31) && HasInk(connected, 50, 64) && HasInk(connected, 75, 96));
    assert(HasInk(connected, 117, 129) && HasInk(connected, 157, 169) && HasInk(connected, 209, 221));

    info.vendorId  = 0;
    info.productId = UINT16_MAX;
    screen.Init(info);
    CheckChange(connected, Render(screen, status), 117, 129);
    info.vendorId              = 0xFFF1;
    info.productId             = 0x8001;
    info.softwareVersionNumber = UINT32_MAX;
    screen.Init(info);
    CheckChange(connected, Render(screen, status), 209, 235);
    info.softwareVersionNumber      = 1;
    info.softwareVersionNumberValid = false;
    screen.Init(info);
    CheckChange(connected, Render(screen, status), 209, 235);
    info.softwareVersionNumberValid = true;
    screen.Init(info);

    status.network     = DisplayNetworkStatus::kDisconnected;
    Frame disconnected = Render(screen, status);
    CheckChange(connected, disconnected, 50, 64);
    status.lightOn = false;
    Frame off      = Render(screen, status);
    CheckChange(disconnected, off, 75, 96);
    status.identifying = true;
    Frame identifying  = Render(screen, status);
    CheckChange(off, identifying, 33, 43);
    status.network = DisplayNetworkStatus::kConnecting;
    CheckChange(identifying, Render(screen, status), 50, 64);
    status.network = DisplayNetworkStatus::kUnknown;
    Render(screen, status);
    status.lightKnown = false;
    Render(screen, status);

    // Max-length strings, preserving every SN character and the numeric version.
    memset(info.serialNumber, 'W', 32);
    info.serialNumber[31] = 'Z';
    memset(info.softwareVersion, 'v', 64);
    info.softwareVersionNumber = UINT32_MAX;
    screen.Init(info);
    Frame longFields = Render(screen, status);
    assert(HasInk(longFields, 171, 183) && HasInk(longFields, 227, 234));
    // The wrapped final SN character is Z, whose top row has all five columns set.
    for (int x = 10; x < 16; ++x)
        assert(longFields[171 * 480 + static_cast<size_t>(x * 2)] != 0xFF);

    info.serialNumberValid = info.softwareVersionValid = false;
    screen.Init(info);
    Frame missingStrings   = Render(screen, status);
    info.serialNumberValid = info.softwareVersionValid = true;
    memset(info.serialNumber, 'X', sizeof(info.serialNumber));
    memset(info.softwareVersion, 'X', sizeof(info.softwareVersion));
    screen.Init(info);
    assert(Render(screen, status) == missingStrings);
    memset(info.serialNumber, 0, sizeof(info.serialNumber));
    memset(info.softwareVersion, 0, sizeof(info.softwareVersion));
    screen.Init(info);
    assert(Render(screen, status) == missingStrings);
    screen.Init({});
    Frame unavailable = Render(screen, status);
    assert(HasInk(unavailable, 117, 129) && HasInk(unavailable, 157, 169) && HasInk(unavailable, 209, 221));

    if (argc == 6)
    {
        WritePreview(connected, argv[1]);
        WritePreview(off, argv[2]);
        WritePreview(identifying, argv[3]);
        WritePreview(longFields, argv[4]);
        WritePreview(unavailable, argv[5]);
    }
    puts("Status screen tests passed");
    return 0;
}
