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
#define DEFAULT_FLASH_BP_LEVEL_ENABLE              0
//add more here


/*============================================================================*
 *                        os configuration
 *============================================================================*/
//add more here

/*============================================================================*
 *                        platform configuration
 *============================================================================*/
/** @brief default enable swd pinmux */
#define SWD_PINMUX_ENABLE                          1

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
