/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include "sd_test.h"
#include "os_sync.h"
#include "os_mem.h"
#include <string.h>

/* Private define ------------------------------------------------------------*/
#define Init            Sd_Init
#define Read            Sd_Read
#define Write           Sd_Write
#define GetBlockCnt     Sd_GetBlockCnt
#define SD_Demo       SDHC0

#define START_BLOCK     (113)

#define BLOCK_CNT       (61)
#define BUF_BYTES       (512 * BLOCK_CNT)

static const SdEmmcInitParm_t Parm =
{
    .CardType = CARDTYPE_SD,
    .DataWidth = DATAWIDTH_4BIT,
    .ClkOutFreq_kHz = 50 * 1000,
};


/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initialize SDCard. write data then read back data to compare
  *         whether the data is correct.
  * @param  None.
  * @retval None.
  */
void SD_Test(void)
{
    SdEmmcRes_t Res = Init(SD_Demo, &Parm);
    DBG_DIRECT("Init: %d", Res);

    DBG_DIRECT("GetBlockCnt: %d", GetBlockCnt(SD_Demo));

    /* Notes: sd dma can only access RAM_TYPE_EXT_DATA_SRAM */
    uint8_t *pBuf = os_mem_alloc(RAM_TYPE_EXT_DATA_SRAM, BUF_BYTES);
    DBG_DIRECT("pBuf: %p", pBuf);

    memset(pBuf, 0xff, BUF_BYTES / 2);
    memset(pBuf + BUF_BYTES / 2, 0, BUF_BYTES / 2);
    Res = Write(SD_Demo, START_BLOCK, BLOCK_CNT, pBuf);
    DBG_DIRECT("Write: %d", Res);

    memset(pBuf, 0x77, BUF_BYTES);
    Res = Read(SD_Demo, START_BLOCK, BLOCK_CNT, pBuf);
    DBG_DIRECT("Read: %d", Res);

    /* Print all read data */
    for (size_t i = 0; i < BUF_BYTES; i += 8)
    {
        DBG_DIRECT("Block %d: 0x%04x -> 0x%02x - 0x%02x - 0x%02x - 0x%02x --- 0x%02x - 0x%02x - 0x%02x - 0x%02x",
                   i / 512, i % 512,
                   pBuf[i], pBuf[i + 1], pBuf[i + 2], pBuf[i + 3],
                   pBuf[i + 4], pBuf[i + 5], pBuf[i + 6], pBuf[i + 7]);
    }

    /* Compare whether the read data is correct from write data */
    for (uint32_t j = 0; j < BUF_BYTES; j++)
    {
        if (((j < BUF_BYTES / 2) && ((pBuf[j] != 0xff))) ||
            ((j >= BUF_BYTES / 2) && (j < BUF_BYTES) && (pBuf[j] != 0x00)))
        {
            DBG_DIRECT("Block %d: 0x%04x -> 0x%02x data error", j / 512, j % 512, pBuf[j]);
            if (pBuf[j] == 0x77) { break; }
        }
    }

    os_mem_free(pBuf);
}


/**
  * @brief  SD interrupt handler function.
  * @param  None.
  * @return None.
  */
void SDHC0_Handler(void)
{
    DisableIntrByNvic(SDHC0);
    os_sem_give(gSDHC0Sem);
}