/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _TEST_MODE_H_
#define _TEST_MODE_H_

#include <stdbool.h>
#include "wdt.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup TEST_MODE_API  Test Mode
  * @brief Test mode APIs
  * @{
  */

/*============================================================================*
 *                      Functions
 *============================================================================*/
/** @defgroup TEST_MODE_Exported_Functions Test Mode Sets Exported Functions
  * @brief
  * @{
  */
/**
  * @brief  Check whether MCU should be single tone test mode or not.
  * @retval true if the MCU is in single tone test mode, false otherwise.
  */
extern bool (*is_single_tone_test_mode)(void);

/**
  * @brief  Switch to single tone test mode.
  */
extern void (*switch_into_single_tone_test_mode)(void);

/**
  * @brief  Reset single tone test mode.
  */
extern void (*reset_single_tone_test_mode)(void);

/**
  * @brief  Set HCI mode flag.
  * @param[in]  enable   true means enable, false means disable.
  */
extern void (*set_hci_mode_flag)(bool enable);

/**
  * @brief  Check whether MCU should be hci mode or not.
  * @retval true if the MCU is in HCI mode, false otherwise.
  */
bool check_hci_mode_flag(void);

/**
  * @brief  Switch to HCI mode.
  */
extern void (*switch_into_hci_mode)(void);


/** End of TEST_MODE_Exported_Functions
  * @}
  */

/** End of TEST_MODE_API
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* _TEST_MODE_H_ */
