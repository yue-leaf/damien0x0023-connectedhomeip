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

#include "DisplayManager.h"

#include <assert.h>
#include <functional>
#include <stdio.h>
#include <string.h>
#include <vector>

namespace {
std::vector<uint8_t> sQueue;
std::vector<std::function<void()>> sActions;
size_t sAction            = 0;
bool sPending             = false;
void (*sTask)(void *)     = nullptr;
void * sContext           = nullptr;
unsigned sRows            = 0;
unsigned sFills           = 0;
uint8_t sFrame[240 * 480] = {};
unsigned sYields          = 0;
bool sBacklight           = false;
bool sFailDraw            = false;
bool sSimulateTimeouts    = false;
unsigned sTimeouts        = 0;
unsigned sDrawAttempts    = 0;
unsigned sDrawFailures    = 0;
int sFailRow              = -1;
struct TestDone
{
};
} // namespace

QueueHandle_t xQueueCreate(unsigned, unsigned size)
{
    sQueue.resize(size);
    return &sQueue;
}
void vQueueDelete(QueueHandle_t) {}
BaseType_t xQueueOverwrite(QueueHandle_t, const void * data)
{
    memcpy(sQueue.data(), data, sQueue.size());
    sPending = true;
    return pdPASS;
}
BaseType_t xQueueReceive(QueueHandle_t, void * data, TickType_t wait)
{
    if (!sPending)
    {
        if (sSimulateTimeouts && wait != portMAX_DELAY)
        {
            assert(wait == pdMS_TO_TICKS(250));
            ++sTimeouts;
            return pdFALSE;
        }
        if (sAction == sActions.size())
        {
            throw TestDone{};
        }
        sActions[sAction++]();
    }
    assert(sPending);
    memcpy(data, sQueue.data(), sQueue.size());
    sPending = false;
    return pdTRUE;
}
BaseType_t xTaskCreate(void (*entry)(void *), const char *, unsigned, void * context, UBaseType_t, TaskHandle_t * task)
{
    sTask    = entry;
    sContext = context;
    *task    = &sQueue;
    return pdPASS;
}
void vTaskDelay(TickType_t)
{
    ++sYields;
}
extern "C" int cc27xxLog(const char *, ...)
{
    return 0;
}

CHIP_ERROR St7789Display::Init()
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR St7789Display::SetBacklight(bool enabled)
{
    sBacklight = enabled;
    return CHIP_NO_ERROR;
}
CHIP_ERROR St7789Display::Fill(uint16_t)
{
    ++sFills;
    return CHIP_NO_ERROR;
}
CHIP_ERROR St7789Display::DrawImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t * data)
{
    assert(x == 0 && y < 240 && width == 240 && height == 1 && data != nullptr);
    ++sDrawAttempts;
    if (sDrawFailures > 0)
    {
        --sDrawFailures;
        return CHIP_ERROR_NO_MEMORY;
    }
    if (sFailDraw || y == sFailRow)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    ++sRows;
    memcpy(sFrame + y * 480, data, 480);
    return CHIP_NO_ERROR;
}

