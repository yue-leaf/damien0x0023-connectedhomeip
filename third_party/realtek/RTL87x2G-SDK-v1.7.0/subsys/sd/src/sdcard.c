/**
*********************************************************************************************************
*               Copyright(c) 2023, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* \file     sdcard.c
* \brief    The header file of SDCard.
* \details  This file provides SDCard firmware functions.
* \author   Bert
* \date     2023-07-05
* *********************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "sdcard.h"
#include "sd_utils.h"
#include "os_mem.h"
#include "os_sched.h"


/* Private define ------------------------------------------------------------*/
// SD 3.01 Command Index
#define SD_GO_IDLE_STATE        0
#define SD_ALL_SEND_CID         2
#define SD_SEND_RELATIVE_ADDR   3
#define SD_SET_DSR              4
#define SD_IO_SEND_OP_COND      5
#define SD_SWITCH_FUNC          6
#define SD_SELECT_DESELECT_CARD 7
#define SD_SEND_IF_COND         8
#define SD_SEND_CSD             9
#define SD_SEND_CID             10
#define SD_VOLTAGE_SWITCH       11
#define SD_READ_DAT_UTIL_STOP   11
#define SD_STOP_TRANSMISSION    12
#define SD_SEND_STATUS          13
#define SD_GO_INACTIVE_STATE    15

#define SD_SET_BLOCKLEN         16
#define SD_READ_SINGLE_BLOCK    17
#define SD_READ_MULTIPLE_BLOCK  18
#define SD_SEND_TUNING_PATTERN  19

#define SD_SET_BLOCK_COUNT      23
#define SD_WRITE_BLOCK          24
#define SD_WRITE_MULTIPLE_BLOCK 25
#define SD_PROGRAM_CSD          27

#define SD_ERASE_WR_BLK_START   32
#define SD_ERASE_WR_BLK_END     33
#define SD_ERASE_CMD            38

#define SD_LOCK_UNLOCK          42
#define SD_IO_RW_DIRECT         52

#define SD_APP_CMD              55
#define SD_GEN_CMD              56

// ACMD
#define SD_SET_BUS_WIDTH        6
#define SD_SD_STATUS            13
#define SD_SEND_NUM_WR_BLOCKS   22
#define SD_SET_WR_BLK_ERASE_COUNT  23
#define SD_SD_APP_OP_COND       41
#define SD_SET_CLR_CARD_DETECT  42
#define SD_SEND_SCR             51


#define SDR12_CLKFREQ_kHz  (25 * 1000)
#define CARD_IDENTIFICATION_MODE_CLK_FREQ_kHz  400

#define CMD6_STATUS_DATA_BYTES  (512/8)


typedef struct
{
    uint32_t Reserved0: 3;
    uint32_t AkeSeqError: 1;

    uint32_t Reserved1: 1;
    uint32_t AppCmd: 1;

    uint32_t Reserved2 : 2;
    uint32_t ReadyForData: 1;
    uint32_t CurrentState: 4;
    uint32_t EraseReset: 1;
    uint32_t CardEccDisabled: 1;
    uint32_t WpErarseSkip: 1;
    uint32_t CsdOverwrite: 1;

    uint32_t Reserved3: 2;
    uint32_t Error: 1;
    uint32_t CcError: 1;
    uint32_t CardEccFailed: 1;
    uint32_t IllegalCommand: 1;
    uint32_t ComCrcError: 1;
    uint32_t LockUnlockFailure: 1;
    uint32_t CardIsLocked: 1;
    uint32_t WpViolation: 1;
    uint32_t EraseParam: 1;
    uint32_t EraseSeqError: 1;
    uint32_t BlockLenError: 1;
    uint32_t AddressError: 1;
    uint32_t OutOfRange: 1;
} R1Rsp_t, CardStatus_t;

#define CARDSTATE_IDLE  0
#define CARDSTATE_READY  1
#define CARDSTATE_IDENT  2
#define CARDSTATE_STBY  3
#define CARDSTATE_TRAN  4
#define CARDSTATE_DATA  5
#define CARDSTATE_RCV  6
#define CARDSTATE_PRG  7
#define CARDSTATE_DIS  8


#define CSD_CID_BYTES  16

typedef struct
{
    uint8_t aCidOrCsd[CSD_CID_BYTES];
} R2Rsp_t;


