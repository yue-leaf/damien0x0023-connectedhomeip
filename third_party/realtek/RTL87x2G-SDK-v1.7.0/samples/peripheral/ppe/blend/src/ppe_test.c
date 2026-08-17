/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "rtl_pinmux.h"
#include "rtl_ppe.h"
#include "rtl_rcc.h"
#include "trace.h"
#include "string.h"

uint8_t __attribute__((aligned(4))) target_buf[16384];
uint8_t __attribute__((aligned(4))) sim_buf[16384];

void PPE_blend_test(void)
{
    uint32_t *source_buffer = (uint32_t *)sim_buf;
    uint32_t *target_buffer = (uint32_t *)target_buf;
    for (int i = 0; i < 4096; i++)
    {
        source_buffer[i] = 0x8080C84E;
    }
    memset(target_buf, 0, 16384);

    RCC_PeriphClockCmd(APBPeriph_PPE, APBPeriph_PPE_CLOCK, ENABLE);
    ppe_buffer_t target, source;
    memset(&target, 0, sizeof(ppe_buffer_t));
    memset(&source, 0, sizeof(ppe_buffer_t));
    source.address = (uint32_t) source_buffer;
    source.memory = source_buffer;
    source.color_key_en = DISABLE;
    source.color_key_value = 0;
    source.format = PPE_ABGR8888;
    source.width = 64;
    source.height = 64;
    source.stride = source.width;
    source.global_alpha_en = 0;
    source.global_alpha = 0xFF;

    target.address = (uint32_t) target_buffer;
    target.memory = target_buffer;
    target.color_key_en = DISABLE;
    target.color_key_value = 0;
    target.format = PPE_ABGR8888;
    target.width = 64;
    target.height = 64;
    target.stride = target.width;

    ppe_translate_t position = {.x = 0, .y = 0};
    PPE_ERR err = PPE_Blend(&source, &target, &position, PPE_SRC_OVER_MODE);
    DBG_DIRECT("PPE blend finish %d", err);
    for (int i = 0; i < 4096; i++)
    {
        if (target_buffer[i] != 0x40406427)
        {
            DBG_DIRECT("Data mismatch, expect 0x%8x, actual 0x%08x", 0x40406427, target_buffer[i]);
            DBG_DIRECT("PPE alpha blend error at offset %d", i);
            return;
        }
    }
    DBG_DIRECT("PPE blend test passed %d", err);
}