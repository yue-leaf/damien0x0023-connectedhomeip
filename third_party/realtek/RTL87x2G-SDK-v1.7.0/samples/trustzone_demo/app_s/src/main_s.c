/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Demo includes. */
#include "rtl876x.h"
#include "trace.h"
#include "secure_rom_cfg.h"
#include "tzm_config.h"
#include "mem_config.h"

/*-----------------------------------------------------------*/
void sau_region_init(void)
{
    boot_cfg.used_sau_entries = 8;

    boot_cfg.sau_region[1].RBAR = SAU_REGION1_BASE_ADDR;
    boot_cfg.sau_region[1].RLAR = SAU_REGION1_END_ADDR;
    boot_cfg.sau_region[1].NSC = 0;

    boot_cfg.sau_region[2].RBAR = SAU_REGION2_BASE_ADDR;
    boot_cfg.sau_region[2].RLAR = SAU_REGION2_END_ADDR;
    boot_cfg.sau_region[2].NSC = 0;

    boot_cfg.sau_region[3].RBAR = SAU_REGION3_BASE_ADDR;
    boot_cfg.sau_region[3].RLAR = SAU_REGION3_END_ADDR;
    boot_cfg.sau_region[3].NSC = 0;

    boot_cfg.sau_region[4].RBAR = SAU_REGION4_BASE_ADDR;
    boot_cfg.sau_region[4].RLAR = SAU_REGION4_END_ADDR;
    boot_cfg.sau_region[4].NSC = 0;

    boot_cfg.sau_region[5].RBAR = SAU_REGION5_BASE_ADDR;
    boot_cfg.sau_region[5].RLAR = SAU_REGION5_END_ADDR;
    boot_cfg.sau_region[5].NSC = 0;

    boot_cfg.sau_region[6].RBAR = SAU_REGION6_BASE_ADDR;
    boot_cfg.sau_region[6].RLAR = SAU_REGION6_END_ADDR;
    boot_cfg.sau_region[6].NSC = 0;

    boot_cfg.sau_region[7].RBAR = SAU_REGION7_BASE_ADDR;
    boot_cfg.sau_region[7].RLAR = SAU_REGION7_END_ADDR;
    boot_cfg.sau_region[7].NSC = 1;
}

/* Secure main. */
void secure_app_main(void)
{
    DBG_DIRECT("[S] Secure APP: main");

    /* Print SAU configuration so the developer can verify the S/NS memory
     * partition at runtime without a debugger.
     *
     * SAU->CTRL bits:
     *   ENABLE (bit 0) : 1 = SAU is enabled; 0 = all memory is Secure
     *   ALLNS  (bit 1) : 1 = all memory treated as NS (overrides regions)
     *
     * For each enabled region:
     *   RBAR[31:5] : base address (32-byte aligned)
     *   RLAR[31:5] : limit address (last byte of region, 32-byte aligned - 1)
     *   RLAR[1]    : NSC flag - 1 = Non-Secure Callable, 0 = Non-Secure
     */
    DBG_DIRECT("[S] SAU CTRL: ENABLE=%d ALLNS=%d",
               (int)((SAU->CTRL & SAU_CTRL_ENABLE_Msk) >> SAU_CTRL_ENABLE_Pos),
               (int)((SAU->CTRL & SAU_CTRL_ALLNS_Msk)  >> SAU_CTRL_ALLNS_Pos));

    for (uint32_t i = 0; i < 8U; i++)
    {
        SAU->RNR = i;
        if (SAU->RLAR & SAU_RLAR_ENABLE_Msk)
        {
            uint32_t base  =  SAU->RBAR & SAU_RBAR_BADDR_Msk;
            uint32_t limit = (SAU->RLAR & SAU_RLAR_LADDR_Msk) | 0x1FU; /* fill lower 5 bits */
            uint32_t nsc   = (SAU->RLAR & SAU_RLAR_NSC_Msk) >> SAU_RLAR_NSC_Pos;
            DBG_DIRECT("[S] SAU[%d]: 0x%08x - 0x%08x  %s",
                       (int)i, base, limit, nsc ? "NSC" : "NS");
        }
    }

    DBG_DIRECT("[S] Secure APP: ready");
}


/*-----------------------------------------------------------*/