typedef struct
{
    uint32_t Reserved0: 15;
    uint32_t _2p7_2p8: 1;
    uint32_t _2p8_2p9: 1;
    uint32_t _2p9_3p0: 1;
    uint32_t _3p0_3p1: 1;
    uint32_t _3p1_3p2: 1;
    uint32_t _3p2_3p3: 1;
    uint32_t _3p3_3p4: 1;
    uint32_t _3p4_3p5: 1;
    uint32_t _3p5_3p6: 1;
    uint32_t SwitchingTo1p8VAccepted_S18A: 1;
    uint32_t Reserved1: 5;
    uint32_t CardCapacityStatus_Ccs: 1;
    uint32_t CardPowerUpStatusBit_Busy: 1;
} R3Rsp_t, Ocr_t;

#define R3RSP_BUSY  0
#define R3RSP_NOT_BUSY  1


typedef struct
{
    uint32_t Reserved0: 3;
    uint32_t AkeSeqError: 1;
    uint32_t Reserved1: 1;
    uint32_t AppCmd: 1;
    uint32_t Reserved2 : 2;
    uint32_t ReadyForData: 1;
    uint32_t CurrentState: 4;
    uint32_t Error: 1;
    uint32_t IllegalCommand: 1;
    uint32_t ComCrcError: 1;

    uint32_t NewPublishedRca: 16;
} R6Rsp_t;


typedef struct
{
    uint32_t EchoBackPattern: 8;
    uint32_t VoltageAccepted: 4;
    uint32_t Reserved: 20;
} R7Rsp_t;


typedef struct
{
    uint64_t Reserved0: 32;
    uint64_t CmdSupport: 2;
    uint64_t Reserved1: 9;
    uint64_t ExSecurity: 4;
    uint64_t SdSpec3: 1;
    uint64_t SdBusWidth: 4;
    uint64_t SdSecurity: 3;
    uint64_t DataStatAfterErase: 1;
    uint64_t SdSpec: 4;
    uint64_t ScrStructure: 4;
} Scr_t;
STATIC_ASSERT(sizeof(Scr_t) == 8);


typedef enum
{
    SDTYPE_SDSC = 1,
    SDTYPE_SDHC_OR_SDXC,
} SdCardType_t;


typedef struct
{
    SdCardType_t CardType;
    uint16_t Rca;
    uint8_t aCsd[CSD_CID_BYTES];
    Scr_t Scr;
} SdDb_t;

// To deal with multiple IP instances.
static inline SdDb_t *GetSdDb(SDHC_TypeDef *SDHCx)
{
    static SdDb_t SdDb0, SdDb1;
    return (SDHCx == SDHC0) ? &SdDb0 : &SdDb1;
}


/* Private function prototypes -----------------------------------------------*/
static SdEmmcRes_t Cmd0_GoIdleState(SDHC_TypeDef *SDHCx);
static SdEmmcRes_t Cmd8_SendIfCond(SDHC_TypeDef *SDHCx, R7Rsp_t *pR7Rsp);
static SdEmmcRes_t Cmd55_AppCmd(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp);
static SdEmmcRes_t Acmd41_SdSendOpCond(SDHC_TypeDef *SDHCx, bool IsHcsSet, R3Rsp_t *pR3Rsp);
static SdEmmcRes_t Cmd2_AllSendCid(SDHC_TypeDef *SDHCx, R2Rsp_t *pR2Rsp);
static SdEmmcRes_t Cmd3_SendRelativeAddr(SDHC_TypeDef *SDHCx, R6Rsp_t *pR6Rsp);
static SdEmmcRes_t Cmd7_SelectCard(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp);
static SdEmmcRes_t Cmd9_SendCsd(SDHC_TypeDef *SDHCx, R2Rsp_t *pR2Rsp);
static SdEmmcRes_t Cmd13_SendStatus(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp);
static SdEmmcRes_t Acmd6_SetBusWidth(SDHC_TypeDef *SDHCx, DataWidth_t DataWidth, R1Rsp_t *pR1Rsp);
static SdEmmcRes_t Cmd16_SetBlockLen(SDHC_TypeDef *SDHCx, uint32_t BlockLen, R1Rsp_t *pR1Rsp);
static SdEmmcRes_t Acmd42_SetClrCardDetect(SDHC_TypeDef *SDHCx, bool IsCdSet, R1Rsp_t *pR1Rsp);
static SdEmmcRes_t Acmd51_SendScr(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp, void *pScr);
static SdEmmcRes_t Cmd6_SwitchFunc(SDHC_TypeDef *SDHCx, uint32_t CmdArg, R1Rsp_t *pR1Rsp,
                                   void *pStatusData);

