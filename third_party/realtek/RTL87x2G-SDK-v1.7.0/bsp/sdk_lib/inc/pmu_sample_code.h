/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _PMU_SAMPLE_CODE_H_
#define _PMU_SAMPLE_CODE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @defgroup PMU Performance Manager Unit
  * @brief
  * @{
  */

/*============================================================================*
 *                              Functions
*============================================================================*/
/** @defgroup PMU_Exported_Functions PMU Exported Functions
  * @{
  */

/**
 * @brief  Init pmu counter, then PMU (Performance Manager Unit) will start monitoring the D-Cache and I-Cache hit rate.
 *
 * <b>Example usage</b>
 * \code{.c}
 * int test(void)
 * {
 *     // Clean cache if needed.
 *     //SCB_CleanInvalidateDCache();
 *     //SCB_InvalidateICache();
 *
 *     // Start pmu counter for cache hit rate calculation.
 *     init_pmu_counter();
 *
 *     // Print pmu counter for cache hit rate statistics.
 *     pmu_print_counter();
 *
 *     return 0;
 * }
 * \endcode
 *
*/
void init_pmu_counter(void);

/**
 * @brief  Print pmu counter for D-Cache and I-Cache hit rate between init_pmu_counter() and pmu_print_counter().
 *
 * <b>Example usage</b>
 * \code{.c}
 * int test(void)
 * {
 *     // Clean cache if needed.
 *     //SCB_CleanInvalidateDCache();
 *     //SCB_InvalidateICache();
 *
 *     // Start pmu counter for cache hit rate calculation.
 *     init_pmu_counter();
 *
 *     // Print pmu counter for cache hit rate statistics.
 *     pmu_print_counter();
 *
 *     return 0;
 * }
 * \endcode
 *
*/
void pmu_print_counter(void);

/** End of group PMU_Exported_Functions
  * @}
  */

/** End of PMU
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif
