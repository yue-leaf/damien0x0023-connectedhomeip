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
constexpr uint32_t kDisplayTaskStackSize = 2048;
constexpr UBaseType_t kDisplayTaskPriority = tskIDLE_PRIORITY + 1;

constexpr uint16_t kColorBooting        = 0x0010;
constexpr uint16_t kColorCommissionable = 0xFD20;
constexpr uint16_t kColorCommissioning  = 0x07FF;
constexpr uint16_t kColorThreadAttached = 0x07E0;
constexpr uint16_t kColorThreadDetached = 0xF800;
constexpr uint16_t kColorLightOn        = 0xFFFF;
constexpr uint16_t kColorLightOff       = 0x0000;
constexpr uint16_t kColorIdentify       = 0xF81F;
constexpr uint16_t kColorError          = 0xF800;
} // namespace

DisplayManager & DisplayManager::GetInstance()
{
    static DisplayManager instance;
    return instance;
}

CHIP_ERROR DisplayManager::Start()
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

    if (xTaskCreate(TaskMain, "LCD", kDisplayTaskStackSize / sizeof(StackType_t), this, kDisplayTaskPriority, &mTask) != pdPASS)
    {
        vQueueDelete(mQueue);
        mQueue = nullptr;
        return CHIP_ERROR_NO_MEMORY;
    }
    return CHIP_NO_ERROR;
}

void DisplayManager::PostState(State state)
{
    if (mQueue == nullptr)
    {
        return;
    }
    taskENTER_CRITICAL();
    mDesiredState     = state;
    Snapshot snapshot = { mDesiredState, mDesiredBacklightEnabled };
    xQueueOverwrite(mQueue, &snapshot);
    taskEXIT_CRITICAL();
}

void DisplayManager::ToggleBacklight()
{
    if (mQueue == nullptr)
    {
        return;
    }
    taskENTER_CRITICAL();
    mDesiredBacklightEnabled = !mDesiredBacklightEnabled;
    Snapshot snapshot        = { mDesiredState, mDesiredBacklightEnabled };
    xQueueOverwrite(mQueue, &snapshot);
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
    if (mDisplayInitialized)
    {
        error = Render(State::kBooting);
        if (error == CHIP_NO_ERROR)
        {
            mDisplay.SetBacklight(true);
        }
    }
    if (error != CHIP_NO_ERROR)
    {
        PLAT_LOG("LCD initialization failed: %s", chip::ErrorStr(error));
    }

    Snapshot snapshot;
    while (true)
    {
        if (xQueueReceive(mQueue, &snapshot, portMAX_DELAY) != pdTRUE || !mDisplayInitialized)
        {
            continue;
        }

        if (snapshot.backlightEnabled != mBacklightEnabled)
        {
            mBacklightEnabled = snapshot.backlightEnabled;
            mDisplay.SetBacklight(mBacklightEnabled);
        }

        if (snapshot.state != mState)
        {
            mState = snapshot.state;
            error  = Render(mState);
            if (error != CHIP_NO_ERROR)
            {
                PLAT_LOG("LCD refresh failed: %s", chip::ErrorStr(error));
            }
        }
    }
}

CHIP_ERROR DisplayManager::Render(State state)
{
    uint16_t color = kColorError;
    switch (state)
    {
    case State::kBooting:
        color = kColorBooting;
        break;
    case State::kCommissionable:
        color = kColorCommissionable;
        break;
    case State::kCommissioning:
        color = kColorCommissioning;
        break;
    case State::kThreadAttached:
        color = kColorThreadAttached;
        break;
    case State::kThreadDetached:
        color = kColorThreadDetached;
        break;
    case State::kLightOn:
        color = kColorLightOn;
        break;
    case State::kLightOff:
        color = kColorLightOff;
        break;
    case State::kIdentify:
        color = kColorIdentify;
        break;
    case State::kError:
        color = kColorError;
        break;
    }
    return mDisplay.Fill(color);
}
