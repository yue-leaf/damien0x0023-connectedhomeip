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

#pragma once

#include <qrcodegen.h>
#include <stddef.h>
#include <stdint.h>

struct OnboardingCodes
{
    char qrCode[129]    = {};
    char manualCode[12] = {};
    bool valid          = false;
};

// Rasterize one RGB565 scanline at a time; no full-screen framebuffer is needed.
class OnboardingScreen
{
public:
    static constexpr uint16_t kWidth  = 240;
    static constexpr uint16_t kHeight = 240;

    bool Init(const OnboardingCodes & codes);
    void RenderRow(uint16_t y, uint8_t * rgb565) const;
    bool IsValid() const { return mValid; }

private:
    static constexpr int kMaxQrVersion                               = 4;
    uint8_t mQrCode[qrcodegen_BUFFER_LEN_FOR_VERSION(kMaxQrVersion)] = {};
    char mManualCode[14]                                             = {};
    int mQrSize                                                      = 0;
    int mModuleSize                                                  = 0;
    int mQrX                                                         = 0;
    int mQrY                                                         = 0;
    bool mValid                                                      = false;
};
