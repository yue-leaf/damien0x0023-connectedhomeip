/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SECURE_ROM_CFG_H
#define SECURE_ROM_CFG_H

/* This file includes:
     1) Boot Config
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "boot_rom_common_cfg.h"

/*============================================================================*
*                              Part1: Boot Config
*============================================================================*/
typedef struct
{
    uint32_t RBAR;
    uint32_t RLAR;
    uint8_t  NSC;    /*1: is non-secure callable region, 0: non-secure region*/
} __attribute__((packed)) SAU_ATTRIBUTE_CONFIG;



typedef struct
{
    BOOT_ROM_COMMON_CFG common;

    uint32_t wdgAonBackup: 1;       /* 1 for release version, 0 for debug version. default = 0 */
    uint32_t wdgEnableInRom: 1;     /* 1 for release version, 0 for debug version. default = 0 */
    uint32_t wdgTimeoutMs: 23;      /* seconds, default = 4s */
    uint32_t wdgMode: 2;            /* 0: reset all,             1: reset all but aon
                                       2: interrupt km4 and kr0, 3: interrupt km4 */
    uint32_t wdgIP : 3;             /* default = 0 */
    uint32_t profiling_boot_flow: 1;/* default = 0 */
    uint32_t enableROMSAU : 1;      /* default = 1 */

    uint8_t total_sau_entries : 4;
    uint8_t used_sau_entries: 4;

    uint8_t dump_secure_mem_when_ns_hardfault: 1;   /* default: 0 */
    uint8_t enable_dump_all_mem: 1;                 /*0: no dump, 1: dump all ram*/
    uint8_t boot_patch_region_index: 3;
    uint8_t reserved0: 3;

    SAU_ATTRIBUTE_CONFIG sau_region[8];

    uint32_t reserved1[3];

    uint32_t secure_heap_addr;
    uint32_t secure_heap_size;

} __attribute__((packed)) BOOT_ROM_CFG;

extern BOOT_ROM_CFG boot_cfg;

#endif
