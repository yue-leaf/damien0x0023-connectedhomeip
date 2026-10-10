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

#include "BoardRfSwitch.h"

#include <stdint.h>

#include <ti/drivers/GPIO.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/power/PowerCC27XX.h>
#include <ti_drivers_config.h>

namespace {

constexpr uint32_t kRfSwitchStartupTimeUs = 5;
Power_NotifyObj sPowerNotifyObj;

void DisableRfSwitch()
{
    GPIO_write(CONFIG_GPIO_RF_SWITCH_SELECT, 0);
    GPIO_write(CONFIG_GPIO_RF_SWITCH_POWER, 0);
}

void EnableOutput1()
{
    GPIO_write(CONFIG_GPIO_RF_SWITCH_POWER, 1);
    GPIO_write(CONFIG_GPIO_RF_SWITCH_SELECT, 1);
    ClockP_usleep(kRfSwitchStartupTimeUs);
}

int_fast16_t PowerNotify(uint_fast16_t eventType, uintptr_t eventArg, uintptr_t clientArg)
{
    (void) eventArg;
    (void) clientArg;

    if (eventType == PowerLPF3_AWAKE_STANDBY)
    {
        EnableOutput1();
    }
    else if (eventType == PowerLPF3_ENTERING_STANDBY || eventType == PowerLPF3_ENTERING_SHUTDOWN)
    {
        DisableRfSwitch();
    }

    return Power_NOTIFYDONE;
}

} // namespace

namespace BoardRfSwitch {

void Init()
{
    EnableOutput1();
    (void) Power_registerNotify(&sPowerNotifyObj,
                                PowerLPF3_ENTERING_STANDBY | PowerLPF3_AWAKE_STANDBY |
                                    PowerLPF3_ENTERING_SHUTDOWN,
                                PowerNotify, 0);
}

} // namespace BoardRfSwitch