int main(int argc, char ** argv)
{
    static_assert(sizeof(DisplayManager) <= 640, "LCD static storage exceeds the CC2755R10 budget");
    if (argc > 1 && strcmp(argv[1], "uncommissioned") == 0)
        printf("DisplayManager static storage: %zu bytes\n", sizeof(DisplayManager));
    auto & manager = DisplayManager::GetInstance();
    OnboardingCodes codes;
    strcpy(codes.qrCode, "MT:Y3.13Y2N00KA0648G00");
    strcpy(codes.manualCode, "34970112332");
    codes.valid              = true;
    const bool commissioned  = argc > 1 && strcmp(argv[1], "commissioned") == 0;
    const bool invalid       = argc > 1 && strcmp(argv[1], "invalid") == 0;
    const bool failure       = argc > 1 && strcmp(argv[1], "failure") == 0;
    const bool statusFailure = argc > 1 && strcmp(argv[1], "status-failure") == 0;
    const bool autoRetry     = argc > 1 && strcmp(argv[1], "auto-retry") == 0;
    const bool exhausted     = argc > 1 && strcmp(argv[1], "retry-exhausted") == 0;
    const bool partial       = argc > 1 && strcmp(argv[1], "partial-frame") == 0;
    const bool retryUpdate   = argc > 1 && strcmp(argv[1], "retry-update") == 0;
    if (commissioned)
    {
        codes = {};
    }
    if (invalid)
    {
        codes.valid = false;
    }
    DeviceDisplayInfo info;
    strcpy(info.serialNumber, "TI2755R10-12345678");
    strcpy(info.softwareVersion, "1.0.0");
    info.serialNumberValid = info.softwareVersionValid = true;
    StatusScreen expected;
    expected.Init(info);
    auto expectStatus = [&](DisplayNetworkStatus network, bool lightOn, bool identifying, bool lightKnown = true) {
        DisplayStatus status;
        status.network     = network;
        status.lightOn     = lightOn;
        status.lightKnown  = lightKnown;
        status.identifying = identifying;
        uint8_t row[480];
        for (uint16_t y = 0; y < 240; ++y)
        {
            expected.RenderRow(y, row, status);
            assert(memcmp(row, sFrame + y * 480, 480) == 0);
        }
    };
    sFailDraw         = failure || statusFailure || exhausted || retryUpdate;
    sSimulateTimeouts = autoRetry || exhausted;
    sDrawFailures     = autoRetry ? 2 : 0;
    assert(manager.Start(codes, commissioned || statusFailure, info) == CHIP_NO_ERROR);
    // Start consumes the caller's buffers synchronously; later mutations must
    // not alter either the initial page or a subsequent commissioning rollback.
    OnboardingCodes savedCodes = codes;
    memset(codes.qrCode, 'X', sizeof(codes.qrCode));
    memset(codes.manualCode, 'X', sizeof(codes.manualCode));
    memset(info.serialNumber, 'X', sizeof(info.serialNumber));
    memset(info.softwareVersion, 'X', sizeof(info.softwareVersion));
    if (autoRetry || exhausted)
    {
        sActions.push_back([&] {
            assert(sTimeouts == 2);
            if (autoRetry)
            {
                assert(sRows == 240 && sDrawAttempts == 242 && sBacklight);
                throw TestDone{};
            }
            assert(sRows == 0 && sDrawAttempts == 3 && !sBacklight);
            sFailDraw = false;
            manager.SetLightOn(true);
        });
        if (exhausted)
            sActions.push_back([&] {
                assert(sRows == 240 && sBacklight);
                throw TestDone{};
            });
    }
    else if (retryUpdate)
    {
        sActions.push_back([&] {
            assert(sRows == 0 && !sBacklight);
            sFailDraw = false;
            manager.SetLightOn(true);
            manager.SetIdentify(true);
            manager.SetNetworkStatus(DisplayNetworkStatus::kConnected, true);
        });
        sActions.push_back([&] {
            assert(sRows == 240 && sBacklight);
            expectStatus(DisplayNetworkStatus::kConnected, true, true);
            throw TestDone{};
        });
    }
    else if (partial)
    {
        sActions.push_back([&] {
            assert(sRows == 240 && sBacklight);
            sFailRow = 80;
            manager.SetNetworkStatus(DisplayNetworkStatus::kConnected, true);
        });
        sActions.push_back([&] {
            assert(sRows == 320 && !sBacklight);
            sFailRow = -1;
            manager.SetLightOn(false);
        });
        sActions.push_back([&] {
            assert(sRows == 560 && sBacklight);
            expectStatus(DisplayNetworkStatus::kConnected, false, false);
            throw TestDone{};
        });
    }
    else if (commissioned)
    {
        sActions.push_back([&] {
            assert(sRows == 240 && sFills == 0 && sBacklight);
            expectStatus(DisplayNetworkStatus::kUnknown, false, false, false);
            manager.SetNetworkStatus(DisplayNetworkStatus::kConnecting, true);
        });
        sActions.push_back([&] {
            assert(sRows == 480 && sFills == 0);
            expectStatus(DisplayNetworkStatus::kConnecting, false, false, false);
            manager.ToggleBacklight();
        });
        sActions.push_back([&] {
            assert(sRows == 480 && sFills == 0 && !sBacklight);
            throw TestDone{};
        });
    }
    else if (failure || statusFailure)
    {
        sActions.push_back([&] {
            assert(sRows == 0 && !sBacklight);
            sFailDraw = false;
            manager.SetNetworkStatus(DisplayNetworkStatus::kConnected, statusFailure);
        });
        sActions.push_back([&] {
            assert(sRows == 240 && sBacklight);
            if (statusFailure)
                expectStatus(DisplayNetworkStatus::kConnected, false, false, false);
            throw TestDone{};
        });
    }
    else
    {
        sActions.push_back([&] {
            assert(sRows == 240 && sFills == 0 && sBacklight && sYields == 30);
            manager.SetLightOn(true);
        });
        sActions.push_back([&] {
            assert(sRows == 240 && sFills == 0);
            manager.SetNetworkStatus(DisplayNetworkStatus::kConnected, false);
        });
        sActions.push_back([&] {
            assert(sRows == 240 && sFills == 0);
            manager.SetIdentify(true);
            manager.SetNetworkStatus(DisplayNetworkStatus::kConnected, true);
        });
        sActions.push_back([&] {
            assert(sRows == 480 && sFills == 0);
            expectStatus(DisplayNetworkStatus::kConnected, true, true);
            manager.SetLightOn(false);
        });
        sActions.push_back([&] {
            assert(sRows == 720);
            expectStatus(DisplayNetworkStatus::kConnected, false, true);
            manager.SetNetworkStatus(DisplayNetworkStatus::kDisconnected, true);
        });
        sActions.push_back([&] {
            assert(sRows == 960);
            expectStatus(DisplayNetworkStatus::kDisconnected, false, true);
            manager.SetIdentify(false);
        });
        sActions.push_back([&] {
            assert(sRows == 1200);
            expectStatus(DisplayNetworkStatus::kDisconnected, false, false);
            manager.SetIdentify(false);
        });
        sActions.push_back([&] {
            assert(sRows == 1200);
            manager.ToggleBacklight();
        });
        sActions.push_back([&] {
            assert(!sBacklight && sRows == 1200 && sFills == 0);
            manager.SetNetworkStatus(DisplayNetworkStatus::kUnknown, false);
        });
        sActions.push_back([&] {
            assert(sRows == 1440 && !sBacklight && sFills == 0);
            OnboardingScreen pairing;
            pairing.Init(savedCodes);
            uint8_t row[480];
            for (uint16_t y = 0; y < 240; ++y)
            {
                pairing.RenderRow(y, row);
                assert(memcmp(row, sFrame + y * 480, 480) == 0);
            }
            throw TestDone{};
        });
    }
    try
    {
        sTask(sContext);
    } catch (const TestDone &)
    {}
    assert(sAction == sActions.size());
    puts("Display manager tests passed");
    return 0;
}
