// Copyright 2026 Project CHIP Authors
// SPDX-License-Identifier: Apache-2.0
// Host-test stand-in; never included by the firmware target.
#pragma once
#include "FreeRTOS.h"
using TaskHandle_t = void *;
BaseType_t xTaskCreate(void (*entry)(void *), const char *, unsigned, void * context, UBaseType_t, TaskHandle_t * task);
void vTaskDelay(TickType_t);
