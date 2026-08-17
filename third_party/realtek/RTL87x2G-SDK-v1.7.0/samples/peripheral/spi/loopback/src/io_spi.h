/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Defines ------------------------------------------------------------------*/
/* SPI1 MASTER pin define*/
#define SPI_MASTER_SCK_PIN               P2_4
#define SPI_MASTER_MOSI_PIN              P2_5
#define SPI_MASTER_MISO_PIN              P2_6
#define SPI_MASTER_CS_PIN                P2_7

/* SPI0 SLAVE pin define*/
#define SPI_SLAVE_SCK_PIN                P3_0
#define SPI_SLAVE_MOSI_PIN               P3_1
#define SPI_SLAVE_MISO_PIN               P3_2
#define SPI_SLAVE_CS_PIN                 P3_3

/* SPI1 MASTER define*/
#define SPI_MASTER                      SPI1
#define SPI_CLK_MASTER                  SPI1_CLK_MASTER
#define SPI_MO_MASTER                   SPI1_MO_MASTER
#define SPI_MI_MASTER                   SPI1_MI_MASTER
#define SPI_SS_N_0_MASTER               SPI1_SS_N_0_MASTER
#define SPI_IRQn_MASTER                 SPI1_IRQn
#define SPI_Handler_MASTER              SPI1_Handler
#define APBPeriph_SPI_MASTER            APBPeriph_SPI1
#define APBPeriph_SPI_CLOCK_MASTER      APBPeriph_SPI1_CLOCK

/* SPI0 SALVE define*/
#define SPI_SLAVE                       SPI0_SLAVE
#define SPI_CLK_SLAVE                   SPI0_CLK_SLAVE
#define SPI_MO_SLAVE                    SPI0_SI_SLAVE
#define SPI_MI_SLAVE                    SPI0_SO_SLAVE
#define SPI_SS_N_0_SLAVE                SPI0_CSN_0_SLAVE
#define SPI_IRQn_SLAVE                  SPI_Slave_IRQn
#define SPI_Handler_SLAVE               SPI_Slave_Handler
#define APBPeriph_SPI_SLAVE             APBPeriph_SPI0_SLAVE
#define APBPeriph_SPI_CLOCK_SLAVE       APBPeriph_SPI0_SLAVE_CLOCK

#define SEND_LENGTH     20
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

void spi_demo(void);


