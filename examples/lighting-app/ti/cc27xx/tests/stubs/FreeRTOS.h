// Copyright 2026 Project CHIP Authors
// SPDX-License-Identifier: Apache-2.0
// Host-test stand-in; never included by the firmware target.
#pragma once
#include <stdint.h>
using StackType_t                   = uint32_t;
using UBaseType_t                   = unsigned;
using BaseType_t                    = int;
using TickType_t                    = uint32_t;
constexpr int pdPASS                = 1;
constexpr int pdTRUE                = 1;
constexpr int pdFALSE               = 0;
constexpr unsigned tskIDLE_PRIORITY = 0;
constexpr TickType_t portMAX_DELAY  = UINT32_MAX;
#define taskENTER_CRITICAL() ((void) 0)
#define taskEXIT_CRITICAL() ((void) 0)
#define pdMS_TO_TICKS(ms) static_cast<TickType_t>(ms)
