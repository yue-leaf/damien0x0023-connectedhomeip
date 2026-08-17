/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef PSRAM_H
#define PSRAM_H

#include <stdbool.h>
#include <stdint.h>
#include "fmc_platform.h"

/** @defgroup PSRAM  PSRAM
  * @brief
  * @{
  */
/****************************************************************************************
 * PSRAM Enumeration
 ****************************************************************************************/
/** @defgroup PSRAM_Exported_Types  PSRAM Exported Types
  * @brief
  * @{
  */

/** @brief PSRAM low power mode. */
typedef enum
{
    PSRAM_LPM_STANDBY_MODE,
    PSRAM_LPM_HALF_SLEEP_MODE,
    PSRAM_LPM_DEEP_POWER_DOWN_MODE,
} PSRAM_LPM_TYPE;
/** End of PSRAM_Exported_Types
  * @}
  */

/****************************************************************************************
 * PSRAM Function
 ****************************************************************************************/
/** @defgroup PSRAM_Exported_Functions  PSRAM Exported Functions
  * @brief
  * @{
  */

/**
* @brief  PSRAM init
* @return PSRAM init result
* @retval true PSRAM init successfully
* @retval false PSRAM init fail
*/
bool psram_winbond_opi_init(void);


/** End of PSRAM_Exported_Functions
  * @}
  */

/**
 * \cond     private
 * \defgroup PSRAM_Private_Functions
 * \{
 */

/**
 * @brief           set psram low power mode
 * @param idx       specific psram
 * @param enable    true if enable
 * @return          false if psram enter power mode fail
 */
bool psram_winbond_opi_dp_ctrl(FLASH_NOR_IDX_TYPE idx, bool enable, PSRAM_LPM_TYPE lpm_mode);

/**
 * @brief           read psram config register
 * @param idx       specific psram
 * @param cfg0      config register0 value
 * @param cfg1      config register1 value
 * @return          false if read psram register fail
 */
bool psram_winbond_opi_read_cfg(FLASH_NOR_IDX_TYPE idx, uint16_t *cfg0, uint16_t *cfg1);

/**
*  End of PSRAM_Private_Functions
* \}
* \endcond
*/

/** End of PSRAM
 * @}
 */

#endif /* PSRAM_H */
