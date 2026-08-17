/**
*********************************************************************************************************
*               Copyright(c) 2023, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* \file     mmc.h
* \brief    The header file of MMC.
* \details  This file provides MMC firmware functions.
* \author   Bert
* \date     2023-07-05
* *********************************************************************************************************
*/

#ifndef MMC_H
#define MMC_H

/*============================================================================*
 *                         Includes
 *============================================================================*/
#include "rtl_sdhc.h"
#include "sd.h"


/*============================================================================*
 *                         Functions
 *============================================================================*/
/* All the following APIs must run in task environment. */

/**
  * @brief  EMMC init flow.
  * @param  SDHCx: Specifies the SDHC peripheral.
  * @param  pParm: Specifies the parameters for CardType, DataWidth, ClkOutFreq_kHz.
            Please refer to SdEmmcInitParm_t for more details.
  * @return SdEmmcRes: Please refer to SdEmmcRes_t for more details.
  */
SdEmmcRes_t Emmc_Init(SDHC_TypeDef *SDHCx, const SdEmmcInitParm_t *pParm);

/**
  * @brief  EMMC read data.
  * @param  SDHCx: Specifies the SDHC peripheral.
  * @param  StartBlock:  Start block.
  * @param  BlockCnt:  Block count.
  * @param  pBuf: Pointer to a read buffer.
  * @return SdEmmcRes: Please refer to SdEmmcRes_t for more details.
  */
SdEmmcRes_t Emmc_Read(SDHC_TypeDef *SDHCx, uint32_t StartBlock, uint32_t BlockCnt, void *pBuf);

/**
  * @brief  EMMC write data.
  * @param  SDHCx: Specifies the SDHC peripheral.
  * @param  StartBlock:  Start block.
  * @param  BlockCnt:  Block count.
  * @param  pBuf: Pointer to a write buffer.
  * @return SdEmmcRes: Please refer to SdEmmcRes_t for more details.
  */
SdEmmcRes_t Emmc_Write(SDHC_TypeDef *SDHCx, uint32_t StartBlock, uint32_t BlockCnt,
                       const void *pBuf);

/**
  * @brief  Get Block count.
  * @param  SDHCx: Specifies the SDHC peripheral.
  * @return Block count: Block count for transfer .
  */
uint32_t Emmc_GetBlockCnt(SDHC_TypeDef *SDHCx);

#endif /* MMC_H */
