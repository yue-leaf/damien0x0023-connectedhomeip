/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    int16_t AxisDirection;
    int16_t AxisCount;
} QDEC_Data_TypeDef;

/* Private define ------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return Void
  */
void board_qdec_init(void);

/**
  * @brief  Initialize Qdecoder peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_qdec_init(void);