static SdEmmcRes_t RepeatAcmd41UntilNotBusy(SDHC_TypeDef *SDHCx, bool IsHcsSet, R3Rsp_t *pR3Rsp,
                                            uint32_t Interval_ms, uint32_t MaxRepeatCnt);
static SdEmmcRes_t RepeatCmd13UntillIntoXferState(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp,
                                                  uint32_t Interval_ms, uint32_t MaxRepeatCnt);



/**
  * @brief  SDCard init flow.
  * @param  SDHCx: Specifies the SDHC peripheral.
  * @param  pParm: Specifies the parameters for CardType, DataWidth, ClkOutFreq_kHz.
            Please refer to SdEmmcInitParm_t for more details.
  * @return SdEmmcRes: Please refer to SdEmmcRes_t for more details.
  */
SdEmmcRes_t Sd_Init(SDHC_TypeDef *SDHCx, const SdEmmcInitParm_t *pParm)
{
    if (pParm == NULL || pParm->CardType != CARDTYPE_SD)
    {
        ASSERT(0);
        return SDEMMCRES_ILLEGAL_PARM;
    }

    SDHC_InitPad(SDHCx, pParm->DataWidth);
    SDHC_Init(SDHCx);
    SDHC_SetClkOutFreq(SDHCx, MIN2(CARD_IDENTIFICATION_MODE_CLK_FREQ_kHz, pParm->ClkOutFreq_kHz));

    uint8_t *pBuf = os_mem_alloc(RAM_TYPE_EXT_DATA_SRAM, CMD6_STATUS_DATA_BYTES);
    uint8_t *pSwap = os_mem_alloc(RAM_TYPE_EXT_DATA_SRAM, sizeof(Scr_t));
    if (pBuf == NULL || pSwap == NULL)
    {
        ASSERT(0);
        return SDEMMCRES_MALLOC_FAILED;
    }

    SdEmmcRes_t Res = SDEMMCRES_OK;
    R1Rsp_t R1Rsp;
    R2Rsp_t R2Rsp;
    R3Rsp_t R3Rsp;
    R6Rsp_t R6Rsp;
    R7Rsp_t R7Rsp;
    SdDb_t *pSdDb = GetSdDb(SDHCx);

    // The following steps are based on SD spec V3.01 figure 4-2 and figure 4-9.
    Res = Cmd0_GoIdleState(SDHCx);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }

    Res = Cmd8_SendIfCond(SDHCx, &R7Rsp);
    if (Res == SDEMMCRES_OK)
    {
        Res = RepeatAcmd41UntilNotBusy(SDHCx, true, &R3Rsp, 2, 500);
        if (Res != SDEMMCRES_OK)
        {
            goto Exit;
        }
        pSdDb->CardType = (R3Rsp.CardCapacityStatus_Ccs == 1) ? SDTYPE_SDHC_OR_SDXC : SDTYPE_SDSC;
    }
    else
    {
        Res = RepeatAcmd41UntilNotBusy(SDHCx, false, &R3Rsp, 2, 500);
        if (Res != SDEMMCRES_OK)
        {
            goto Exit;
        }
        pSdDb->CardType = SDTYPE_SDSC;
    }

    Res = Cmd2_AllSendCid(SDHCx, &R2Rsp);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }

    Res = Cmd3_SendRelativeAddr(SDHCx, &R6Rsp);
    if (Res != SDEMMCRES_OK || R6Rsp.NewPublishedRca == 0)
    {
        goto Exit;
    }
    pSdDb->Rca = R6Rsp.NewPublishedRca;

    Res = Cmd9_SendCsd(SDHCx, &R2Rsp);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }
    memcpy(pSdDb->aCsd, &R2Rsp, CSD_CID_BYTES);

    Res = Cmd7_SelectCard(SDHCx, &R1Rsp);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }

    Res = Acmd42_SetClrCardDetect(SDHCx, false, &R1Rsp);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }

    Res = Cmd16_SetBlockLen(SDHCx, BYTES_PER_BLOCK, &R1Rsp);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }

    SDHC_SetClkOutFreq(SDHCx, MIN2(SDR12_CLKFREQ_kHz, pParm->ClkOutFreq_kHz));

    Res = Acmd51_SendScr(SDHCx, &R1Rsp, pBuf);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }
    for (uint8_t i = 0; i < sizeof(Scr_t); i++)
    {
        *(pSwap + sizeof(Scr_t) - 1 - i) = *(pBuf + i);
    }
    memcpy(&pSdDb->Scr, pSwap, sizeof(Scr_t));

    // Refer to SD spec V3.01 4.3.10
    if (pParm->ClkOutFreq_kHz > SDR12_CLKFREQ_kHz && pSdDb->Scr.SdSpec >= 1)
    {
        Res = Cmd6_SwitchFunc(SDHCx, 0x00fffff1, &R1Rsp, pBuf);
        if (Res != SDEMMCRES_OK)
        {
            goto Exit;
        }
        // StatusData[379:376] == 0x1, which means switch to SDR25 is allowed.
        if ((pBuf[16] & 0xf) == 0x1)
        {
            Res = Cmd6_SwitchFunc(SDHCx, 0x80fffff1, &R1Rsp, pBuf);
            if (Res != SDEMMCRES_OK)
            {
                goto Exit;
            }

            //Check transfer state before switching host to SDR25
            Res = RepeatCmd13UntillIntoXferState(SDHCx, &R1Rsp, 1, 1000);
            if (Res != SDEMMCRES_OK)
            {
                goto Exit;
            }
            SDHC_SetClkOutFreq(SDHCx, pParm->ClkOutFreq_kHz);
        }
    }

    Res = Acmd6_SetBusWidth(SDHCx, pParm->DataWidth, &R1Rsp);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }
    SDHC_SetHostDataWidth(SDHCx, pParm->DataWidth);

