/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include "rtl_gpio.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define FLASH_SPI                       SPI0

/* SPI pin define*/
#define SPI_SCK_PIN                     P4_0
#define SPI_MISO_PIN                    P4_1
#define SPI_MOSI_PIN                    P4_2
#define SPI_CS_PIN                      P4_3

#define SPI_CLK_MASTER                  SPI0_CLK_MASTER
#define SPI_MO_MASTER                   SPI0_MO_MASTER
#define SPI_MI_MASTER                   SPI0_MI_MASTER
#define SPI_CSN_0_MASTER                SPI0_CSN_0_MASTER

/* Private function prototypes -----------------------------------------------*/
#define SPI_MODE_FULLDUPLEX             0
#define SPI_MODE_EEPROM                 3

#define SPI_MODE                        SPI_MODE_FULLDUPLEX
#define SPI_CONFIG_GPIO_SIM_CS          0
/* Private functions ---------------------------------------------------------*/

void board_spi_init(void);
void driver_spi_init(void);
void spi_demo(void);
#if (SPI_CONFIG_GPIO_SIM_CS == 1)
void driver_gpio_init(void);
#endif


