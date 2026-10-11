/*
 * Copyright (c) 2026 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "StatusFontData.h"
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

template <size_t Height>
void CheckGlyph(const Frame & frame, int left, int top, int width, const uint16_t (&glyph)[Height])
{
    for (size_t y = 0; y < Height; ++y)
        for (int x = 0; x < width; ++x)
        {
            const size_t offset  = (static_cast<size_t>(top) + y) * 480 + static_cast<size_t>((left + x) * 2);
            const uint16_t pixel = static_cast<uint16_t>(frame[offset] * 256 + frame[offset + 1]);
            assert(pixel == ((glyph[y] & (1U << x)) ? 0x0000 : 0xFFFF));
        }
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
    strcpy(info.serialNumber, "0123456789ABCDEF0123456789ABCDEF");
    strcpy(info.softwareVersion, "1.0.1+1");
    info.softwareVersionNumber = 1;
    StatusScreen screen;
    screen.Init(info);
    DisplayStatus status;
    status.network    = DisplayNetworkStatus::kConnected;
    status.lightKnown = status.lightOn = true;
    Frame connected                    = Render(screen, status);
    assert(HasInk(connected, 6, 34) && HasInk(connected, 40, 58) && HasInk(connected, 64, 92));
    assert(HasInk(connected, 100, 118) && HasInk(connected, 140, 178) && HasInk(connected, 200, 218));
    CheckGlyph(connected, 10, 140, 9, StatusFont::kBody['0' - 32]);
    CheckGlyph(connected, 10, 160, 9, StatusFont::kBody['0' - 32]);
    CheckGlyph(connected, 175, 160, 9, StatusFont::kBody['F' - 32]);

    info.vendorId  = 0;
    info.productId = UINT16_MAX;
    screen.Init(info);
    CheckChange(connected, Render(screen, status), 100, 118);
    info.vendorId              = 0xFFF1;
    info.productId             = 0x8001;
    info.softwareVersionNumber = UINT32_MAX;
    screen.Init(info);
    CheckChange(connected, Render(screen, status), 200, 238);
    info.softwareVersionNumber      = 1;
    info.softwareVersionNumberValid = false;
    screen.Init(info);
    CheckChange(connected, Render(screen, status), 200, 238);
    info.softwareVersionNumberValid = true;
    screen.Init(info);

    status.network     = DisplayNetworkStatus::kDisconnected;
    Frame disconnected = Render(screen, status);
    CheckChange(connected, disconnected, 40, 58);
    status.lightOn = false;
    Frame off      = Render(screen, status);
    CheckChange(disconnected, off, 64, 92);
    status.identifying = true;
    Frame identifying  = Render(screen, status);
    CheckChange(off, identifying, 16, 28);
    // Identify must not overwrite any part of the large product title.
    for (int y = 6; y < 34; ++y)
        assert(memcmp(off.data() + y * 480, identifying.data() + y * 480, 166 * 2) == 0);
    status.network = DisplayNetworkStatus::kConnecting;
    CheckChange(identifying, Render(screen, status), 40, 58);
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
    assert(HasInk(longFields, 160, 178) && HasInk(longFields, 226, 238));
    CheckGlyph(longFields, 175, 160, 9, StatusFont::kBody['Z' - 32]);
    CheckGlyph(longFields, 186, 226, 7, StatusFont::kCompact[')' - 32]);

    // Exercise both font choices at every line boundary without losing the tail.
    info.softwareVersionNumberValid = false;
    for (size_t length : { 1U, 20U, 21U, 40U, 41U, 54U, 55U, 64U })
    {
        memset(info.softwareVersion, 'W', length);
        info.softwareVersion[length - 1] = 'Z';
        info.softwareVersion[length]     = '\0';
        screen.Init(info);
        Frame wrapped        = Render(screen, status);
        const bool compact   = length > 40;
        const size_t columns = compact ? 27 : 20;
        const int x          = 10 + static_cast<int>((length - 1) % columns) * (compact ? 8 : 11);
        const int y          = 200 + static_cast<int>((length - 1) / columns) * (compact ? 13 : 20);
        if (compact)
            CheckGlyph(wrapped, x, y, 7, StatusFont::kCompact['Z' - 32]);
        else
            CheckGlyph(wrapped, x, y, 9, StatusFont::kBody['Z' - 32]);
    }

    // Every printable ASCII glyph is native-size and fits in its SN cell.
    for (int c = 32; c <= 126; ++c)
    {
        memset(info.serialNumber, c, 32);
        screen.Init(info);
        Frame ascii = Render(screen, status);
        CheckGlyph(ascii, 10, 140, 9, StatusFont::kBody[c - 32]);
        CheckGlyph(ascii, 175, 160, 9, StatusFont::kBody[c - 32]);
        for (int y = 140; y < 178; ++y)
            for (int x : { 19, 20, 184, 185 })
                assert(ascii[static_cast<size_t>(y * 480 + x * 2)] == 0xFF);
    }
    info.serialNumber[0] = '\x01';
    info.serialNumber[1] = '\0';
    screen.Init(info);
    CheckGlyph(Render(screen, status), 10, 140, 9, StatusFont::kBody['?' - 32]);

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
    assert(HasInk(unavailable, 100, 118) && HasInk(unavailable, 140, 158) && HasInk(unavailable, 200, 218));
    CheckGlyph(unavailable, 50, 100, 9, StatusFont::kBody['-' - 32]);
    CheckGlyph(unavailable, 165, 100, 9, StatusFont::kBody['-' - 32]);

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
