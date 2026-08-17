/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "rtl_pinmux.h"
#include "rtl_ppe.h"
#include "rtl_rcc.h"
#include "trace.h"
#include "ppe_simulation.h"
#include "pic_source.h"
#include "string.h"

uint8_t __attribute__((aligned(4))) target_buf[16384];
uint8_t __attribute__((aligned(4))) sim_buf[16384];

void PPE_scale_test(void)
{
    memset(target_buf, 0, 16384);
    memset(sim_buf, 0, 16384);

    ppe_buffer_t target, source, simulation;
    memset(&target, 0, sizeof(ppe_buffer_t));
    memset(&source, 0, sizeof(ppe_buffer_t));
    memset(&simulation, 0, sizeof(ppe_buffer_t));
    source.address = (uint32_t)pic_data;
    source.memory = (uint32_t *)pic_data;
    source.color_key_en = DISABLE;
    source.color_key_value = 0;
    source.format = PPE_RGB888;
    source.width = 25;
    source.height = 25;
    source.stride = source.width;

    target.address = (uint32_t) target_buf;
    target.memory = (uint32_t *) target_buf;
    target.color_key_en = DISABLE;
    target.color_key_value = 0;
    target.format = PPE_RGB888;
    target.stride = target.width;

    simulation.address = (uint32_t) sim_buf;
    simulation.memory = (uint32_t *) sim_buf;
    simulation.color_key_en = DISABLE;
    simulation.color_key_value = 0;
    simulation.format = PPE_RGB888;

    float horizontal_ratio = 2.5;
    float vertical_ratio = 0.8;

    PPE_ERR err = PPE_Scale(&source, &target, horizontal_ratio, vertical_ratio);
    PPE_Scale_Simulation(&source, &simulation, horizontal_ratio, vertical_ratio);
    DBG_DIRECT("PPE scale finish %d", err);
    for (int i = 0; i < target.width * target.height * 3; i++)
    {
        if (target_buf[i] != sim_buf[i])
        {
            DBG_DIRECT("Data mismatch at offset %d, expect 0x%8x, actual 0x%08x", i, 0x40406427,
                       target_buf[i]);
            return;
        }
    }
    DBG_DIRECT("PPE scale test passed %d", err);
}