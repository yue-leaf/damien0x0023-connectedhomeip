/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _OS_POWER_MANAGER_H_
#define _OS_POWER_MANAGER_H_
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "rtl876x.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define portNVIC_INT_CTRL_REG     ( *( ( volatile uint32_t * ) 0xe000ed04 ) )

#define UPDATE_TICK_COUNT()\
    if (portNVIC_INT_CTRL_REG & SCB_ICSR_PENDSTSET_Msk)\
    {\
        os_systick_handler();\
        portNVIC_INT_CTRL_REG = SCB_ICSR_PENDSTCLR_Msk;\
    }

#define US_TO_TICK_COUNT(us, residual)\
    cal_quotient_remainder(1000000 * (uint64_t)(os_sys_tick_clk_get() / os_sys_tick_rate_get()), os_sys_tick_clk_get(), us, &residual)

#define US_TO_SYSTICK(us, residual)\
    cal_quotient_remainder(1000000, os_sys_tick_clk_get(), us, &residual)

#define TICK_COUNT_TO_US(tick_count)\
    cal_quotient_remainder(os_sys_tick_clk_get(), 1000000 * (uint64_t)(os_sys_tick_clk_get() / os_sys_tick_rate_get()), tick_count, NULL)

#define SYSTICK_TO_US(systick)\
    cal_quotient_remainder(os_sys_tick_clk_get(), 1000000, systick, NULL)

#define SYSTICK_RELOAD_VALUE    (os_sys_tick_clk_get() / os_sys_tick_rate_get() - 1)

/** @addtogroup POWER_MANAGER_Exported_Types
* @{
*/
typedef enum
{
    PLATFORM_PM_EXCLUDED_TIMER,     /**< Timer exclude type for power manager. */
    PLATFORM_PM_EXCLUDED_TASK,      /**< Task exclude type for power manager. */
    PLATFORM_PM_EXCLUDED_TYPE_MAX,  /**< Max exclude type number for power manager. */
} PlatformExcludedHandleType;
/** @} */

typedef struct
{
    struct PlatformPMExcludedHandleQueueElem *pNext;
    void **handle;
} PlatformPMExcludedHandleQueueElem;

typedef union
{
    uint8_t value[1];
    struct
    {
        uint8_t os_pm_statistic:        1;
        uint8_t rsvd:                   7;
    };
} OSPMFeatureConfig;

typedef struct
{
    uint32_t last_sleep_clk;
    uint32_t last_sleep_systick;
    uint32_t tickcount_restore_remain_us;
} OSPMSystem;

extern void (*os_pm_init)(void);
extern bool (*os_sched_restore)(void);
extern void (*os_systick_handler)(void);
extern uint32_t (*os_sys_tick_rate_get)(void);
extern uint32_t (*os_sys_tick_clk_get)(void);
extern uint64_t (*os_sys_tick_increase)(uint32_t tick_increment);


extern uint32_t (*os_pm_next_timeout_value_get)(void);

/** @addtogroup POWER_MANAGER_Exported_Functions
* @{
*/

/**
 * @brief  Exclude SW timer or task from low power wake list, SW timer needs to be one-shot timer.
 * @param[in]  handle Handle of SW timer or task.
 * @param[in]  type Type of SW timer or task, refer to @ref PlatformExcludedHandleType.
 * @return Operation status. It returns false when pp_handle is NULL or there is not enough heap to malloc.
*/
extern bool (*os_register_pm_excluded_handle)(void **pp_handle, PlatformExcludedHandleType type);

/**
 * @brief  Cancel exclude SW timer or task from low power wake list.
 * @param[in]  handle Handle of timer or task.
 * @param[in]  type Type of timer or task, refer to @ref PlatformExcludedHandleType.
 * @return Operation status. It returns false when pp_handle is NULL or the pp_handle is not found.
*/
extern bool (*os_unregister_pm_excluded_handle)(void **pp_handle, PlatformExcludedHandleType type);
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _OS_POWER_MANAGER_H_ */
