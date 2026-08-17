/**
*********************************************************************************************************
*               Copyright(c) 2023, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* \file     sd.h
* \brief    The header file of SD APP API.
* \details  This file provides SD firmware functions for app.
* \author   Bert
* \date     2023-10-27
* \version  v1.0
* *********************************************************************************************************
*/

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef SD_H
#define SD_H

/*============================================================================*
 *                         Includes
 *============================================================================*/
#include "rtl_sdhc.h"

/** \defgroup SD        SD
  * \brief
  * \{
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/
/** \defgroup SD_Exported_Constants SD Exported Constants
 * \brief
 * \{
 */

/**
 * \defgroup    SD_EMMC_Res SD/EMMC Return Value
 * \{
 * \ingroup     SD_Exported_Constants
 */
typedef enum
{
    SDEMMCRES_OK = 0,           //!< Status OK.

    SDEMMCRES_ILLEGAL_PARM,     //!< Illegal Parameter.

    SDEMMCRES_CMD0_ERROR,       //!< Command 0 Error (SD/EMMC go IDLE State).
    SDEMMCRES_CMD8_ERROR,       //!< Command 8 Error (EMMC Send external CSD(Completion Signal Disable) & SD Send Interface Condition).
    SDEMMCRES_CMD55_ERROR,      //!< Command 55 Error (SD/EMMC APP Command).
    SDEMMCRES_ACMD41_ERROR,     //!< Application Command 41 Error (SD Application Operation Condition).
    SDEMMCRES_ACMD41_TIMEOUT,   //!< Application Command 41 Timeout (SD Application Operation Condition).
    SDEMMCRES_CMD2_ERROR,       //!< Command 2 Error (SD/EMMC All Send CID).
    SDEMMCRES_CMD3_ERROR,       //!< Command 3 Error (SD/EMMC Send Relative Address).
    SDEMMCRES_CMD7_ERROR,       //!< Command 7 Error (SD/EMMC Select/Deselect Card).
    SDEMMCRES_CMD9_ERROR,       //!< Command 9 Error (SD/EMMC Send CSD).
    SDEMMCRES_CMD13_ERROR,      //!< Command 13 Error (SD/EMMC Send Status).
    SDEMMCRES_ACMD6_ERROR,      //!< Application Command 6 Error (SD Set Bus Width).
    SDEMMCRES_CMD16_ERROR,      //!< Command 16 Error (SD/EMMC Set Block Length).
    SDEMMCRES_ACMD42_ERROR,     //!< Application Command 42 Error (SD Set/Clear Card Detect).
    SDEMMCRES_CMD18_ERROR,      //!< Command 18 Error (SD/EMMC Read Multiple Blocks).
    SDEMMCRES_CMD13_TIMEOUT,    //!< Command 13 Timeout (SD/EMMC Send Status).
    SDEMMCRES_ACMD51_ERROR,     //!< Application Command 51 Error (SD Send SD Configuration Register).
    SDEMMCRES_CMD6_ERROR,       //!< Command 6 Error (SD/EMMC Switch Function).
    SDEMMCRES_CMD25_ERROR,      //!< Command 25 Error (SD/EMMC Write Multiple Blocks).
    SDEMMCRES_CMD1_ERROR,       //!< Command 1 Error (EMMC Send Operation Conditions).
    SDEMMCRES_CMD1_TIMEOUT,     //!< Command 1 Timeout (EMMC Send Operation Conditions).
    SDEMMCRES_CMD6_TIMEOUT,     //!< Command 6 Timeout (SD/EMMC Switch Function).
    SDEMMCRES_WRITE_TIMEOUT,    //!< SD/EMMC Write Timeout.
    SDEMMCRES_MALLOC_FAILED,    //!< SD/EMMC Memory Malloc Failed.
} SdEmmcRes_t;
/** End of SD_EMMC_Res
  * \}
  */

/** End of SD_Exported_Constants
  * \}
  */

/** \defgroup SD_Exported_Types SD Exported Types
 * \brief
 * \{
 */

/**
 * \defgroup    Card_Type Card Type
 * \{
 * \ingroup     SD_Exported_Types
 */
typedef enum
{
    CARDTYPE_SD = 1,            //!< Card Type SD.
    CARDTYPE_EMMC,              //!< Card Type EMMC.
} CardType_t;
/** End of Card_Type
  * \}
  */

/**
 * \defgroup    SD_EMMC_Init_Parameters SD/EMMC Init Parameters
 * \{
 * \ingroup     SD_Exported_Types
 */
typedef struct
{
    // Pin group.
    // Power enable pin and active level.

    // Enter DLPS callback;
    // Power on/off callback;
    // Auto power off;
    // ...

    CardType_t
    CardType;        /*!< Specify the Card type. This parameter can be a value of \ref Card_Type */
    DataWidth_t DataWidth;      /*!< Specify the Data Width. */
    uint32_t ClkOutFreq_kHz;    /*!< Specify the Clock Output Frequency.*/
} SdEmmcInitParm_t;

/** End of SD_EMMC_Init_Parameters
  * \}
  */

/** End of SD_Exported_Types
  * \}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/
/* All the following APIs must run in task environment. */

/** \defgroup SD_Exported_Functions SD Exported Functions
  * \brief
  * \{
  */

/**
  * \brief  SDCard or EMMC init flow.
  * \param[in]  SDHCx: Specifies the SDHC peripheral.
  * \param[in]  pParm: Specifies the parameters for CardType, DataWidth, ClkOutFreq_kHz.
            Please refer to \ref SD_EMMC_Init_Parameters for more details.
  * \return Please refer to \ref SD_EMMC_Res for more details.
  */
SdEmmcRes_t SdEmmc_Init(SDHC_TypeDef *SDHCx, const SdEmmcInitParm_t *pParm);

/**
  * \brief  SDCard or EMMC read data.
  * \param[in]  SDHCx: Specifies the SDHC peripheral.
  * \param[in]  StartBlock:  Start block.
  * \param[in]  BlockCnt:  Block count.
  * \param[out]  pBuf: Pointer to a read buffer.
  * \return Please refer to \ref SD_EMMC_Res for more details.
  */
SdEmmcRes_t SdEmmc_Read(SDHC_TypeDef *SDHCx, uint32_t StartBlock, uint32_t BlockCnt, void *pBuf);

/**
  * \brief  SDCard or EMMC write data.
  * \param[in]  SDHCx: Specifies the SDHC peripheral.
  * \param[in]  StartBlock:  Start block.
  * \param[in]  BlockCnt:  Block count.
  * \param[in]  pBuf: Pointer to a write buffer.
  * \return Please refer to \ref SD_EMMC_Res for more details.
  */
SdEmmcRes_t SdEmmc_Write(SDHC_TypeDef *SDHCx, uint32_t StartBlock, uint32_t BlockCnt,
                         const void *pBuf);

/**
  * \brief   SDCard or EMMC get block count.
  * \param[in]  SDHCx: Specifies the SDHC peripheral.
  * \return Block count for transfer.
  */
uint32_t SdEmmc_GetBlockCnt(SDHC_TypeDef *SDHCx);

/** End of SD_Exported_Functions
  * \}
  */

/** End of SD
  * \}
  */

#endif /* SD_H */
