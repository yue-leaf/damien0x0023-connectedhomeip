/*
 * Copyright (c) 2026 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "StatusScreen.h"
#include "StatusFontData.h"

#include <stdio.h>
#include <string.h>

namespace {
constexpr uint16_t kBackground = 0xFFFF;
constexpr uint16_t kText       = 0x0000;
constexpr uint16_t kLabel      = 0x4208;
constexpr uint16_t kGreen      = 0x0388;
constexpr uint16_t kRed        = 0xB904;
constexpr uint16_t kOrange     = 0xB380;
constexpr uint16_t kIdentify   = 0x8810;
constexpr int kMargin          = 10;

enum class Font
{
    kBody,
    kCompact,
    kLarge
};

void Pixel(uint8_t * row, int x, uint16_t color)
{
    if (x >= 0 && x < StatusScreen::kWidth)
    {
        row[x * 2]     = static_cast<uint8_t>(color >> 8);
        row[x * 2 + 1] = static_cast<uint8_t>(color);
    }
}

void TextRow(uint8_t * row, uint16_t y, const char * text, int left, int top, uint16_t color, Font font = Font::kBody,
             size_t limit = 80)
{
    const int height  = font == Font::kLarge ? 28 : (font == Font::kBody ? 18 : 12);
    const int width   = font == Font::kLarge ? 16 : (font == Font::kBody ? 9 : 7);
    const int advance = font == Font::kCompact ? 8 : width + 2;
    if (y < top || y >= top + height)
        return;
    const int glyphY = y - top;
    for (size_t i = 0; i < limit && text[i] != '\0'; ++i)
    {
        const int x = left + static_cast<int>(i) * advance;
        if (x + width > StatusScreen::kWidth - kMargin)
            break;
        const unsigned char c = static_cast<unsigned char>(text[i]);
        const unsigned index  = (c >= 32 && c <= 126 ? c : static_cast<unsigned char>('?')) - 32U;
        uint16_t bits;
        if (font == Font::kLarge)
        {
            const char * glyph = strchr(StatusFont::kLargeCharacters, c);
            if (glyph == nullptr)
                continue;
            bits = StatusFont::kLarge[glyph - StatusFont::kLargeCharacters][glyphY];
        }
        else
        {
            bits = font == Font::kBody ? StatusFont::kBody[index][glyphY] : StatusFont::kCompact[index][glyphY];
        }
        for (int dx = 0; dx < width; ++dx)
            if ((bits & (1U << dx)) != 0)
                Pixel(row, x + dx, color);
    }
}

void WrappedRow(uint8_t * row, uint16_t y, const char * text, int top, size_t columns, Font font = Font::kBody)
{
    const int lineHeight = font == Font::kCompact ? 13 : 20;
    const size_t length  = strlen(text);
    for (size_t start = 0; start < length; start += columns)
        TextRow(row, y, text + start, kMargin, top + static_cast<int>(start / columns) * lineHeight, kText, font, columns);
}

template <size_t N>
bool ValidString(const char (&text)[N])
{
    return text[0] != '\0' && memchr(text, '\0', N) != nullptr;
}

static_assert(sizeof(StatusFont::kBody) + sizeof(StatusFont::kCompact) + sizeof(StatusFont::kLarge) <= 7000,
              "Status font Flash budget exceeded");
} // namespace

void StatusScreen::Init(const DeviceDisplayInfo & info)
{
    if (info.vendorIdValid)
        snprintf(mVendorId, sizeof(mVendorId), "0x%04X", static_cast<unsigned>(info.vendorId));
    else
        strcpy(mVendorId, "--");
    if (info.productIdValid)
        snprintf(mProductId, sizeof(mProductId), "0x%04X", static_cast<unsigned>(info.productId));
    else
        strcpy(mProductId, "--");
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

    TextRow(rgb565, y, "TI-Light", kMargin, 6, kText, Font::kLarge);
    if (status.identifying)
        TextRow(rgb565, y, "Identify", 166, 16, kIdentify, Font::kCompact);
    TextRow(rgb565, y, "Network", kMargin, 40, kLabel);
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
        network      = "Connected";
        networkColor = kGreen;
        break;
    case DisplayNetworkStatus::kDisconnected:
        network      = "Disconnected";
        networkColor = kRed;
        break;
    }
    TextRow(rgb565, y, network, 92, 40, networkColor);
    TextRow(rgb565, y, "Light", kMargin, 70, kLabel);
    TextRow(rgb565, y, status.lightKnown ? (status.lightOn ? "ON" : "OFF") : "Unknown", 92, status.lightKnown ? 64 : 70,
            status.lightKnown && status.lightOn ? kGreen : kText, status.lightKnown ? Font::kLarge : Font::kBody);
    if (y == 94)
        for (int x = kMargin; x < kWidth - kMargin; ++x)
            Pixel(rgb565, x, 0xDEFB);

    TextRow(rgb565, y, "VID", kMargin, 100, kLabel);
    TextRow(rgb565, y, mVendorId, 50, 100, kText);
    TextRow(rgb565, y, "PID", 125, 100, kLabel);
    TextRow(rgb565, y, mProductId, 165, 100, kText);
    TextRow(rgb565, y, "SN", kMargin, 122, kLabel);
    WrappedRow(rgb565, y, mSerialNumber, 140, 16);
    TextRow(rgb565, y, "SW Version", kMargin, 180, kLabel);
    // Normal versions use two body-font lines; the maximum 77-character value
    // uses three independently rasterized compact lines, never scaled glyphs.
    const bool compactVersion = strlen(mSoftwareVersion) > 40;
    WrappedRow(rgb565, y, mSoftwareVersion, 200, compactVersion ? 27 : 20, compactVersion ? Font::kCompact : Font::kBody);
}
