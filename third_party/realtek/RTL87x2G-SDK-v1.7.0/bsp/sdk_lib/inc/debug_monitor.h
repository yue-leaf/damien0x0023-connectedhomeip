/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef __DEBUG_MONITOR_H
#define __DEBUG_MONITOR_H

/*============================================================================*
 *                               Header Files
*============================================================================*/

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @defgroup DBG_MONITOR Debug Monitor
  * @brief
  * @{
  */

/*============================================================================*
 *                         Types
 *============================================================================*/
/** @defgroup DBG_MONITOR_Exported_Types Debug Monitor Exported Types
  * @brief
  * @{
  */

/* DWT Config*/
typedef enum
{
    BYTE  = 0,
    HALFWORD,
    WORD,
    WATCH_SIZE_MAX
} DWT_WATCH_SIZE;

typedef enum
{
    TRIGGER_ONLY = 0,
    DEBUG_EVENT,
    ACTION_MAX
} DWT_ACTION_TYPE;


typedef enum
{
    FUNCTION_DISABLED = 0,
    FUNCTION_INSTR_ADDR = 2,
    FUNCTION_INSTR_ADDR_LIM,
    FUNCTION_DADDR_RW,
    FUNCTION_DADDR_W,
    FUNCTION_DADDR_R,
    FUNCTION_DADDR_LIM,
    FUNCTION_DVAL_RW,
    FUNCTION_DVAL_W,
    FUNCTION_DVAL_R,
    FUNCTION_DVAL_LINK,
    FUNCTION_MAX
} DWT_FUNCTION_TYPE;

/** End of group DBG_MONITOR_Exported_Types
  * @}
  */

/*============================================================================*
 *                              Functions
*============================================================================*/
/** @defgroup DBG_MONITOR_Exported_Functions Debug Monitor Exported Functions
  * @{
  */

/**
 * @brief  Enable the debug monitor to track code execution and detect anomalies.
 */
void enable_debug_monitor(void);

/**
 * @brief  Configure settings for watchpoint 0 to monitor specified memory transactions.
 * @param[in]  watch_address Specifies the RAM address to monitor.
 * @param[in]  watch_size    Specifies the size of the RAM segment to monitor.
 * @param[in]  action_type   Specifies the type of action to trigger on watchpoint event.
 * @param[in]  read_write_func Specifies the function type for read/write access detection.
 */
void watch_point_0_setting(uint32_t watch_address,
                           DWT_WATCH_SIZE watch_size,
                           DWT_ACTION_TYPE action_type,
                           DWT_FUNCTION_TYPE read_write_func);

/**
 * @brief  Configure settings for watchpoint 1 to monitor specified memory transactions.
 * @param[in]  watch_address Specifies the RAM address to monitor.
 * @param[in]  watch_size    Specifies the size of the RAM segment to monitor.
 * @param[in]  action_type   Specifies the type of action to trigger on watchpoint event.
 * @param[in]  read_write_func Specifies the function type for read/write access detection.
 */
void watch_point_1_setting(uint32_t watch_address,
                           DWT_WATCH_SIZE watch_size,
                           DWT_ACTION_TYPE action_type,
                           DWT_FUNCTION_TYPE read_write_func);

/** End of group DBG_MONITOR_Exported_Functions
  * @}
  */

/** End of DBG_MONITOR
  * @}
  */

#endif  /* __DEBUG_MONITOR_H */
