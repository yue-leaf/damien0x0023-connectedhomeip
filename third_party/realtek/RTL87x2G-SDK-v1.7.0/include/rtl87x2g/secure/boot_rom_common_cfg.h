/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ROM_BOOT_COMMON_CFG_H
#define ROM_BOOT_COMMON_CFG_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    LOG_CHANNEL_UART0,
    LOG_CHANNEL_UART1,
    LOG_CHANNEL_UART2,
} LogChannel_TypeDef;

typedef enum
{
    TIMESTAMP_HW_TIM4 = 1,
    TIMESTAMP_MAX = 2
} T_TIMESTAMP_TYPE;

typedef struct
{
    uint32_t RBAR;
    uint32_t RLAR;
    uint8_t Attr;
} __attribute__((packed)) MPU_ATTRIBUTE_CONFIG;



typedef struct
{
    uint32_t aon_reg_parsing_tag : 8;
    uint32_t logDisable : 1;                /* Disable all DBG_DIRECT and DBG_BUFFER log */
    uint32_t directlogDisable : 1;          /* Disable DBG_DIRECT log */
    uint32_t logPin : 6;                    /* default = P2_0 */
    uint32_t logChannel : 2;                /* LogChannel_TypeDef: default is LOG_CHANNEL_UART2 */
    uint32_t logBaudRate : 5;               /* UartBaudRate_TypeDef: default is BAUD_RATE_2000000 */
    uint32_t dump_info_before_reset : 1;    /* Dump sram before sw reset */
    uint32_t rsvd0: 1;
    uint32_t trustzone_enable: 1;
    uint32_t hardfault_print_buf_log : 1;   /* Print buffered log in hard fault handler */
    uint32_t resetWhenHardFault : 1;        /* default = 0 */
    uint32_t enableASSERT: 1;               /* 0 for release version, 1 for debug version */
    uint32_t icache_enable: 1;
    uint32_t dcache_enable: 1;
    uint32_t enableROMMPU : 1;              /* protect rom as Read-only region, default = 1 */

    uint32_t total_mpu_entries : 4;
    uint32_t used_mpu_entries : 4;
    uint32_t swd_clk_pin: 6;                /* default is P1_1 */
    uint32_t swd_io_pin: 6;                 /* default is P1_0 */
#if (SYS_TIMESTAMP_USE_HW_TIMER == 1)
    uint32_t timestamp_src : 2; /* 0: Use OS Tick, not sync, 1: Use HW Timer 4, 2: RSVD */
    uint32_t timestamp_div : 3;   /* HW Timer 4 divider when it is used as log timestamp */
    uint32_t rsvd1 : 7;
#else
    uint32_t rsvd1 : 12;
#endif

    MPU_ATTRIBUTE_CONFIG mpu_region[8];

} __attribute__((packed)) BOOT_ROM_COMMON_CFG;

#endif
