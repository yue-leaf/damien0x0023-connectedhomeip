/**
*********************************************************************************************************
*               Copyright(c) 2023, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* \file     mmc.c
* \brief    The header file of MMC.
* \details  This file provides MMC firmware functions.
* \author   Bert
* \date     2023-07-05
* *********************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "mmc.h"
#include "sd_utils.h"
#include "os_mem.h"
#include "os_sched.h"


/* Private define ------------------------------------------------------------*/
/* Standard MMC commands (4.1)           type  argument     response */
/* class 1 */
#define MMC_GO_IDLE_STATE         0   /* bc                          */
#define MMC_SEND_OP_COND          1   /* bcr  [31:0] OCR         R3  */
#define MMC_ALL_SEND_CID          2   /* bcr                     R2  */
#define MMC_SET_RELATIVE_ADDR     3   /* ac   [31:16] RCA        R1  */
#define MMC_SET_DSR               4   /* bc   [31:16] RCA            */
#define MMC_SLEEP_AWAKE           5   /* ac   [31:16] RCA 15:flg R1b */
#define MMC_SWITCH                6   /* ac   [31:0] See below   R1b */
#define MMC_SELECT_CARD           7   /* ac   [31:16] RCA        R1  */
#define MMC_SEND_EXT_CSD          8   /* adtc                    R1  */
#define MMC_SEND_CSD              9   /* ac   [31:16] RCA        R2  */
#define MMC_SEND_CID             10   /* ac   [31:16] RCA        R2  */
#define MMC_READ_DAT_UNTIL_STOP  11   /* adtc [31:0] dadr        R1  */
#define MMC_STOP_TRANSMISSION    12   /* ac                      R1b */
#define MMC_SEND_STATUS          13   /* ac   [31:16] RCA        R1  */
#define MMC_BUS_TEST_R           14   /* adtc                    R1  */
#define MMC_GO_INACTIVE_STATE    15   /* ac   [31:16] RCA            */
#define MMC_BUS_TEST_W           19   /* adtc                    R1  */
#define MMC_SPI_READ_OCR         58   /* spi                  spi_R3 */
#define MMC_SPI_CRC_ON_OFF       59   /* spi  [0:0] flag      spi_R1 */

/* class 2 */
#define MMC_SET_BLOCKLEN         16   /* ac   [31:0] block len   R1  */
#define MMC_READ_SINGLE_BLOCK    17   /* adtc [31:0] data addr   R1  */
#define MMC_READ_MULTIPLE_BLOCK  18   /* adtc [31:0] data addr   R1  */
#define MMC_SEND_TUNING_BLOCK    19   /* adtc                    R1  */
#define MMC_SEND_TUNING_BLOCK_HS200 21  /* adtc R1  */

/* class 3 */
#define MMC_WRITE_DAT_UNTIL_STOP 20   /* adtc [31:0] data addr   R1  */

/* class 4 */
#define MMC_SET_BLOCK_COUNT      23   /* adtc [31:0] data addr   R1  */
#define MMC_WRITE_BLOCK          24   /* adtc [31:0] data addr   R1  */
#define MMC_WRITE_MULTIPLE_BLOCK 25   /* adtc                    R1  */
#define MMC_PROGRAM_CID          26   /* adtc                    R1  */
#define MMC_PROGRAM_CSD          27   /* adtc                    R1  */

/* class 6 */
#define MMC_SET_WRITE_PROT       28   /* ac   [31:0] data addr   R1b */
#define MMC_CLR_WRITE_PROT       29   /* ac   [31:0] data addr   R1b */
#define MMC_SEND_WRITE_PROT      30   /* adtc [31:0] wpdata addr R1  */

/* class 5 */
#define MMC_ERASE_GROUP_START    35   /* ac   [31:0] data addr   R1  */
#define MMC_ERASE_GROUP_END      36   /* ac   [31:0] data addr   R1  */
#define MMC_ERASE                38   /* ac                      R1b */

/* class 9 */
#define MMC_FAST_IO              39   /* ac   <Complex>          R4  */
#define MMC_GO_IRQ_STATE         40   /* bcr                     R5  */

/* class 7 */
#define MMC_LOCK_UNLOCK          42   /* adtc                    R1b */

/* class 8 */
#define MMC_APP_CMD              55   /* ac   [31:16] RCA        R1  */
#define MMC_GEN_CMD              56   /* adtc [0] RD/WR          R1  */

