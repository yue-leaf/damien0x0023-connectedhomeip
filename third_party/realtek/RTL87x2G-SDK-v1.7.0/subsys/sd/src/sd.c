/**
*********************************************************************************************************
*               Copyright(c) 2023, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* \file     sd.c
* \brief    The header file of SD APP API.
* \details  This file provides SD firmware functions for app.
* \author   Bert
* \date     2023-10-27
* \version  v1.0
* *********************************************************************************************************
*/

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "sd.h"
#include "sdcard.h"
#include "mmc.h"

/*============================================================================*
 *                          Private Functions
 *============================================================================*/
static inline void SetCardType(SDHC_TypeDef *SDHCx, CardType_t CardType);
static inline CardType_t GetCardType(SDHC_TypeDef *SDHCx);

/*============================================================================*
 *                           Public Functions
 *============================================================================*/
/**
  * \brief  SDCard or EMMC init flow.
  * \param  SDHCx: Specifies the SDHC peripheral.
  * \param  pParm: Specifies the parameters for CardType, DataWidth, ClkOutFreq_kHz.
            Please refer to SdEmmcInitParm_t for more details.
  * \return SdEmmcRes: Please refer to SdEmmcRes_t for more details.
  */
SdEmmcRes_t SdEmmc_Init(SDHC_TypeDef *SDHCx, const SdEmmcInitParm_t *pParm)
{
    return (GetCardType(SDHCx) == CARDTYPE_SD) ? Sd_Init(SDHCx, pParm) : Emmc_Init(SDHCx, pParm);
}


/**
  * \brief  SDCard or EMMC read data.
  * \param  SDHCx: Specifies the SDHC peripheral.
  * \param  StartBlock:  Start block.
  * \param  BlockCnt:  Block count.
  * \param  pBuf: Pointer to a read buffer.
  * \return SdEmmcRes: Please refer to SdEmmcRes_t for more details.
  */
SdEmmcRes_t SdEmmc_Read(SDHC_TypeDef *SDHCx, uint32_t StartBlock, uint32_t BlockCnt, void *pBuf)
{
    return (GetCardType(SDHCx) == CARDTYPE_SD) ?
           Sd_Read(SDHCx, StartBlock, BlockCnt, pBuf) :
           Emmc_Read(SDHCx, StartBlock, BlockCnt, pBuf);
}


/**
  * \brief  SDCard or EMMC write data.
  * \param  SDHCx: Specifies the SDHC peripheral.
  * \param  StartBlock:  Start block.
  * \param  BlockCnt:  Block count.
  * \param  pBuf: Pointer to a write buffer.
  * \return SdEmmcRes: Please refer to SdEmmcRes_t for more details.
  */
SdEmmcRes_t SdEmmc_Write(SDHC_TypeDef *SDHCx, uint32_t StartBlock, uint32_t BlockCnt,
                         const void *pBuf)
{
    return (GetCardType(SDHCx) == CARDTYPE_SD) ?
           Sd_Write(SDHCx, StartBlock, BlockCnt, pBuf) :
           Emmc_Write(SDHCx, StartBlock, BlockCnt, pBuf);
}


/**
  * \brief  SDCard or EMMC get block count.
  * \param  SDHCx: Specifies the SDHC peripheral.
  * \return Block count: Block count for transfer .
  */
uint32_t SdEmmc_GetBlockCnt(SDHC_TypeDef *SDHCx)
{
    return (GetCardType(SDHCx) == CARDTYPE_SD) ? Sd_GetBlockCnt(SDHCx) : Emmc_GetBlockCnt(SDHCx);
}



static CardType_t gSDHC0CardType, gSDHC1CardType;

static inline void SetCardType(SDHC_TypeDef *SDHCx, CardType_t CardType)
{
    if (SDHCx == SDHC0)
    {
        gSDHC0CardType = CardType;
    }
    else
    {
        gSDHC1CardType = CardType;
    }
}

static inline CardType_t GetCardType(SDHC_TypeDef *SDHCx)
{
    return (SDHCx == SDHC0) ? gSDHC0CardType : gSDHC1CardType;
}
