/*
 * Copyright (c) 2026 Project CHIP Authors
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */

#include "OnboardingScreen.h"

#include <assert.h>
#include <initializer_list>
#include <stdio.h>
#include <string.h>

namespace {
constexpr char kSampleQr[]     = "MT:Y3.13Y2N00KA0648G00";
constexpr char kSampleManual[] = "34970112332";

OnboardingCodes SampleCodes()
{
    OnboardingCodes codes;
    memcpy(codes.qrCode, kSampleQr, sizeof(kSampleQr));
    memcpy(codes.manualCode, kSampleManual, sizeof(kSampleManual));
    codes.valid = true;
    return codes;
}

void TestGeometryAndBounds(OnboardingScreen & screen)
{
    uint8_t qr[qrcodegen_BUFFER_LEN_FOR_VERSION(4)];
    uint8_t scratch[sizeof(qr)];
    assert(qrcodegen_encodeText(kSampleQr, scratch, qr, qrcodegen_Ecc_MEDIUM, 1, 4, qrcodegen_Mask_AUTO, true));
    const int size   = qrcodegen_getSize(qr);
    const int scale  = 176 / (size + 8);
    const int extent = (size + 8) * scale;
    const int left   = (240 - extent) / 2;
    const int top    = 40 + (176 - extent) / 2;
    uint8_t row[OnboardingScreen::kWidth * 2 + 2];
    for (uint16_t y = 0; y < OnboardingScreen::kHeight; ++y)
    {
        memset(row, 0xA5, sizeof(row));
        screen.RenderRow(y, row + 1);
        assert(row[0] == 0xA5 && row[sizeof(row) - 1] == 0xA5);
        for (int x = 0; x < 240; ++x)
        {
            assert(row[1 + x * 2] == row[2 + x * 2]);
            assert(row[1 + x * 2] == 0 || row[1 + x * 2] == 0xFF);
            if (x >= left && x < left + extent && y >= top && y < top + extent)
            {
                const int moduleX = (x - left) / scale - 4;
                const int moduleY = (y - top) / scale - 4;
                const bool black =
                    moduleX >= 0 && moduleX < size && moduleY >= 0 && moduleY < size && qrcodegen_getModule(qr, moduleX, moduleY);
                assert((row[1 + x * 2] == 0) == black);
            }
        }
    }
    // "3497-011-2332": centered 13-glyph line, dashes at glyphs 4 and 8.
    screen.RenderRow(228, row);
    constexpr int textLeft = (240 - (13 * 12 - 2)) / 2;
    for (int glyph : { 4, 8 })
    {
        for (int x = textLeft + glyph * 12; x < textLeft + glyph * 12 + 10; ++x)
        {
            assert(row[x * 2] == 0);
        }
    }
}

void WritePreview(const OnboardingScreen & screen, const char * path)
{
    FILE * output = fopen(path, "wb");
    assert(output != nullptr);
    fprintf(output, "P6\n240 240\n255\n");
    uint8_t row[480];
    for (uint16_t y = 0; y < OnboardingScreen::kHeight; ++y)
    {
        screen.RenderRow(y, row);
        for (size_t x = 0; x < OnboardingScreen::kWidth; ++x)
        {
            const uint8_t pixel[3] = { row[x * 2], row[x * 2], row[x * 2] };
            assert(fwrite(pixel, 1, sizeof(pixel), output) == sizeof(pixel));
        }
    }
    assert(fclose(output) == 0);
}
} // namespace

int main(int argc, char ** argv)
{
    static_assert(sizeof(OnboardingScreen) <= 256, "QR renderer must remain small");
    OnboardingScreen screen;
    assert(screen.Init(SampleCodes()));
    assert(screen.IsValid());
    TestGeometryAndBounds(screen);
    if (argc > 1)
    {
        WritePreview(screen, argv[1]);
    }
    OnboardingCodes codes = SampleCodes();
    codes.valid           = false;
    assert(!screen.Init(codes) && !screen.IsValid());
    codes               = SampleCodes();
    codes.manualCode[5] = 'X';
    assert(!screen.Init(codes));
    codes                = SampleCodes();
    codes.manualCode[11] = '1';
    assert(!screen.Init(codes));
    codes           = SampleCodes();
    codes.qrCode[0] = 'X';
    assert(!screen.Init(codes));
    codes = SampleCodes();
    memset(codes.qrCode, 'A', sizeof(codes.qrCode));
    assert(!screen.Init(codes));
    codes.qrCode[0]   = 'M';
    codes.qrCode[1]   = 'T';
    codes.qrCode[2]   = ':';
    codes.qrCode[128] = '\0';
    assert(!screen.Init(codes));
    codes           = SampleCodes();
    codes.qrCode[3] = '\0';
    assert(!screen.Init(codes));
    assert(!screen.Init(OnboardingCodes{}));
    if (argc > 2)
    {
        WritePreview(screen, argv[2]);
    }
    assert(screen.Init(SampleCodes()));
    puts("Onboarding screen tests passed: bounds, QR quiet zone, manual-code grouping, invalid inputs, recovery");
    return 0;
}
