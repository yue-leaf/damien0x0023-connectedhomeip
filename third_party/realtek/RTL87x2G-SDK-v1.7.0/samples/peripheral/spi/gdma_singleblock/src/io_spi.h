/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __IO_SPI_H
#define __IO_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "rtl_gdma.h"
#include "rtl_nvic.h"
#include "rtl_pinmux.h"
#include "rtl_rcc.h"
#include "rtl_spi.h"
#include "trace.h"

#define SPI_FLASH_PAGE_SIZE             256

#define SPI_FLASH_WRITE_ENABLE          0x06
#define SPI_FLASH_WRITE_DISABLE         0x04
#define SPI_FLASH_READ_STATUS_REG_1     0x05
#define SPI_FLASH_WRITE_STATUS_REG_1    0x01
#define SPI_FLASH_READ_STATUS_REG_2     0x35
#define SPI_FLASH_WRITE_STATUS_REG_2    0x31
#define SPI_FLASH_PAGE_PROGRAM          0x02
#define SPI_FLASH_SECTOR_ERASE          0x20
#define SPI_FLASH_BLOCK_ERASE_32K       0x52
#define SPI_FLASH_BLOCK_ERASE_64K       0xD8
#define SPI_FLASH_CHIP_ERASE            0xC7
#define SPI_FLASH_POWER_DOWN            0xB9
#define SPI_FLASH_READ_DATA             0x03
#define SPI_FLASH_FAST_READ             0x0B
#define SPI_FLASH_RELEASE_POWER_DOWN    0xAB
#define SPI_FLASH_DEVICE_ID             0xAB
#define SPI_FLASH_MANU_ID               0x90
#define SPI_FLASH_JEDEC_ID              0x9F
#define SPI_FLASH_ENABLE_RESET          0x66
#define SPI_FLASH_RESET                 0x99

#define EWIP_FLAG                       0x01  /* Erase/Write in progress (WIP) flag */

#define SPI_SCK_PIN                     P4_0
#define SPI_MOSI_PIN                    P4_1
#define SPI_MISO_PIN                    P4_2
#define SPI_CS_PIN                      P4_3
#define FLASH_SPI                       SPI0

#define GDMA_TRANSFER_SIZE 200

#define GDMA_TX_CHANNEL_NUM                 2
#define GDMA_TX_CHANNEL                     GDMA_Channel2
#define GDMA_TX_CHANNEL_IRQN                GDMA0_Channel2_IRQn
#define GDMA_TX_CHANNEL_HANDLER             GDMA0_Channel2_Handler

#define GDMA_RX_CHANNEL_NUM                 4
#define GDMA_RX_CHANNEL                     GDMA_Channel4
#define GDMA_RX_CHANNEL_IRQN                GDMA0_Channel4_IRQn
#define GDMA_RX_CHANNEL_HANDLER             GDMA0_Channel4_Handler

#define CONFIG_SPI_SW_SIM_CS                1

void spi_demo(void);
void driver_spi_init(void);
void driver_tx_gdma_init(void);
void driver_rx_gdma_init(void);
void spi_dma_demo(void);


#ifdef __cplusplus
}
#endif

#endif

