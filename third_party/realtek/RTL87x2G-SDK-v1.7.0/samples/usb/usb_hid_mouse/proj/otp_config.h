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
//add more here


/*============================================================================*
 *                        flash configuration
 *============================================================================*/
//add more here


/*============================================================================*
 *                        os configuration
 *============================================================================*/
//add more here

/*============================================================================*
 *                        platform configuration
 *============================================================================*/
//add more here


/**************************************************/


/*============================================================================*
 *                        upperstack configuration
 *============================================================================*/
//add more here


/*============================================================================*
 *                        app configuration
 *============================================================================*/
//add more here
/** @brief *Config ext_data_sram whether be replaced by general_ram_type, default heap_ext_data_sram_mask is 0 */
#define HEAP_EXT_DATA_SRAM_MASK_EN   1


#ifdef __cplusplus
}
#endif


/** @} */ /* End of group OTP_CONFIG */
#endif