Exit:
    os_mem_free(pBuf);
    os_mem_free(pSwap);
    return Res;
}

/**
  * @brief  SDCard read data.
  * @param  SDHCx: Specifies the SDHC peripheral.
  * @param  StartBlock:  Start block.
  * @param  BlockCnt:  Block count.
  * @param  pBuf: Pointer to a read buffer.
  * @return SdEmmcRes: Please refer to SdEmmcRes_t for more details.
  */
SdEmmcRes_t Sd_Read(SDHC_TypeDef *SDHCx, uint32_t StartBlock, uint32_t BlockCnt, void *pBuf)
{

    uint8_t *puBuf = pBuf;
    uint32_t BlockAddr = StartBlock;
    uint32_t RemainBlock = BlockCnt;

    while (RemainBlock > 0)
    {
        uint32_t BlockCntSend = MIN2(MAX_BLOCK_PER_XFER, RemainBlock);

        const CmdInfo_t Cmd18 =
        {
            .CmdIdx = SD_READ_MULTIPLE_BLOCK,
            .CmdArg = (GetSdDb(SDHCx)->CardType == SDTYPE_SDHC_OR_SDXC) ? BlockAddr : BlockAddr * BYTES_PER_BLOCK,
            .IsResetCmd = false,
            .IsStopCmd = false,
            .IsRspExpected = true,
            .IsR2Rsp = false,
            .CheckRspCrc = true,
        };
        const DataInfo_t DataInfo =
        {
            .BlockSize = BYTES_PER_BLOCK,
            .BlockCount = BlockCntSend,
            .SendAutoStop = true,
        };
        R1Rsp_t R1Rsp;
        SDHCRes_t SDHCRes = SDHC_SendCmdWithRxData(SDHCx, &Cmd18, &R1Rsp, &DataInfo, puBuf);
        if (SDHCRes != SDHCRES_OK || R1Rsp.Error)
        {
            DBG_DIRECT("SDHCRes: %d", SDHCRes);
            return SDEMMCRES_CMD18_ERROR;
        }

        puBuf += (BlockCntSend * BYTES_PER_BLOCK);
        BlockAddr += BlockCntSend;
        RemainBlock -= BlockCntSend;
    }

    return SDEMMCRES_OK;
}