/* class 11 */
#define MMC_QUE_TASK_PARAMS      44   /* ac   [20:16] task id    R1  */
#define MMC_QUE_TASK_ADDR        45   /* ac   [31:0] data addr   R1  */
#define MMC_EXECUTE_READ_TASK    46   /* adtc [20:16] task id    R1  */
#define MMC_EXECUTE_WRITE_TASK   47   /* adtc [20:16] task id    R1  */
#define MMC_CMDQ_TASK_MGMT       48   /* ac   [20:16] task id    R1b */


#define CARD_IDENTIFICATION_MODE_CLK_FREQ_kHz  400
#define NORMAL_SPEED_MODE_CLK_FREQ_kHz  (20*1000)
#define HIGH_SPEED_MODE_CLK_FREQ_kHz  (52*1000)


typedef struct
{
    uint32_t Reserved0: 2;
    uint32_t Reserved1: 2;
    uint32_t Reserved2: 1;
    uint32_t AppCmd: 1;
    uint32_t UrgentBkops : 1;
    uint32_t SwitchError : 1;

    uint32_t ReadyForData: 1;
    uint32_t CurrentState: 4;
    uint32_t EraseReset: 1;
    uint32_t Reserved3: 1;
    uint32_t WpErarseSkip: 1;
    uint32_t CidCsdOverwrite: 1;

    uint32_t Overrun: 1;
    uint32_t Underrun: 1;
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
    uint32_t AddressMisalign: 1;
    uint32_t AddressOutOfRange: 1;
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
#define CARDSTATE_BTST  9
#define CARDSTATE_SLP  10



#define CSD_CID_BYTES  16

typedef struct
{
    uint8_t aCidOrCsd[CSD_CID_BYTES];
} R2Rsp_t;


#define EXT_CSD_BYTES  512

typedef struct
{
    uint32_t Reserved0: 7;
    uint32_t _1p70_1p95V: 1;
    uint32_t _2p0_2p6V: 7;
    uint32_t _2p7_3p6V: 9;
    uint32_t Reserved1: 5;
    uint32_t AcessMode: 2;
    uint32_t Busy: 1;
} Ocr_t, R3Rsp_t;

#define ACESSMODE_BYTE  0
#define ACESSMODE_SECTOR  2

#define R3RSP_BUSY  0
#define R3RSP_NOT_BUSY  1


typedef union
{
    uint32_t d32;
    struct
    {
        uint32_t CmdSet: 3;
        uint32_t Reserved0: 5;
        uint32_t Value: 8;
        uint32_t Index: 8;
        uint32_t Access: 2;
        uint32_t Reserved1: 6;
    } b;
} Cmd6Arg_t;

#define CMD6ACEESS_COMMAND_SET  0
#define CMD6ACEESS_SET_BITS  1
#define CMD6ACEESS_CLEAR_BITS  2
#define CMD6ACEESS_WRITE_BTYES  3


typedef enum
{
    EMMCTYPE_2GB_OR_LOWER = 1,
    EMMCTYPE_2GB_HIGHER,
} EmmcType_t;

#define RCA  0x10

typedef struct
{
    EmmcType_t CardType;
    uint8_t aCsd[CSD_CID_BYTES];
    uint32_t SectorCountInExtCsd;
} EmmcDb_t;


/* Private function prototypes -----------------------------------------------*/
static inline EmmcDb_t *GetEmmcDb(SDHC_TypeDef *SDHCx)
{
    static EmmcDb_t EmmcDb0, EmmcDb1;
    return (SDHCx == SDHC0) ? &EmmcDb0 : &EmmcDb1;
}

static SdEmmcRes_t Cmd0_GoIdleState(SDHC_TypeDef *SDHCx);
static SdEmmcRes_t Cmd1_SendOpCond(SDHC_TypeDef *SDHCx, R3Rsp_t *pR3Rsp);
static SdEmmcRes_t Cmd2_AllSendCid(SDHC_TypeDef *SDHCx, R2Rsp_t *pR2Rsp);
static SdEmmcRes_t Cmd3_SetRelativeAddr(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp);
static SdEmmcRes_t Cmd9_SendCsd(SDHC_TypeDef *SDHCx, R2Rsp_t *pR2Rsp);
static SdEmmcRes_t Cmd7_SelectCard(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp);
static SdEmmcRes_t Cmd16_SetBlockLen(SDHC_TypeDef *SDHCx, uint32_t BlockLen, R1Rsp_t *pR1Rsp);
static SdEmmcRes_t Cmd13_SendStatus(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp);
static SdEmmcRes_t Cmd8_SendExtCsd(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp, void *pExtCsd);
static SdEmmcRes_t Cmd6_Switch(SDHC_TypeDef *SDHCx, Cmd6Arg_t Cmd6Arg, R1Rsp_t *pR1Rsp);

static SdEmmcRes_t RepeatCmd1UntilNotBusy(SDHC_TypeDef *SDHCx, R3Rsp_t *pR3Rsp,
                                          uint32_t Interval_ms,
                                          uint32_t MaxRepeatCnt);
static SdEmmcRes_t RepeatCmd13UntillIntoXferState(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp,
                                                  uint32_t Interval_ms, uint32_t MaxRepeatCnt);


/**
  * @brief  EMMC init flow.
  * @param  SDHCx: Specifies the SDHC peripheral.
  * @param  pParm: Specifies the parameters for CardType, DataWidth, ClkOutFreq_kHz.
            Please refer to SdEmmcInitParm_t for more details.
  * @return SdEmmcRes: Please refer to SdEmmcRes_t for more details.
  */
SdEmmcRes_t Emmc_Init(SDHC_TypeDef *SDHCx, const SdEmmcInitParm_t *pParm)
{
    if (pParm == NULL || pParm->CardType != CARDTYPE_EMMC)
    {
        ASSERT(0);
        return SDEMMCRES_ILLEGAL_PARM;
    }

    SDHC_InitPad(SDHCx, pParm->DataWidth);
    SDHC_Init(SDHCx);
    SDHC_SetClkOutFreq(SDHCx, MIN2(CARD_IDENTIFICATION_MODE_CLK_FREQ_kHz, pParm->ClkOutFreq_kHz));

    uint8_t *pExtCsdBuf = os_mem_alloc(RAM_TYPE_EXT_DATA_SRAM, EXT_CSD_BYTES);
    if (pExtCsdBuf == NULL)
    {
        ASSERT(0);
        return SDEMMCRES_MALLOC_FAILED;
    }
    SdEmmcRes_t Res = SDEMMCRES_OK;
    R1Rsp_t R1Rsp;
    R2Rsp_t R2Rsp;
    R3Rsp_t R3Rsp;
    EmmcDb_t *pEmmcDb = GetEmmcDb(SDHCx);

    Res = Cmd0_GoIdleState(SDHCx);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }

    Res = RepeatCmd1UntilNotBusy(SDHCx, &R3Rsp, 2, 1000);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }
    pEmmcDb->CardType = (R3Rsp.AcessMode == ACESSMODE_SECTOR) ? EMMCTYPE_2GB_HIGHER :
                        EMMCTYPE_2GB_OR_LOWER;
    DBG_DIRECT("pEmmcDb->CardType: %d", pEmmcDb->CardType);

    Res = Cmd2_AllSendCid(SDHCx, &R2Rsp);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }

    Res = Cmd3_SetRelativeAddr(SDHCx, &R1Rsp);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }

    Res = Cmd9_SendCsd(SDHCx, &R2Rsp);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }
    memcpy(pEmmcDb->aCsd, &R2Rsp, CSD_CID_BYTES);

    Res = Cmd7_SelectCard(SDHCx, &R1Rsp);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }

    Res = Cmd16_SetBlockLen(SDHCx, BYTES_PER_BLOCK, &R1Rsp);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }

    SDHC_SetClkOutFreq(SDHCx, MIN2(NORMAL_SPEED_MODE_CLK_FREQ_kHz, pParm->ClkOutFreq_kHz));

    Res = Cmd8_SendExtCsd(SDHCx, &R1Rsp, pExtCsdBuf);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }
    memcpy(&pEmmcDb->SectorCountInExtCsd, &pExtCsdBuf[212], 4);
    DBG_DIRECT("pEmmcDb->SectorCountInExtCsd: %d", pEmmcDb->SectorCountInExtCsd);

    static const Cmd6Arg_t SetHighSpeedArg = {.b = {
            .Access = CMD6ACEESS_WRITE_BTYES,
            .Index = 185,
            .Value = 1,
        }
    };
    Res = Cmd6_Switch(SDHCx, SetHighSpeedArg, &R1Rsp);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }
    SDHC_SetClkOutFreq(SDHCx, MIN2(HIGH_SPEED_MODE_CLK_FREQ_kHz, pParm->ClkOutFreq_kHz));

    static const uint8_t aDataWidthValue[] = {[DATAWIDTH_1BIT] = 0, 1, 2, 5, 6};
    const Cmd6Arg_t SetDataWidthArg = {.b = {
            .Access = CMD6ACEESS_WRITE_BTYES,
            .Index = 183,
            .Value = aDataWidthValue[pParm->DataWidth],
        }
    };
    Res = Cmd6_Switch(SDHCx, SetDataWidthArg, &R1Rsp);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }
    SDHC_SetHostDataWidth(SDHCx, pParm->DataWidth);

    Res = RepeatCmd13UntillIntoXferState(SDHCx, &R1Rsp, 1, 1000);
    if (Res != SDEMMCRES_OK)
    {
        goto Exit;
    }

