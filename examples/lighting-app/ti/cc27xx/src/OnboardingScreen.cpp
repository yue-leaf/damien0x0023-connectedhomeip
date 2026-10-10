/*
 * Copyright (c) 2026 Project CHIP Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "OnboardingScreen.h"
#include "MatterLogo.h"

#include <string.h>

namespace {
constexpr int kQuietZone   = 4;
constexpr int kQrAreaTop   = 40;
constexpr int kQrAreaSize  = 176;
constexpr int kFontScale   = 2;
constexpr int kGlyphWidth  = 5;
constexpr int kGlyphHeight = 7;
constexpr int kAdvance     = (kGlyphWidth + 1) * kFontScale;

// Five columns per glyph, least-significant bit at the top.
constexpr char kGlyphCharacters[]        = "0123456789-ACDFILNOR TVY";
constexpr uint8_t kGlyphs[][kGlyphWidth] = {
    { 0x3E, 0x51, 0x49, 0x45, 0x3E }, { 0x00, 0x42, 0x7F, 0x40, 0x00 }, { 0x42, 0x61, 0x51, 0x49, 0x46 },
    { 0x21, 0x41, 0x45, 0x4B, 0x31 }, { 0x18, 0x14, 0x12, 0x7F, 0x10 }, { 0x27, 0x45, 0x45, 0x45, 0x39 },
    { 0x3C, 0x4A, 0x49, 0x49, 0x30 }, { 0x01, 0x71, 0x09, 0x05, 0x03 }, { 0x36, 0x49, 0x49, 0x49, 0x36 },
    { 0x06, 0x49, 0x49, 0x29, 0x1E }, { 0x08, 0x08, 0x08, 0x08, 0x08 }, { 0x7E, 0x11, 0x11, 0x11, 0x7E },
    { 0x3E, 0x41, 0x41, 0x41, 0x22 }, { 0x7F, 0x41, 0x41, 0x22, 0x1C }, { 0x7F, 0x09, 0x09, 0x09, 0x01 },
    { 0x00, 0x41, 0x7F, 0x41, 0x00 }, { 0x7F, 0x40, 0x40, 0x40, 0x40 }, { 0x7F, 0x02, 0x04, 0x08, 0x7F },
    { 0x3E, 0x41, 0x41, 0x41, 0x3E }, { 0x7F, 0x09, 0x19, 0x29, 0x46 }, { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x01, 0x01, 0x7F, 0x01, 0x01 }, { 0x1F, 0x20, 0x40, 0x20, 0x1F }, { 0x07, 0x08, 0x70, 0x08, 0x07 },
};
static_assert(sizeof(kGlyphCharacters) - 1 == sizeof(kGlyphs) / sizeof(kGlyphs[0]), "Font table mismatch");

bool TextPixel(const char * text, int top, int x, int y)
{
    if (y < top || y >= top + kGlyphHeight * kFontScale)
    {
        return false;
    }
    const int length = static_cast<int>(strlen(text));
    const int width  = length * kAdvance - kFontScale;
    const int left   = (OnboardingScreen::kWidth - width) / 2;
    if (x < left || x >= left + width)
    {
        return false;
    }
    const int column = (x - left) % kAdvance / kFontScale;
    if (column >= kGlyphWidth)
    {
        return false;
    }
    const char * glyph = strchr(kGlyphCharacters, text[(x - left) / kAdvance]);
    return glyph != nullptr && (kGlyphs[glyph - kGlyphCharacters][column] & (1U << ((y - top) / kFontScale))) != 0;
}
} // namespace

bool OnboardingScreen::Init(const OnboardingCodes & codes)
{
    mValid = false;
    memset(mQrCode, 0, sizeof(mQrCode));
    memset(mManualCode, 0, sizeof(mManualCode));
    if (!codes.valid || memchr(codes.qrCode, '\0', sizeof(codes.qrCode)) == nullptr || strncmp(codes.qrCode, "MT:", 3) != 0 ||
        strlen(codes.qrCode) <= 3 || codes.manualCode[11] != '\0')
    {
        return false;
    }
    for (size_t i = 0; i < 11; ++i)
    {
        if (codes.manualCode[i] < '0' || codes.manualCode[i] > '9')
        {
            return false;
        }
    }

    uint8_t scratch[sizeof(mQrCode)];
    if (!qrcodegen_encodeText(codes.qrCode, scratch, mQrCode, qrcodegen_Ecc_MEDIUM, 1, kMaxQrVersion, qrcodegen_Mask_AUTO, true))
    {
        return false;
    }
    // Matter's standard 11-digit manual code is grouped 4-3-4, not the raw passcode.
    memcpy(mManualCode, codes.manualCode, 4);
    mManualCode[4] = '-';
    memcpy(mManualCode + 5, codes.manualCode + 4, 3);
    mManualCode[8] = '-';
    memcpy(mManualCode + 9, codes.manualCode + 7, 4);

    mQrSize          = qrcodegen_getSize(mQrCode);
    mModuleSize      = kQrAreaSize / (mQrSize + 2 * kQuietZone);
    const int extent = (mQrSize + 2 * kQuietZone) * mModuleSize;
    mQrX             = (kWidth - extent) / 2 + kQuietZone * mModuleSize;
    mQrY             = kQrAreaTop + (kQrAreaSize - extent) / 2 + kQuietZone * mModuleSize;
    mValid           = mModuleSize > 0;
    return mValid;
}

void OnboardingScreen::RenderRow(uint16_t y, uint8_t * rgb565) const
{
    for (int x = 0; x < kWidth; ++x)
    {
        bool black      = false;
        const int logoX = x - (kWidth - kMatterLogoWidth) / 2;
        const int logoY = static_cast<int>(y) - 3;
        if (logoX >= 0 && logoX < kMatterLogoWidth && logoY >= 0 && logoY < kMatterLogoHeight)
        {
            const size_t offset = static_cast<size_t>(logoY * (kMatterLogoWidth / 8) + logoX / 8);
            black               = (kMatterLogoBitmap[offset] & (0x80U >> (logoX % 8))) != 0;
        }
        if (mValid)
        {
            if (x >= mQrX && x < mQrX + mQrSize * mModuleSize && y >= mQrY && y < mQrY + mQrSize * mModuleSize)
            {
                black = qrcodegen_getModule(mQrCode, (x - mQrX) / mModuleSize, (y - mQrY) / mModuleSize);
            }
            black = black || TextPixel(mManualCode, 222, x, y);
        }
        else
        {
            black = black || TextPixel("FACTORY DATA", 108, x, y) || TextPixel("INVALID", 134, x, y);
        }
        rgb565[x * 2]     = black ? 0x00 : 0xFF;
        rgb565[x * 2 + 1] = black ? 0x00 : 0xFF;
    }
}