/**
  * @brief  SDCard write data.
  * @param  SDHCx: Specifies the SDHC peripheral.
  * @param  StartBlock:  Start block.
  * @param  BlockCnt:  Block count.
  * @param  pBuf: Pointer to a write buffer.
  * @return SdEmmcRes: Please refer to SdEmmcRes_t for more details.
  */
SdEmmcRes_t Sd_Write(SDHC_TypeDef *SDHCx, uint32_t StartBlock, uint32_t BlockCnt, const void *pBuf)
{
    const uint8_t *puBuf = pBuf;
    uint32_t BlockAddr = StartBlock;
    uint32_t RemainBlock = BlockCnt;

    while (RemainBlock > 0)
    {
        uint32_t BlockCntSend = MIN2(MAX_BLOCK_PER_XFER, RemainBlock);

        const CmdInfo_t Cmd25 =
        {
            .CmdIdx = SD_WRITE_MULTIPLE_BLOCK,
            .CmdArg = (GetSdDb(SDHCx)->CardType == SDTYPE_SDHC_OR_SDXC) ? BlockAddr : BlockAddr * BYTES_PER_BLOCK,
            .IsResetCmd = false,
            .IsStopCmd = false,
            .IsRspExpected = true,
            .IsR2Rsp = false,
            .CheckRspCrc = true,
        };
        const DataInfo_t DataInfo =
        {
            .BlockSize = BYTES_PER_BLOCK,
            .BlockCount = BlockCntSend,
            .SendAutoStop = true,
        };
        R1Rsp_t R1Rsp;
        SDHCRes_t SDHCRes = SDHC_SendCmdWithTxData(SDHCx, &Cmd25, &R1Rsp, &DataInfo, puBuf);
        if (SDHCRes != SDHCRES_OK || R1Rsp.Error)
        {
            DBG_DIRECT("SDHCRes: %d", SDHCRes);
            return SDEMMCRES_CMD25_ERROR;
        }

        SDHCRes = SDHC_WaitData0Idle(SDHCx, 2000);
        if (SDHCRes != SDHCRES_OK)
        {
            return SDEMMCRES_WRITE_TIMEOUT;
        }

        SdEmmcRes_t SdEmmcRes = RepeatCmd13UntillIntoXferState(SDHCx, &R1Rsp, 1, 1000);
        if (SdEmmcRes != SDEMMCRES_OK)
        {
            return SdEmmcRes;
        }

        puBuf += (BlockCntSend * BYTES_PER_BLOCK);
        BlockAddr += BlockCntSend;
        RemainBlock -= BlockCntSend;
    }

    return SDEMMCRES_OK;
}

/**
  * @brief  Get Block count.
  * @param  SDHCx: Specifies the SDHC peripheral.
  * @return Block count: Block count for transfer .
  */
uint32_t Sd_GetBlockCnt(SDHC_TypeDef *SDHCx)
{
    SdDb_t *pSdDb = GetSdDb(SDHCx);

    uint64_t Capacity_Byte = 0;
    if (pSdDb->CardType == SDTYPE_SDHC_OR_SDXC)
    {
        uint32_t CSize = BitsToU32(pSdDb->aCsd, 48, 69);
        Capacity_Byte = (CSize + 1ULL) * 1024 * 512;
    }
    else
    {
        uint32_t CSize = BitsToU32(pSdDb->aCsd, 62, 73);
        uint32_t CSizeMult = BitsToU32(pSdDb->aCsd, 47, 49);
        uint32_t ReadBlLen = BitsToU32(pSdDb->aCsd, 80, 83);

        uint32_t Mult = 1UL << (CSizeMult + 2);
        uint32_t BlockLen = 1UL << ReadBlLen;
        uint32_t Blocknr = (CSize + 1) * Mult;

        Capacity_Byte = (uint64_t)Blocknr * BlockLen;
    }

    return Capacity_Byte / BYTES_PER_BLOCK;
}


static SdEmmcRes_t Cmd0_GoIdleState(SDHC_TypeDef *SDHCx)
{
    static const CmdInfo_t Cmd0 =
    {
        .CmdIdx = SD_GO_IDLE_STATE,
        .CmdArg = 0,
        .IsResetCmd = true,
        .IsStopCmd = false,
        .IsRspExpected = false,
        .IsR2Rsp = false,
        .CheckRspCrc = false,
    };
    SDHCRes_t Res = SDHC_SendNoDataCmd(SDHCx, &Cmd0, NULL);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_CMD0_ERROR;
    }
    return SDEMMCRES_OK;
}

