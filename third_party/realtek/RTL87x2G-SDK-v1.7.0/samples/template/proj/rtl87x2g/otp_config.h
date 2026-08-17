/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef OTP_CONFIG_H
#define OTP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        debug configuration
 *============================================================================*/
/** @brief just for debug */
#define SYSTEM_TRACE_ENABLE                        0

/**************************************************/
/** @brief config enable write hardfault record to flash example */
//config enable write hardfault record to flash
//if enable, must define 1; if disable, define 0 or not define
#define WRITE_HARDFAULT_RECORD_TO_FLASH_ENABLE     0
//if enable must define hardfault record begin and end flash addr, and record cfg depend on user flash usage
#define HARDFAULT_RECORD_BEG_ADDR                  0x040AD000  //change to user unused space
#define HARDFAULT_RECORD_END_ADDR                 (HARDFAULT_RECORD_BEG_ADDR + 2 * FMC_SEC_SECTION_LEN)
#define HARDFAULT_RECORD_CFG                      ( BIT_ENABLE_SAVE_HARDFAULT )
//add more here


/*============================================================================*
 *                        flash configuration
 *============================================================================*/
/** @brief for enable default bp level */
#define DEFAULT_FLASH_BP_LEVEL_ENABLE              0
//add more here


/*============================================================================*
 *                        os configuration
 *============================================================================*/
/** @brief *default cpu sleep en is 1, can enter wfi */
#define CPU_SLEEP_EN                                1

/** @brief *default os tick time is 10ms */
#define OS_TICK_TIME                              OS_TICK_10MS

/** @brief *Config ext_data_sram whether be replaced by general_ram_type, default heap_ext_data_sram_mask is 0 */
#define HEAP_EXT_DATA_SRAM_MASK_EN                  0

/*============================================================================*
 *                        platform configuration
 *============================================================================*/
/** @brief *default sw timer number is 32 */
#define TIMER_MAX_NUMBER                            32

/** @brief *default sw timer task stack size is 1KB */
#define TIMER_TASK_STACK_SIZE                       (256 * 4)

/** @brief *default use_ftl is 1 */
#define USE_FTL                                    1

/**
 * @brief FTL logic address mapping bit width, unit: 4 bits per step.
 *        Actual mapping bit width = value * 4 bits.
 *        Valid range: 3~7 (12~28 bit). Default value 3 -> 12-bit mapping.
 *        Note: this macro has no effect when FTL_POOL is defined.
 *
 * Max supported FTL physical pages (1 page = 4 KB, PAGE_element = 1024 cells/page):
 *   The mapping field encodes phy_addr_offset = (pageID * PAGE_element + cell_index) / 2,
 *   so the maximum representable offset is (1 << (value * 4)) - 1.
 *     max_pages = (1 << (value * 4)) * 2 / PAGE_element
 *     value 3 ->   8 pages =  32 KB  (default)
 *
 * RAM consumed by mapping table (ftl_mapping_table), only when ftl_use_mapping_table = 1:
 *   Let m = FTL physical size in KB (must be a multiple of 4).
 *     size = (511 * (m - 4) - 4) * 0.375  bytes  (fixed for value = 3)
 *     m = 16 KB (default) ->  (511 * 12 - 4) * 0.375 ~= 2.3 KB
 *     m = 32 KB (max)     ->  (511 * 28 - 4) * 0.375 ~= 5.4 KB
 */
#define FTL_LOGIC_ADDR_MAP_BIT_MAP                 3

/** @brief watch dog configuration */
#define ROM_WATCH_DOG_ENABLE                       0
/** @brief set wdg mode */
#define ROM_WATCH_DOG_MODE                         RESET_ALL
/** @brief set wdg timeout time, unit: ms */
#define ROM_WATCH_DOG_TIME_OUT_PERIOD              4000

/** @brief enable aon wdg which continue work in dlps state */
#define AON_WDG_ENABLE                             0
/** @brief set aon wdg mode */
#define AON_WATCH_DOG_MODE                         RESET_ALL_EXCEPT_AON
/** @brief set aon wdg timeout period, unit: ms */
#define AON_WDG_TIME_OUT_PERIOD                    10000

/** @brief *default log_ram_size is 3, log_buffer_size = (LOG_RAM_SIZE + 1) * 256 * 3.
            If want to disable buffer log to save ram, set log disable in config file*/
#define LOG_RAM_SIZE                               3

/**************************************************/


/*============================================================================*
 *                        upperstack configuration
 *============================================================================*/
//add more here


/*============================================================================*
 *                        app configuration
 *============================================================================*/
//add more here



#ifdef __cplusplus
}
#endif


/** @} */ /* End of group OTP_CONFIG */
#endif
