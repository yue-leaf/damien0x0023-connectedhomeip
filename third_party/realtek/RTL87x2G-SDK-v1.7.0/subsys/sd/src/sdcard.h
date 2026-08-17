/**
*********************************************************************************************************
*               Copyright(c) 2023, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* \file     sdcard.h
* \brief    The header file of SDCard.
* \details  This file provides SDCard firmware functions.
* \author   Bert
* \date     2023-07-05
* *********************************************************************************************************
*/

#ifndef SDCARD_H
#define SDCARD_H

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
  * @brief  SDCard init flow.
  * @param  SDHCx: Specifies the SDHC peripheral.
  * @param  pParm: Specifies the parameters for CardType, DataWidth, ClkOutFreq_kHz.
            Please refer to SdEmmcInitParm_t for more details.
  * @return SdEmmcRes: Please refer to SdEmmcRes_t for more details.
  */
SdEmmcRes_t Sd_Init(SDHC_TypeDef *SDHCx, const SdEmmcInitParm_t *pParm);

/**
  * @brief  SDCard read data.
  * @param  SDHCx: Specifies the SDHC peripheral.
  * @param  StartBlock:  Start block.
  * @param  BlockCnt:  Block count.
  * @param  pBuf: Pointer to a read buffer.
  * @return SdEmmcRes: Please refer to SdEmmcRes_t for more details.
  */
SdEmmcRes_t Sd_Read(SDHC_TypeDef *SDHCx, uint32_t StartBlock, uint32_t BlockCnt, void *pBuf);


/**
  * @brief  SDCard write data.
  * @param  SDHCx: Specifies the SDHC peripheral.
  * @param  StartBlock:  Start block.
  * @param  BlockCnt:  Block count.
  * @param  pBuf: Pointer to a write buffer.
  * @return SdEmmcRes: Please refer to SdEmmcRes_t for more details.
  */
SdEmmcRes_t Sd_Write(SDHC_TypeDef *SDHCx, uint32_t StartBlock, uint32_t BlockCnt, const void *pBuf);

/**
  * @brief  Get Block count.
  * @param  SDHCx: Specifies the SDHC peripheral.
  * @return Block count: Block count for transfer .
  */
uint32_t Sd_GetBlockCnt(SDHC_TypeDef *SDHCx);

#endif /* SDCARD_H */
