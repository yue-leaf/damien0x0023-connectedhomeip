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

#include <lib/core/CHIPError.h>

#include <stddef.h>
#include <stdint.h>
#include <ti/drivers/SPI.h>

class St7789Display
{
public:
    static constexpr uint16_t kWidth  = 240;
    static constexpr uint16_t kHeight = 240;

    CHIP_ERROR Init();
    CHIP_ERROR SetBacklight(bool enabled);
    CHIP_ERROR Fill(uint16_t rgb565);
    CHIP_ERROR FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t rgb565);
    CHIP_ERROR DrawImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t * data);

private:
    bool Transfer(const uint8_t * data, size_t count);
    CHIP_ERROR WriteCommand(uint8_t command);
    CHIP_ERROR WriteData(const uint8_t * data, size_t count);
    CHIP_ERROR WriteCommandData(uint8_t command, const uint8_t * data, size_t count);
    CHIP_ERROR SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    SPI_Handle mSpi = nullptr;
    uint8_t mPixelBuffer[256];
};
