/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define OUTPUT_PIN          P0_0
#define GPIO_PIN            GPIO_GetPin(OUTPUT_PIN)
#define GPIO_PORT           GPIO_GetPort(OUTPUT_PIN)
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialize TIMER peripheral.
  * @param  None.
  * @retval None.
  */
void driver_timer_init(void);

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_gpio_init(void);

/**
  * @brief  Initialize GPIO peripheral.
  * @param  None.
  * @retval None.
  */
void driver_gpio_init(void);


