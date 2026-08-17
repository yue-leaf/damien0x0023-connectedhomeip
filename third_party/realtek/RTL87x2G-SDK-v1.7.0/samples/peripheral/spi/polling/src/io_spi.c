/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "rtl_rcc.h"
#include "rtl_pinmux.h"
#include "trace.h"

#include "rtl_spi.h"
#include "io_spi.h"
#include "spi_flash.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_spi_init(void)
{
    Pad_Config(SPI_SCK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI_MOSI_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI_MISO_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI_CS_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);

    Pinmux_Deinit(SPI_SCK_PIN);
    Pinmux_Deinit(SPI_MOSI_PIN);
    Pinmux_Deinit(SPI_MISO_PIN);
    Pinmux_Deinit(SPI_CS_PIN);

    Pinmux_Config(SPI_SCK_PIN, SPI_CLK_MASTER);
    Pinmux_Config(SPI_MOSI_PIN, SPI_MO_MASTER);
    Pinmux_Config(SPI_MISO_PIN, SPI_MI_MASTER);
#if (SPI_CONFIG_GPIO_SIM_CS == 1)
    Pinmux_Config(SPI_CS_PIN, DWGPIO);
#else
    Pinmux_Config(SPI_CS_PIN, SPI_CSN_0_MASTER);
#endif
}

#if (SPI_CONFIG_GPIO_SIM_CS == 1)
void driver_gpio_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_GPIOB, APBPeriph_GPIOB_CLOCK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(SPI_CS_PIN);
    GPIO_InitStruct.GPIO_Mode   = GPIO_DIR_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(GPIO_GetPort(SPI_CS_PIN), &GPIO_InitStruct);
}
#endif

void driver_spi_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_SPI0, APBPeriph_SPI0_CLOCK, ENABLE);

    SPI_InitTypeDef  SPI_InitStruct;
    SPI_StructInit(&SPI_InitStruct);
#if (SPI_MODE_FULLDUPLEX == SPI_MODE)
    SPI_InitStruct.SPI_Direction   = SPI_Direction_FullDuplex;
    DBG_DIRECT("SPI is set to FullDulpex Mode!");
#elif (SPI_MODE_EEPROM == SPI_MODE)
    SPI_InitStruct.SPI_Direction   = SPI_Direction_EEPROM;
    /* SPI_Direction_EEPROM mode read data lenth. */
    SPI_InitStruct.SPI_NDF         = 1;
    DBG_DIRECT("SPI is set to EEPROM Mode!");
#endif
    SPI_InitStruct.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStruct.SPI_CPHA        = SPI_CPHA_2Edge;
    SPI_InitStruct.SPI_BaudRatePrescaler  = 100;
    SPI_InitStruct.SPI_RxThresholdLevel  = 1;
    SPI_InitStruct.SPI_FrameFormat = SPI_Frame_Motorola;
    SPI_Init(SPI0, &SPI_InitStruct);

    SPI_Cmd(SPI0, ENABLE);

}

/**
  * @brief  Demo code of operation about spi.
  * @param  No parameter.
  * @return void
*/
void spi_demo(void)
{
    uint8_t flash_id[10] = {0};
    uint8_t write_data[100];
    uint8_t read_data[105] = {0};
    for (uint16_t i = 0; i < 100; i++)
    {
        write_data[i] = (i + 10) & 0xFF;
    }

    uint8_t flash_id_type = 0;

    while (flash_id_type < 3)
    {
        spi_flash_read_id((Flash_ID_Type)flash_id_type, flash_id);
        for (uint8_t i = 0; i < flash_id[0]; i++)
        {
            DBG_DIRECT("flash id = %x", flash_id[1 + i]);
        }
        flash_id_type++;
        memset(flash_id, 0, sizeof(flash_id));
    }

    spi_flash_sector_erase(0x001000);
    DBG_DIRECT("spi_demo: spi_flash_sector_erase done");

    spi_flash_read(SPI_FLASH_FAST_READ, 0x001000, read_data, 105);
    for (uint8_t i = 0; i < 105; i++)
    {
        DBG_DIRECT("spi_demo: after erase read_data[%d] = 0x%x", i, read_data[i]);
    }

    spi_flash_page_write(0x001000, write_data, 100);

    spi_flash_read(SPI_FLASH_FAST_READ, 0x001000, read_data, 105);
    for (uint8_t i = 0; i < 105; i++)
    {
        DBG_DIRECT("spi_demo: after write read_data[%d] = 0x%x", i, read_data[i]);
    }
}


