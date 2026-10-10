/*
 * Copyright (c) 2026 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "StatusScreen.h"

#include <stdio.h>
#include <string.h>

namespace {
constexpr uint16_t kBackground = 0xFFFF;
constexpr uint16_t kText       = 0x1082;
constexpr uint16_t kLabel      = 0x632C;
constexpr uint16_t kGreen      = 0x0388;
constexpr uint16_t kRed        = 0xB904;
constexpr uint16_t kOrange     = 0xB380;
constexpr uint16_t kIdentify   = 0x8810;
constexpr int kMargin          = 10;
constexpr int kTextColumns     = 31;

// Printable ASCII, five columns per glyph with the top pixel in bit zero.
constexpr uint8_t kFont[][5] = {
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0x5F, 0, 0 },
    { 0, 7, 0, 7, 0 },
    { 0x14, 0x7F, 0x14, 0x7F, 0x14 },
    { 0x24, 0x2A, 0x7F, 0x2A, 0x12 },
    { 0x23, 0x13, 8, 0x64, 0x62 },
    { 0x36, 0x49, 0x55, 0x22, 0x50 },
    { 0, 5, 3, 0, 0 },
    { 0, 0x1C, 0x22, 0x41, 0 },
    { 0, 0x41, 0x22, 0x1C, 0 },
    { 0x14, 8, 0x3E, 8, 0x14 },
    { 8, 8, 0x3E, 8, 8 },
    { 0, 0x50, 0x30, 0, 0 },
    { 8, 8, 8, 8, 8 },
    { 0, 0x60, 0x60, 0, 0 },
    { 0x20, 0x10, 8, 4, 2 },
    { 0x3E, 0x51, 0x49, 0x45, 0x3E },
    { 0, 0x42, 0x7F, 0x40, 0 },
    { 0x42, 0x61, 0x51, 0x49, 0x46 },
    { 0x21, 0x41, 0x45, 0x4B, 0x31 },
    { 0x18, 0x14, 0x12, 0x7F, 0x10 },
    { 0x27, 0x45, 0x45, 0x45, 0x39 },
    { 0x3C, 0x4A, 0x49, 0x49, 0x30 },
    { 1, 0x71, 9, 5, 3 },
    { 0x36, 0x49, 0x49, 0x49, 0x36 },
    { 6, 0x49, 0x49, 0x29, 0x1E },
    { 0, 0x36, 0x36, 0, 0 },
    { 0, 0x56, 0x36, 0, 0 },
    { 8, 0x14, 0x22, 0x41, 0 },
    { 0x14, 0x14, 0x14, 0x14, 0x14 },
    { 0, 0x41, 0x22, 0x14, 8 },
    { 2, 1, 0x51, 9, 6 },
    { 0x32, 0x49, 0x79, 0x41, 0x3E },
    { 0x7E, 0x11, 0x11, 0x11, 0x7E },
    { 0x7F, 0x49, 0x49, 0x49, 0x36 },
    { 0x3E, 0x41, 0x41, 0x41, 0x22 },
    { 0x7F, 0x41, 0x41, 0x22, 0x1C },
    { 0x7F, 0x49, 0x49, 0x49, 0x41 },
    { 0x7F, 9, 9, 9, 1 },
    { 0x3E, 0x41, 0x49, 0x49, 0x7A },
    { 0x7F, 8, 8, 8, 0x7F },
    { 0, 0x41, 0x7F, 0x41, 0 },
    { 0x20, 0x40, 0x41, 0x3F, 1 },
    { 0x7F, 8, 0x14, 0x22, 0x41 },
    { 0x7F, 0x40, 0x40, 0x40, 0x40 },
    { 0x7F, 2, 0x0C, 2, 0x7F },
    { 0x7F, 4, 8, 0x10, 0x7F },
    { 0x3E, 0x41, 0x41, 0x41, 0x3E },
    { 0x7F, 9, 9, 9, 6 },
    { 0x3E, 0x41, 0x51, 0x21, 0x5E },
    { 0x7F, 9, 0x19, 0x29, 0x46 },
    { 0x46, 0x49, 0x49, 0x49, 0x31 },
    { 1, 1, 0x7F, 1, 1 },
    { 0x3F, 0x40, 0x40, 0x40, 0x3F },
    { 0x1F, 0x20, 0x40, 0x20, 0x1F },
    { 0x3F, 0x40, 0x38, 0x40, 0x3F },
    { 0x63, 0x14, 8, 0x14, 0x63 },
    { 7, 8, 0x70, 8, 7 },
    { 0x61, 0x51, 0x49, 0x45, 0x43 },
    { 0, 0x7F, 0x41, 0x41, 0 },
    { 2, 4, 8, 0x10, 0x20 },
    { 0, 0x41, 0x41, 0x7F, 0 },
    { 4, 2, 1, 2, 4 },
    { 0x40, 0x40, 0x40, 0x40, 0x40 },
    { 0, 1, 2, 4, 0 },
    { 0x20, 0x54, 0x54, 0x54, 0x78 },
    { 0x7F, 0x48, 0x44, 0x44, 0x38 },
    { 0x38, 0x44, 0x44, 0x44, 0x20 },
    { 0x38, 0x44, 0x44, 0x48, 0x7F },
    { 0x38, 0x54, 0x54, 0x54, 0x18 },
    { 8, 0x7E, 9, 1, 2 },
    { 0x0C, 0x52, 0x52, 0x52, 0x3E },
    { 0x7F, 8, 4, 4, 0x78 },
    { 0, 0x44, 0x7D, 0x40, 0 },
    { 0x20, 0x40, 0x44, 0x3D, 0 },
    { 0x7F, 0x10, 0x28, 0x44, 0 },
    { 0, 0x41, 0x7F, 0x40, 0 },
    { 0x7C, 4, 0x18, 4, 0x78 },
    { 0x7C, 8, 4, 4, 0x78 },
    { 0x38, 0x44, 0x44, 0x44, 0x38 },
    { 0x7C, 0x14, 0x14, 0x14, 8 },
    { 8, 0x14, 0x14, 0x18, 0x7C },
    { 0x7C, 8, 4, 4, 8 },
    { 0x48, 0x54, 0x54, 0x54, 0x20 },
    { 4, 0x3F, 0x44, 0x40, 0x20 },
    { 0x3C, 0x40, 0x40, 0x20, 0x7C },
    { 0x1C, 0x20, 0x40, 0x20, 0x1C },
    { 0x3C, 0x40, 0x30, 0x40, 0x3C },
    { 0x44, 0x28, 0x10, 0x28, 0x44 },
    { 0x0C, 0x50, 0x50, 0x50, 0x3C },
    { 0x44, 0x64, 0x54, 0x4C, 0x44 },
    { 0, 8, 0x36, 0x41, 0 },
    { 0, 0, 0x7F, 0, 0 },
    { 0, 0x41, 0x36, 8, 0 },
    { 8, 4, 8, 0x10, 8 },
};
static_assert(sizeof(kFont) / sizeof(kFont[0]) == 95, "Printable ASCII font is incomplete");

void Pixel(uint8_t * row, int x, uint16_t color)
{
    if (x >= 0 && x < StatusScreen::kWidth)
    {
        row[x * 2]     = static_cast<uint8_t>(color >> 8);
        row[x * 2 + 1] = static_cast<uint8_t>(color);
    }
}

void TextRow(uint8_t * row, uint16_t y, const char * text, int left, int top, int width, int height, uint16_t color,
             size_t limit = 80)
{
    if (y < top || y >= top + height)
    {
        return;
    }
    const int glyphY = (y - top) * 7 / height;
    for (size_t i = 0; i < limit && text[i] != '\0'; ++i)
    {
        const unsigned char c = static_cast<unsigned char>(text[i]);
        const auto & glyph    = kFont[(c >= 32 && c <= 126 ? c : static_cast<unsigned char>('?')) - 32];
        for (int dx = 0; dx < width; ++dx)
        {
            if ((glyph[dx * 5 / width] & (1U << glyphY)) != 0)
            {
                Pixel(row, left + static_cast<int>(i) * (width + 1) + dx, color);
            }
        }
    }
}

void WrappedRow(uint8_t * row, uint16_t y, const char * text, int top, int height)
{
    const size_t length = strlen(text);
    for (size_t start = 0; start < length; start += kTextColumns)
    {
        TextRow(row, y, text + start, kMargin, top + static_cast<int>(start / kTextColumns) * (height + 2), 6, height, kText,
                kTextColumns);
    }
}

template <size_t N>
bool ValidString(const char (&text)[N])
{
    return text[0] != '\0' && memchr(text, '\0', N) != nullptr;
}
} // namespace

void StatusScreen::Init(const DeviceDisplayInfo & info)
{
    if (info.vendorIdValid)
        snprintf(mVendorId, sizeof(mVendorId), "0x%04X", static_cast<unsigned>(info.vendorId));
    else
        strcpy(mVendorId, "Unavailable");
    if (info.productIdValid)
        snprintf(mProductId, sizeof(mProductId), "0x%04X", static_cast<unsigned>(info.productId));
    else
        strcpy(mProductId, "Unavailable");
    strcpy(mSerialNumber, info.serialNumberValid && ValidString(info.serialNumber) ? info.serialNumber : "Unavailable");
    const char * version = info.softwareVersionValid && ValidString(info.softwareVersion) ? info.softwareVersion : "Unavailable";
    if (info.softwareVersionNumberValid)
        snprintf(mSoftwareVersion, sizeof(mSoftwareVersion), "%s (%lu)", version,
                 static_cast<unsigned long>(info.softwareVersionNumber));
    else
        snprintf(mSoftwareVersion, sizeof(mSoftwareVersion), "%s", version);
}

void StatusScreen::RenderRow(uint16_t y, uint8_t * rgb565, const DisplayStatus & status) const
{
    for (int x = 0; x < kWidth; ++x)
        Pixel(rgb565, x, kBackground);

    TextRow(rgb565, y, "TI-Light", kMargin, 10, 12, 21, kText);
    if (status.identifying)
        TextRow(rgb565, y, "Identifying", kMargin, 33, 6, 10, kIdentify);
    TextRow(rgb565, y, "Network", kMargin, 51, 6, 12, kLabel);
    const char * network  = "Unknown";
    uint16_t networkColor = kLabel;
    switch (status.network)
    {
    case DisplayNetworkStatus::kUnknown:
        break;
    case DisplayNetworkStatus::kConnecting:
        network      = "Connecting";
        networkColor = kOrange;
        break;
    case DisplayNetworkStatus::kConnected:
        network      = "Thread Connected";
        networkColor = kGreen;
        break;
    case DisplayNetworkStatus::kDisconnected:
        network      = "Disconnected";
        networkColor = kRed;
        break;
    }
    TextRow(rgb565, y, network, 80, 50, 7, 14, networkColor);
    TextRow(rgb565, y, "Light", kMargin, 82, 6, 12, kLabel);
    TextRow(rgb565, y, status.lightKnown ? (status.lightOn ? "ON" : "OFF") : "Unknown", 80, 75, status.lightKnown ? 12 : 7, 21,
            status.lightKnown && status.lightOn ? kGreen : kText);
    if (y == 106)
        for (int x = kMargin; x < kWidth - kMargin; ++x)
            Pixel(rgb565, x, 0xDEFB);

    TextRow(rgb565, y, "VID", kMargin, 117, 6, 12, kLabel);
    TextRow(rgb565, y, mVendorId, 38, 117, 6, 12, kText);
    TextRow(rgb565, y, "PID", 125, 117, 6, 12, kLabel);
    TextRow(rgb565, y, mProductId, 153, 117, 6, 12, kText);
    TextRow(rgb565, y, "SN", kMargin, 139, 6, 12, kLabel);
    WrappedRow(rgb565, y, mSerialNumber, 157, 12);
    TextRow(rgb565, y, "SW Version", kMargin, 191, 6, 12, kLabel);
    // Keep the longest Matter version string plus numeric version on-screen.
    WrappedRow(rgb565, y, mSoftwareVersion, 209, strlen(mSoftwareVersion) > 2 * kTextColumns ? 7 : 12);
}
