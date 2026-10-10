// Copyright 2026 Project CHIP Authors
// SPDX-License-Identifier: Apache-2.0
// Host-test stand-in; never included by the firmware target.
#pragma once
#include "FreeRTOS.h"
using QueueHandle_t = void *;
QueueHandle_t xQueueCreate(unsigned, unsigned size);
void vQueueDelete(QueueHandle_t);
BaseType_t xQueueOverwrite(QueueHandle_t, const void * data);
BaseType_t xQueueReceive(QueueHandle_t, void * data, TickType_t);