static SdEmmcRes_t Cmd8_SendIfCond(SDHC_TypeDef *SDHCx, R7Rsp_t *pR7Rsp)
{
    static const CmdInfo_t Cmd8 =
    {
        .CmdIdx = SD_SEND_IF_COND,
        .CmdArg = 0x1aa,
        .IsResetCmd = false,
        .IsStopCmd = false,
        .IsRspExpected = true,
        .IsR2Rsp = false,
        .CheckRspCrc = true,
    };

    uint32_t R7Rsp;
    SDHCRes_t Res = SDHC_SendNoDataCmd(SDHCx, &Cmd8, pR7Rsp);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_CMD8_ERROR;
    }

    return SDEMMCRES_OK;
}

static SdEmmcRes_t Cmd55_AppCmd(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp)
{
    const CmdInfo_t Cmd55 =
    {
        .CmdIdx = SD_APP_CMD,
        .CmdArg = GetSdDb(SDHCx)->Rca << 16,
                                      .IsResetCmd = false,
                                      .IsStopCmd = false,
                                      .IsRspExpected = true,
                                      .IsR2Rsp = false,
                                      .CheckRspCrc = false,
    };

    R1Rsp_t R1Rsp;
    SDHCRes_t Res = SDHC_SendNoDataCmd(SDHCx, &Cmd55, pR1Rsp);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_CMD55_ERROR;
    }

    return SDEMMCRES_OK;
}

static SdEmmcRes_t Acmd41_SdSendOpCond(SDHC_TypeDef *SDHCx, bool IsHcsSet, R3Rsp_t *pR3Rsp)
{
    R1Rsp_t R1Rsp;
    SdEmmcRes_t SdEmmcRes = Cmd55_AppCmd(SDHCx, &R1Rsp);
    if (SdEmmcRes != SDEMMCRES_OK || R1Rsp.Error)
    {
        return SDEMMCRES_CMD55_ERROR;
    }

    CmdInfo_t Acmd41 =
    {
        .CmdIdx = SD_SD_APP_OP_COND,
        .CmdArg = BIT28 | BIT15 | BIT16 | BIT17 | BIT18 | BIT19 | BIT20 | BIT21 | BIT22 | BIT23,
        .IsResetCmd = false,
        .IsStopCmd = false,
        .IsRspExpected = true,
        .IsR2Rsp = false,
        .CheckRspCrc = false,
    };
    if (IsHcsSet)
    {
        Acmd41.CmdArg |= BIT30;
    }

    SDHCRes_t Res = SDHC_SendNoDataCmd(SDHCx, &Acmd41, pR3Rsp);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_ACMD41_ERROR;
    }

    return SDEMMCRES_OK;
}

static SdEmmcRes_t RepeatAcmd41UntilNotBusy(SDHC_TypeDef *SDHCx, bool IsHcsSet, R3Rsp_t *pR3Rsp,
                                            uint32_t Interval_ms, uint32_t MaxRepeatCnt)
{
    for (uint32_t i = 0; i < MaxRepeatCnt; ++i)
    {
        os_delay(Interval_ms);

        SdEmmcRes_t Res = Acmd41_SdSendOpCond(SDHCx, IsHcsSet, pR3Rsp);
        if (Res != SDEMMCRES_OK)
        {
            return Res;
        }
        if (pR3Rsp->CardPowerUpStatusBit_Busy == R3RSP_NOT_BUSY)
        {
            return SDEMMCRES_OK;
        }
    }

    return SDEMMCRES_ACMD41_TIMEOUT;
}


static SdEmmcRes_t RepeatCmd13UntillIntoXferState(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp,
                                                  uint32_t Interval_ms, uint32_t MaxRepeatCnt)
{
    for (uint32_t i = 0; i < MaxRepeatCnt; ++i)
    {
        SdEmmcRes_t Res = Cmd13_SendStatus(SDHCx, pR1Rsp);
        if (Res != SDEMMCRES_OK)
        {
            return Res;
        }
        if (pR1Rsp->CurrentState == CARDSTATE_TRAN && pR1Rsp->ReadyForData)
        {
            return SDEMMCRES_OK;
        }
        os_delay(Interval_ms);
    }

    return SDEMMCRES_CMD13_TIMEOUT;
}

