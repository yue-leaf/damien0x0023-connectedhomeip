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

#include "DisplayManager.h"

#include "AppConfig.h"
#include <lib/core/ErrorStr.h>

namespace {
static_assert(sizeof(DisplayManager) <= 640, "LCD static storage exceeds the CC2755R10 budget");
constexpr uint32_t kDisplayTaskStackSize   = 2048;
constexpr UBaseType_t kDisplayTaskPriority = tskIDLE_PRIORITY + 1;
constexpr unsigned kMaxRenderAttempts      = 3;
constexpr TickType_t kRenderRetryDelay     = pdMS_TO_TICKS(250);
} // namespace

DisplayManager & DisplayManager::GetInstance()
{
    static DisplayManager instance;
    return instance;
}

CHIP_ERROR DisplayManager::Start(const OnboardingCodes & codes, bool commissioned, const DeviceDisplayInfo & info)
{
    if (mTask != nullptr)
    {
        return CHIP_NO_ERROR;
    }

    mQueue = xQueueCreate(1, sizeof(Snapshot));
    if (mQueue == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    // Consume startup inputs before publishing the task; retain no raw copies.
    if (!mOnboardingScreen.Init(codes) && codes.valid)
    {
        PLAT_LOG("LCD QR encoding failed; displaying factory-data error");
    }
    mStatusScreen.Init(info);
    mDesiredSnapshot.commissioned = commissioned;
    xQueueOverwrite(mQueue, &mDesiredSnapshot);

    if (xTaskCreate(TaskMain, "LCD", kDisplayTaskStackSize / sizeof(StackType_t), this, kDisplayTaskPriority, &mTask) != pdPASS)
    {
        vQueueDelete(mQueue);
        mQueue = nullptr;
        return CHIP_ERROR_NO_MEMORY;
    }
    return CHIP_NO_ERROR;
}

void DisplayManager::SetNetworkStatus(DisplayNetworkStatus network, bool commissioned)
{
    if (mQueue == nullptr)
    {
        return;
    }
    taskENTER_CRITICAL();
    mDesiredSnapshot.status.network = network;
    mDesiredSnapshot.commissioned   = commissioned;
    xQueueOverwrite(mQueue, &mDesiredSnapshot);
    taskEXIT_CRITICAL();
}

void DisplayManager::SetLightOn(bool lightOn)
{
    if (mQueue == nullptr)
    {
        return;
    }
    taskENTER_CRITICAL();
    mDesiredSnapshot.status.lightOn    = lightOn;
    mDesiredSnapshot.status.lightKnown = true;
    xQueueOverwrite(mQueue, &mDesiredSnapshot);
    taskEXIT_CRITICAL();
}

void DisplayManager::SetIdentify(bool identifying)
{
    if (mQueue == nullptr)
    {
        return;
    }
    taskENTER_CRITICAL();
    mDesiredSnapshot.status.identifying = identifying;
    xQueueOverwrite(mQueue, &mDesiredSnapshot);
    taskEXIT_CRITICAL();
}

void DisplayManager::ToggleBacklight()
{
    if (mQueue == nullptr)
    {
        return;
    }
    taskENTER_CRITICAL();
    mDesiredSnapshot.backlightEnabled = !mDesiredSnapshot.backlightEnabled;
    xQueueOverwrite(mQueue, &mDesiredSnapshot);
    taskEXIT_CRITICAL();
}

void DisplayManager::TaskMain(void * context)
{
    static_cast<DisplayManager *>(context)->Run();
}

void DisplayManager::Run()
{
    CHIP_ERROR error    = mDisplay.Init();
    mDisplayInitialized = (error == CHIP_NO_ERROR);
    if (error != CHIP_NO_ERROR)
    {
        PLAT_LOG("LCD initialization failed: %s", chip::ErrorStr(error));
    }

    Snapshot snapshot;
    unsigned renderAttempts = 0;
    bool retryPending       = false;
    while (true)
    {
        Snapshot update;
        const bool updated = xQueueReceive(mQueue, &update, retryPending ? kRenderRetryDelay : portMAX_DELAY) == pdTRUE;
        if (updated)
        {
            snapshot       = update;
            renderAttempts = 0;
        }
        if ((!updated && !retryPending) || !mDisplayInitialized)
        {
            continue;
        }

        const bool redraw = !mHasRendered || snapshot.commissioned != mRenderedSnapshot.commissioned ||
            (snapshot.commissioned && !(snapshot.status == mRenderedSnapshot.status));
        if (redraw)
        {
            error        = Render(snapshot);
            mHasRendered = (error == CHIP_NO_ERROR);
            ++renderAttempts;
            retryPending = !mHasRendered && renderAttempts < kMaxRenderAttempts;
            if (error != CHIP_NO_ERROR)
            {
                // Never illuminate a partially written pairing/status page.
                mDisplay.SetBacklight(false);
                PLAT_LOG("LCD refresh failed: %s", chip::ErrorStr(error));
            }
        }
        if (mHasRendered && (redraw || snapshot.backlightEnabled != mRenderedSnapshot.backlightEnabled))
        {
            mDisplay.SetBacklight(snapshot.backlightEnabled);
        }
        mRenderedSnapshot = snapshot;
    }
}

CHIP_ERROR DisplayManager::Render(const Snapshot & snapshot)
{
    // Reuse the already reserved LCD task stack, not a second static scanline.
    uint8_t rowBuffer[OnboardingScreen::kWidth * 2];
    static_assert(St7789Display::kWidth == OnboardingScreen::kWidth && St7789Display::kHeight == OnboardingScreen::kHeight,
                  "LCD geometry mismatch");
    static_assert(StatusScreen::kWidth == St7789Display::kWidth && StatusScreen::kHeight == St7789Display::kHeight,
                  "Status page geometry mismatch");
    for (uint16_t y = 0; y < OnboardingScreen::kHeight; ++y)
    {
        if (snapshot.commissioned)
        {
            mStatusScreen.RenderRow(y, rowBuffer, snapshot.status);
        }
        else
        {
            mOnboardingScreen.RenderRow(y, rowBuffer);
        }
        CHIP_ERROR error = mDisplay.DrawImage(0, y, OnboardingScreen::kWidth, 1, rowBuffer);
        if (error != CHIP_NO_ERROR)
        {
            return error;
        }
        if (y % 8 == 7)
        {
            vTaskDelay(1);
        }
    }
    return CHIP_NO_ERROR;
}
