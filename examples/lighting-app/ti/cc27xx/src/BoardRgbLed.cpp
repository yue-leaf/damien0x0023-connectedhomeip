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

#include "BoardRgbLed.h"

#include <ti/drivers/GPIO.h>
#include <ti_drivers_config.h>

namespace {
void WriteCommonAnodeChannel(uint_least8_t gpio, uint8_t value)
{
    GPIO_write(gpio, value == 0 ? 1 : 0);
}
} // namespace

BoardRgbLed & BoardRgbLed::GetInstance()
{
    static BoardRgbLed instance;
    return instance;
}
void BoardRgbLed::Init()
{
    SetColor(0, 0, 0);
}

void BoardRgbLed::SetColor(uint8_t red, uint8_t green, uint8_t blue)
{
    WriteCommonAnodeChannel(CONFIG_GPIO_RGB_RED, red);
    WriteCommonAnodeChannel(CONFIG_GPIO_RGB_GREEN, green);
    WriteCommonAnodeChannel(CONFIG_GPIO_RGB_BLUE, blue);
}

void BoardRgbLed::SetLightOn(bool on)
{
    mLightOn = on;
    SetColor(on ? UINT8_MAX : 0, on ? UINT8_MAX : 0, on ? UINT8_MAX : 0);
}

void BoardRgbLed::SetIdentify(bool active)
{
    if (active)
    {
        SetColor(UINT8_MAX, 0, UINT8_MAX);
    }
    else
    {
        SetLightOn(mLightOn);
    }
}