Exit:
    os_mem_free(pExtCsdBuf);
    return Res;
}

/**
  * @brief  EMMC read data.
  * @param  SDHCx: Specifies the SDHC peripheral.
  * @param  StartBlock:  Start block.
  * @param  BlockCnt:  Block count.
  * @param  pBuf: Pointer to a read buffer.
  * @return SdEmmcRes: Please refer to SdEmmcRes_t for more details.
  */
SdEmmcRes_t Emmc_Read(SDHC_TypeDef *SDHCx, uint32_t StartBlock, uint32_t BlockCnt, void *pBuf)
{
    uint8_t *puBuf = pBuf;
    uint32_t BlockAddr = StartBlock;
    uint32_t RemainBlock = BlockCnt;

    while (RemainBlock > 0)
    {
        uint32_t BlockCntSend = MIN2(MAX_BLOCK_PER_XFER, RemainBlock);

        const CmdInfo_t Cmd18 =
        {
            .CmdIdx = MMC_READ_MULTIPLE_BLOCK,
            .CmdArg = (GetEmmcDb(SDHCx)->CardType == EMMCTYPE_2GB_HIGHER) ? BlockAddr : BlockAddr * BYTES_PER_BLOCK,
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
  * @brief  EMMC write data.
  * @param  SDHCx: Specifies the SDHC peripheral.
  * @param  StartBlock:  Start block.
  * @param  BlockCnt:  Block count.
  * @param  pBuf: Pointer to a write buffer.
  * @return SdEmmcRes: Please refer to SdEmmcRes_t for more details.
  */
SdEmmcRes_t Emmc_Write(SDHC_TypeDef *SDHCx, uint32_t StartBlock, uint32_t BlockCnt,
                       const void *pBuf)
{
    const uint8_t *puBuf = pBuf;
    uint32_t BlockAddr = StartBlock;
    uint32_t RemainBlock = BlockCnt;

    while (RemainBlock > 0)
    {
        uint32_t BlockCntSend = MIN2(MAX_BLOCK_PER_XFER, RemainBlock);

        const CmdInfo_t Cmd25 =
        {
            .CmdIdx = MMC_WRITE_MULTIPLE_BLOCK,
            .CmdArg = (GetEmmcDb(SDHCx)->CardType == EMMCTYPE_2GB_HIGHER) ? BlockAddr : BlockAddr * BYTES_PER_BLOCK,
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
uint32_t Emmc_GetBlockCnt(SDHC_TypeDef *SDHCx)
{
    EmmcDb_t *pEmmcDb = GetEmmcDb(SDHCx);

    if (pEmmcDb->CardType == EMMCTYPE_2GB_HIGHER)
    {
        return pEmmcDb->SectorCountInExtCsd;
    }
    else
    {
        uint32_t CSize = BitsToU32(pEmmcDb->aCsd, 62, 73);
        uint32_t CSizeMult = BitsToU32(pEmmcDb->aCsd, 47, 49);
        uint32_t ReadBlLen = BitsToU32(pEmmcDb->aCsd, 80, 83);

        uint32_t Mult = 1UL << (CSizeMult + 2);
        uint32_t BlockLen = 1UL << ReadBlLen;
        uint32_t Blocknr = (CSize + 1) * Mult;

        uint32_t Capacity_Byte = Blocknr * BlockLen;

        return Capacity_Byte / BYTES_PER_BLOCK;
    }
}


static SdEmmcRes_t Cmd0_GoIdleState(SDHC_TypeDef *SDHCx)
{
    static const CmdInfo_t Cmd0 =
    {
        .CmdIdx = MMC_GO_IDLE_STATE,
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

static SdEmmcRes_t Cmd1_SendOpCond(SDHC_TypeDef *SDHCx, R3Rsp_t *pR3Rsp)
{
    static const CmdInfo_t Cmd1 =
    {
        .CmdIdx = MMC_SEND_OP_COND,
        .CmdArg = 0xc0ff8080,
        .IsResetCmd = false,
        .IsStopCmd = false,
        .IsRspExpected = true,
        .IsR2Rsp = false,
        .CheckRspCrc = false,
    };
    SDHCRes_t Res = SDHC_SendNoDataCmd(SDHCx, &Cmd1, pR3Rsp);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_CMD1_ERROR;
    }
    return SDEMMCRES_OK;
}

static SdEmmcRes_t RepeatCmd1UntilNotBusy(SDHC_TypeDef *SDHCx, R3Rsp_t *pR3Rsp,
                                          uint32_t Interval_ms,
                                          uint32_t MaxRepeatCnt)
{
    for (uint32_t i = 0; i < MaxRepeatCnt; ++i)
    {
        os_delay(Interval_ms);

        SdEmmcRes_t Res = Cmd1_SendOpCond(SDHCx, pR3Rsp);
        if (Res != SDEMMCRES_OK)
        {
            return Res;
        }
        if (pR3Rsp->Busy == R3RSP_NOT_BUSY)
        {
            return SDEMMCRES_OK;
        }
    }

    return SDEMMCRES_CMD1_TIMEOUT;
}

static SdEmmcRes_t Cmd2_AllSendCid(SDHC_TypeDef *SDHCx, R2Rsp_t *pR2Rsp)
{
    static const CmdInfo_t Cmd2 =
    {
        .CmdIdx = MMC_ALL_SEND_CID,
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

static SdEmmcRes_t Cmd3_SetRelativeAddr(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp)
{
    static const CmdInfo_t Cmd3 =
    {
        .CmdIdx = MMC_SET_RELATIVE_ADDR,
        .CmdArg = RCA << 16,
                      .IsResetCmd = false,
                      .IsStopCmd = false,
                      .IsRspExpected = true,
                      .IsR2Rsp = false,
                      .CheckRspCrc = true,
    };
    SDHCRes_t Res = SDHC_SendNoDataCmd(SDHCx, &Cmd3, pR1Rsp);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_CMD3_ERROR;
    }
    return SDEMMCRES_OK;
}

static SdEmmcRes_t Cmd9_SendCsd(SDHC_TypeDef *SDHCx, R2Rsp_t *pR2Rsp)
{
    static const CmdInfo_t Cmd9 =
    {
        .CmdIdx = MMC_SEND_CSD,
        .CmdArg = RCA << 16,
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

static SdEmmcRes_t Cmd7_SelectCard(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp)
{
    static const CmdInfo_t Cmd7 =
    {
        .CmdIdx = MMC_SELECT_CARD,
        .CmdArg = RCA << 16,
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

static SdEmmcRes_t Cmd16_SetBlockLen(SDHC_TypeDef *SDHCx, uint32_t BlockLen, R1Rsp_t *pR1Rsp)
{
    const CmdInfo_t Cmd16 =
    {
        .CmdIdx = MMC_SET_BLOCKLEN,
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

static SdEmmcRes_t Cmd13_SendStatus(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp)
{
    static const CmdInfo_t Cmd13 =
    {
        .CmdIdx = MMC_SEND_STATUS,
        .CmdArg = RCA << 16,
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

static SdEmmcRes_t Cmd8_SendExtCsd(SDHC_TypeDef *SDHCx, R1Rsp_t *pR1Rsp, void *pExtCsd)
{
    static const CmdInfo_t Cmd8 =
    {
        .CmdIdx = MMC_SEND_EXT_CSD,
        .CmdArg = 0,
        .IsResetCmd = false,
        .IsStopCmd = false,
        .IsRspExpected = true,
        .IsR2Rsp = false,
        .CheckRspCrc = true,
    };
    static const DataInfo_t DataInfo =
    {
        .BlockSize = 512,
        .BlockCount = 1,
        .SendAutoStop = false,
    };
    SDHCRes_t Res = SDHC_SendCmdWithRxData(SDHCx, &Cmd8, pR1Rsp, &DataInfo, pExtCsd);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_CMD8_ERROR;
    }
    return SDEMMCRES_OK;
}

static SdEmmcRes_t Cmd6_Switch(SDHC_TypeDef *SDHCx, Cmd6Arg_t Cmd6Arg, R1Rsp_t *pR1Rsp)
{
    const CmdInfo_t Cmd6 =
    {
        .CmdIdx = MMC_SWITCH,
        .CmdArg = Cmd6Arg.d32,
        .IsResetCmd = false,
        .IsStopCmd = false,
        .IsRspExpected = true,
        .IsR2Rsp = false,
        .CheckRspCrc = true,
    };
    SDHCRes_t Res = SDHC_SendNoDataCmd(SDHCx, &Cmd6, pR1Rsp);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_CMD6_ERROR;
    }

    Res = SDHC_WaitData0Idle(SDHCx, 2000);
    if (Res != SDHCRES_OK)
    {
        return SDEMMCRES_CMD6_TIMEOUT;
    }

    return SDEMMCRES_OK;
}