static SdEmmcRes_t Cmd2_AllSendCid(SDHC_TypeDef *SDHCx, R2Rsp_t *pR2Rsp)
{
    static const CmdInfo_t Cmd2 =
    {
        .CmdIdx = SD_ALL_SEND_CID,
        .CmdArg = 0,
        .IsResetCmd = false,
        .IsStopCmd = false,
        .IsRspExpected = true,
        .IsR2Rsp = true,
        .CheckRspCrc = false,
    };

    SDHCRes_t Res = SDHC_SendNoDataCmd(SDHCx, &Cmd2, pR2Rsp);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_CMD2_ERROR;
    }

    return SDEMMCRES_OK;
}


static SdEmmcRes_t Cmd3_SendRelativeAddr(SDHC_TypeDef *SDHCx, R6Rsp_t *pR6Rsp)
{
    static const CmdInfo_t Cmd3 =
    {
        .CmdIdx = SD_SEND_RELATIVE_ADDR,
        .CmdArg = 0,
        .IsResetCmd = false,
        .IsStopCmd = false,
        .IsRspExpected = true,
        .IsR2Rsp = false,
        .CheckRspCrc = true,
    };
    SDHCRes_t Res = SDHC_SendNoDataCmd(SDHCx, &Cmd3, pR6Rsp);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_CMD3_ERROR;
    }

    return SDEMMCRES_OK;
}


static SdEmmcRes_t Cmd7_SelectCard(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp)
{
    const CmdInfo_t Cmd7 =
    {
        .CmdIdx = SD_SELECT_DESELECT_CARD,
        .CmdArg = GetSdDb(SDHCx)->Rca << 16,
                                      .IsResetCmd = false,
                                      .IsStopCmd = false,
                                      .IsRspExpected = true,
                                      .IsR2Rsp = false,
                                      .CheckRspCrc = true,
    };
    SDHCRes_t Res = SDHC_SendNoDataCmd(SDHCx, &Cmd7, pR1Rsp);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_CMD7_ERROR;
    }

    return SDEMMCRES_OK;
}


static SdEmmcRes_t Cmd9_SendCsd(SDHC_TypeDef *SDHCx, R2Rsp_t *pR2Rsp)
{
    const CmdInfo_t Cmd9 =
    {
        .CmdIdx = SD_SEND_CSD,
        .CmdArg = GetSdDb(SDHCx)->Rca << 16,
                                      .IsResetCmd = false,
                                      .IsStopCmd = false,
                                      .IsRspExpected = true,
                                      .IsR2Rsp = true,
                                      .CheckRspCrc = false,
    };

    SDHCRes_t Res = SDHC_SendNoDataCmd(SDHCx, &Cmd9, pR2Rsp);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_CMD9_ERROR;
    }

    return SDEMMCRES_OK;
}

static SdEmmcRes_t Cmd13_SendStatus(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp)
{
    const CmdInfo_t Cmd13 =
    {
        .CmdIdx = SD_SEND_STATUS,
        .CmdArg = GetSdDb(SDHCx)->Rca << 16,
                                      .IsResetCmd = false,
                                      .IsStopCmd = false,
                                      .IsRspExpected = true,
                                      .IsR2Rsp = false,
                                      .CheckRspCrc = true,
    };
    SDHCRes_t Res = SDHC_SendNoDataCmd(SDHCx, &Cmd13, pR1Rsp);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_CMD13_ERROR;
    }

    return SDEMMCRES_OK;
}

static SdEmmcRes_t Acmd6_SetBusWidth(SDHC_TypeDef *SDHCx, DataWidth_t DataWidth, R1Rsp_t *pR1Rsp)
{
    if (DataWidth != DATAWIDTH_1BIT && DataWidth != DATAWIDTH_4BIT)
    {
        return SDEMMCRES_ILLEGAL_PARM;
    }

    SdEmmcRes_t SdEmmcRes = Cmd55_AppCmd(SDHCx, pR1Rsp);
    if (SdEmmcRes != SDEMMCRES_OK || pR1Rsp->Error)
    {
        return SDEMMCRES_CMD55_ERROR;
    }

    const CmdInfo_t Acmd6 =
    {
        .CmdIdx = SD_SET_BUS_WIDTH,
        .CmdArg = (DataWidth == DATAWIDTH_4BIT) ? 0x2 : 0x0,
        .IsResetCmd = false,
        .IsStopCmd = false,
        .IsRspExpected = true,
        .IsR2Rsp = false,
        .CheckRspCrc = false,
    };

    SDHCRes_t Res = SDHC_SendNoDataCmd(SDHCx, &Acmd6, pR1Rsp);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_ACMD6_ERROR;
    }

    return SDEMMCRES_OK;
}

