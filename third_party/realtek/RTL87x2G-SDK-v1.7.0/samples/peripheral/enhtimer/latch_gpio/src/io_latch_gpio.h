/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include "rtl_pinmux.h"
#include "rtl_gpio.h"
#include "rtl876x.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define INPUT_PIN                       P2_2
#define GPIO_PIN_INPUT                  GPIO_GetPin(INPUT_PIN)
#define GPIO_PORT_INPUT                 GPIO_GetPort(INPUT_PIN)

#define OUTPUT_PIN                      P2_4
#define GPIO_PIN_OUTPUT                 GPIO_GetPin(OUTPUT_PIN)
#define GPIO_PORT_OUTPUT                GPIO_GetPort(OUTPUT_PIN)

#define ENHTIMER_NUM                    ENH_TIM0
#define ENHTIMER_IRQn                   Enhanced_Timer0_IRQn

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialize TIM-Enhance peripheral.
  * @param  None.
  * @return None.
  */
void driver_enhance_timer_init(void);

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  None.
  * @return None.
  */
void board_gpio_init(void);

/**
  * @brief  Initialize GPIO peripheral.
  * @param  None.
  * @return None.
  */
void driver_gpio_init(void);

/**
  * @brief  Enhance timer handler.
  * @param  None.
  * @return None.
  */
void driver_enhance_timer_init(void);



