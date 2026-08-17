/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef SD_TEST_H
#define SD_TEST_H

/* Includes ------------------------------------------------------------------*/
#include "sdcard.h"
#include "mmc.h"

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialize SDCard. write data then read back data to compare
  *         whether the data is correct.
  * @param  None.
  * @retval None.
  */
void SD_Test(void);

#endif /* SD_TEST_H */



