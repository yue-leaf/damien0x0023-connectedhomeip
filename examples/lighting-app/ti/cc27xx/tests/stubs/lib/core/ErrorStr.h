// Copyright 2026 Project CHIP Authors
// SPDX-License-Identifier: Apache-2.0
// Host-test stand-in; never included by the firmware target.
#pragma once
#include "CHIPError.h"
namespace chip {
inline const char * ErrorStr(CHIP_ERROR)
{
    return "host test error";
}
} // namespace chip