static SdEmmcRes_t Cmd16_SetBlockLen(SDHC_TypeDef *SDHCx, uint32_t BlockLen, R1Rsp_t *pR1Rsp)
{
    const CmdInfo_t Cmd16 =
    {
        .CmdIdx = SD_SET_BLOCKLEN,
        .CmdArg = BlockLen,
        .IsResetCmd = false,
        .IsStopCmd = false,
        .IsRspExpected = true,
        .IsR2Rsp = false,
        .CheckRspCrc = true,
    };
    SDHCRes_t Res = SDHC_SendNoDataCmd(SDHCx, &Cmd16, pR1Rsp);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_CMD16_ERROR;
    }

    return SDEMMCRES_OK;
}


static SdEmmcRes_t Acmd42_SetClrCardDetect(SDHC_TypeDef *SDHCx, bool IsCdSet, R1Rsp_t *pR1Rsp)
{
    SdEmmcRes_t SdEmmcRes = Cmd55_AppCmd(SDHCx, pR1Rsp);
    if (SdEmmcRes != SDEMMCRES_OK || pR1Rsp->Error)
    {
        return SDEMMCRES_CMD55_ERROR;
    }

    const CmdInfo_t Acmd42 =
    {
        .CmdIdx = SD_SET_CLR_CARD_DETECT,
        .CmdArg = IsCdSet,
        .IsResetCmd = false,
        .IsStopCmd = false,
        .IsRspExpected = true,
        .IsR2Rsp = false,
        .CheckRspCrc = true,
    };
    SDHCRes_t Res = SDHC_SendNoDataCmd(SDHCx, &Acmd42, pR1Rsp);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_ACMD42_ERROR;
    }

    return SDEMMCRES_OK;
}


static SdEmmcRes_t Acmd51_SendScr(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp, void *pScr)
{
    SdEmmcRes_t SdEmmcRes = Cmd55_AppCmd(SDHCx, pR1Rsp);
    if (SdEmmcRes != SDEMMCRES_OK || pR1Rsp->Error)
    {
        return SDEMMCRES_CMD55_ERROR;
    }

    static const CmdInfo_t Acmd51 =
    {
        .CmdIdx = SD_SEND_SCR,
        .CmdArg = 0,
        .IsResetCmd = false,
        .IsStopCmd = false,
        .IsRspExpected = true,
        .IsR2Rsp = false,
        .CheckRspCrc = true,
    };
    static const DataInfo_t DataInfo =
    {
        .BlockSize = sizeof(Scr_t),
        .BlockCount = 1,
        .SendAutoStop = false,
    };
    SDHCRes_t Res = SDHC_SendCmdWithRxData(SDHCx, &Acmd51, pR1Rsp, &DataInfo, pScr);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_ACMD51_ERROR;
    }

    return SDEMMCRES_OK;
}


static SdEmmcRes_t Cmd6_SwitchFunc(SDHC_TypeDef *SDHCx, uint32_t CmdArg, R1Rsp_t *pR1Rsp,
                                   void *pStatusData)
{
    const CmdInfo_t Cmd6 =
    {
        .CmdIdx = SD_SWITCH_FUNC,
        .CmdArg = CmdArg,
        .IsResetCmd = false,
        .IsStopCmd = false,
        .IsRspExpected = true,
        .IsR2Rsp = false,
        .CheckRspCrc = true,
    };
    static const DataInfo_t DataInfo =
    {
        .BlockSize = CMD6_STATUS_DATA_BYTES,
        .BlockCount = 1,
        .SendAutoStop = false,
    };
    SDHCRes_t Res = SDHC_SendCmdWithRxData(SDHCx, &Cmd6, pR1Rsp, &DataInfo, pStatusData);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_CMD6_ERROR;
    }

    return SDEMMCRES_OK;
}



