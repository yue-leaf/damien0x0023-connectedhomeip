/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef RTL_ETHERNET_H_
#define RTL_ETHERNET_H_

#ifdef  __cplusplus
extern "C"
{
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "utils/rtl_utils.h"
#if defined (CONFIG_SOC_SERIES_RTL87X2G)
#include "ethernet/src/rtl87x2g/rtl_ethernet_def.h"
#include "ethernet/src/rtl87x2g/rtl_ethernet_cpu_reg.h"
#endif

/*============================================================================*
 *                          Private Macros
 *============================================================================*/
//#define ETH_PRINT_LOG
#ifdef ETH_PRINT_LOG
#define ETH_DBG_BUFFER(MODULE, LEVEL, fmt, para_num,...) DBG_BUFFER_##LEVEL(TYPE_BEE4, SUBTYPE_FORMAT, MODULE, fmt, para_num, ##__VA_ARGS__)
#else
#define ETH_DBG_BUFFER(MODULE, LEVEL, fmt, para_num,...) ((void)0)
#endif

/// Defines the MAC address length
#define MAC_ADDR_LEN                    6

/// the size (unit: Bytes) of each Tx descriptor
#define ETH_TX_DESC_SIZE               20  // equal to sizeof(ETH_TxDescTypeDef)
/// the size (unit: Bytes) of each Rx descriptor
#define ETH_RX_DESC_SIZE               16  // equal to sizeof(ETH_RxDescTypeDef)

/// Defines the max. timeout value when checking the flag of MDIO operations
#define ETH_TIMEOUT_CNT_MAX             1000000

/// Defines the PHY address
#define ETH_PHY_ADDR                    0x1

/// Defines the address of PHY RTL8201FR register 0
#define ETH_PHY_REG0_ADDR               0x0
/// Defines the address of PHY RTL8201FR register 1
#define ETH_PHY_REG1_ADDR               0x1
/// Defines the address of PHY RTL8201FR register 16
#define ETH_PHY_REG16_ADDR              0x10
/// Defines the address of PHY RTL8201FR register 31
#define ETH_PHY_REG31_ADDR              0x1F
/// Defines the default value when reading PHY's register
#define ETH_PHY_REG_DEFAULT_VALUE       0x0

/// Defines the bit definition of PHY RTL8201FR register 0
#define ETH_PHY_SPEED_MSB               BIT6
#define ETH_PHY_DUPLEX_MODE             BIT8
#define ETH_PHY_RESTART_NWAY            BIT9
#define ETH_PHY_NWAY_EN                 BIT12
#define ETH_PHY_SPEED_LSB               BIT13
#define ETH_PHY_SW_RESET                BIT15

/// Defines the bit definition of PHY RTL8201FR register 1
#define ETH_PHY_LINK_STATUS             BIT2

/// Defines the header of vlantag (ctag)
//#define ETH_C_VLAN_HDR                    0x8100279F

#define ETH_CPU_ETH_0X130_DEFAULT_VALUE 0x01010100UL
#define ETH_CPU_ETH_0X138_DEFAULT_VALUE 0x31000000UL
#define ETH_CPU_ETH_0X134_DEFAULT_VALUE 0x40081000UL

//bit definition of tx descriptor status register
#define ETH_TX_DESC_OWN                 BIT31
#define ETH_TX_DESC_EOR                 BIT30
#define ETH_TX_DESC_FS                  BIT29
#define ETH_TX_DESC_LS                  BIT28
#define ETH_TX_DESC_CRC                 BIT23

//bit definition of rx descriptor status register
#define ETH_RX_DESC_OWN                 BIT31
#define ETH_RX_DESC_EOR                 BIT30
#define ETH_RX_DESC_FS                  BIT29
#define ETH_RX_DESC_LS                  BIT28

/** \defgroup ETHERNET        ETHERNET
  * \brief
  * \{
  */
/*============================================================================*
 *                         Constants
 *============================================================================*/
/** \defgroup ETH_Exported_Constants ETH Exported Constants
  * \brief
  * \{
  */

typedef enum
{
    ETH_LINK_UP = 0,
    ETH_LINK_DOWN = 1
} ETH_Link_Status;

typedef enum
{
    ETH_HALF_DUPLEX = 0,
    ETH_FULL_DUPLEX = 1
} ETH_Duplex_Mode;

typedef enum
{
    ETH_SPEED_100 = 0,
    ETH_SPEED_10 = 1
} ETH_Link_Speed_Type;

typedef enum
{
    ETH_STATUS_OK,
    ETH_STATUS_ERROR
} ETH_Status;

typedef enum
{
    PHY_REG_READ_MODE = 0x00,
    PHY_REG_WRITE_MODE = 0x01,
} PHY_Reg_Operation_Mode;

typedef enum
{
    ETH_TX_CHANGE_ON_FALLING_EDGE = 0x00,
    ETH_TX_CHANGE_ON_RISING_EDGE = 0x01,
} ETH_Tx_Refclk_Phase_TypeDef;

typedef enum
{
    ETH_RX_SAMPLE_ON_FALLING_EDGE = 0x0,
    ETH_RX_SAMPLE_ON_RISING_EDGE = 0x1,
} ETH_Rx_Refclk_Phase_TypeDef;

typedef enum
{
    ETH_PHY_NORMAL_MODE = 0x0,
    ETH_PHY_LOOPBACK_MODE = 0x1,
} ETH_Phy_Mode_TypeDef;

typedef enum
{
    ETH_NORMAL_MODE = 0,
    ETH_LOOPBACK_R2T_MODE = 1,
    ETH_LOOPBACK_T2R_MODE = 3,
} ETH_Mode_TypeDef;

typedef enum
{
    ETH_IFG_0 = 0,
    ETH_IFG_1 = 1,
    ETH_IFG_2 = 2,
    ETH_IFG_3 = 3,// 9.6 us for 10 Mbps, 960 ns for 100 Mbps
    ETH_IFG_4 = 4,
    ETH_IFG_5 = 5,
    ETH_IFG_6 = 6,
    ETH_IFG_7 = 7,
} ETH_Inter_Frame_Gap_Time_TypeDef;

typedef enum
{
    ETH_RX_CONFIG_AAP = BIT0,
    ETH_RX_CONFIG_APM = BIT1,
    ETH_RX_CONFIG_AM = BIT2,
    ETH_RX_CONFIG_AB = BIT3,
    ETH_RX_CONFIG_AR = BIT4,
    ETH_RX_CONFIG_AER = BIT5,
    ETH_RX_CONFIG_AFC = BIT6,
    ETH_RX_CONFIG_HOME_PNA = BIT7,
} ETH_Rx_Config_TypeDef;

typedef enum
{
    ETH_RX_JUMBO_DISABLE = 0x0,
    ETH_RX_JUMBO_ENABLE = 0x1,
} ETH_Rx_Jumbo_Config_TypeDef;

typedef enum
{
    ETH_TX_THRESHOLD_128B = 0,
    ETH_TX_THRESHOLD_256B = 1,
    ETH_TX_THRESHOLD_512B = 2,
    ETH_TX_THRESHOLD_1024B = 3,
} ETH_Tx_Threshold_TypeDef;

typedef enum
{
    ETH_RX_THRESHOLD_1024B = 0,
    ETH_RX_THRESHOLD_128B = 1,
    ETH_RX_THRESHOLD_256B = 2,
    ETH_RX_THRESHOLD_512B = 3,
} ETH_Rx_Threshold_TypeDef;

typedef enum
{
    ETH_TX_TRIGGER_LEVEL_1_PKT = 0,
    ETH_TX_TRIGGER_LEVEL_4_PKTS = 1,
    ETH_TX_TRIGGER_LEVEL_8_PKTS = 2,
    ETH_TX_TRIGGER_LEVEL_12_PKTS = 3,
    ETH_TX_TRIGGER_LEVEL_16_PKTS = 4,
    ETH_TX_TRIGGER_LEVEL_20_PKTS = 5,
    ETH_TX_TRIGGER_LEVEL_24_PKTS = 6,
    ETH_TX_TRIGGER_LEVEL_28_PKTS = 7,
} ETH_Tx_Trigger_Level_TypeDef;

typedef enum
{
    ETH_RX_TRIGGER_LEVEL_1_PKT = 0,
    ETH_RX_TRIGGER_LEVEL_4_PKTS = 1,
    ETH_RX_TRIGGER_LEVEL_8_PKTS = 2,
    ETH_RX_TRIGGER_LEVEL_12_PKTS = 3,
    ETH_RX_TRIGGER_LEVEL_16_PKTS = 4,
    ETH_RX_TRIGGER_LEVEL_20_PKTS = 5,
    ETH_RX_TRIGGER_LEVEL_24_PKTS = 6,
    ETH_RX_TRIGGER_LEVEL_28_PKTS = 7,
} ETH_Rx_Trigger_Level_TypeDef;

//RTL8201FR TX Setup time
typedef enum
{
    ETH_PHY_TX_SETUP_TIME_6NS = 0x6,
    ETH_PHY_TX_SETUP_TIME_8NS = 0x5,
    ETH_PHY_TX_SETUP_TIME_10NS = 0x4, //default 10ns_0x1111
    ETH_PHY_TX_SETUP_TIME_12NS = 0x3,
    ETH_PHY_TX_SETUP_TIME_14NS = 0x2,
    ETH_PHY_TX_SETUP_TIME_16NS = 0x1,
    ETH_PHY_TX_SETUP_TIME_18NS = 0x0,
} ETH_Phy_Tx_Setup_Time_TypeDef;

//RTL8201FR RX Setup time
typedef enum
{
    ETH_PHY_RX_SETUP_TIME_8NS = 0x8,
    ETH_PHY_RX_SETUP_TIME_10NS = 0x9, //default 10ns_0x1111
    ETH_PHY_RX_SETUP_TIME_12NS = 0x6,
    ETH_PHY_RX_SETUP_TIME_14NS = 0x7,
    ETH_PHY_RX_SETUP_TIME_16NS = 0x4,
    ETH_PHY_RX_SETUP_TIME_18NS = 0x5,
} ETH_Phy_Rx_Setup_Time_TypeDef;

typedef enum
{
    ETH_ISR_ROK = BIT0,
    ETH_ISR_CNT_WRAP = BIT1,
    ETH_ISR_RER_RUNT = BIT2,
    ETH_ISR_RER_OVF = BIT4,
    ETH_ISR_RDU = BIT5,
    ETH_ISR_TOK = BIT6,
    ETH_ISR_TER = BIT7,
    ETH_ISR_LINK_CHANGE = BIT8,
    ETH_ISR_TDU = BIT9,
    ETH_ISR_SW_INT = BIT10,
    ETH_ISR_RDU2 = BIT11,
    ETH_ISR_RDU3 = BIT12,
    ETH_ISR_RDU4 = BIT13,
    ETH_ISR_RDU5 = BIT14,
    ETH_ISR_RDU6 = BIT15,
} ETH_Interrupt_Status_TypeDef;

typedef enum
{
    ETH_IMR_ROK = BIT16,
    ETH_IMR_CNT_WRAP = BIT17,
    ETH_IMR_RER_RUNT = BIT18,
    ETH_IMR_RER_OVF = BIT20,
    ETH_IMR_RDU = BIT21,
    ETH_IMR_TOK = BIT22,
    ETH_IMR_TER = BIT23,
    ETH_IMR_LINK_CHANGE = BIT24,
    ETH_IMR_TDU = BIT25,
    ETH_IMR_SW_INT = BIT26,
    ETH_IMR_RDU2 = BIT27,
    ETH_IMR_RDU3 = BIT28,
    ETH_IMR_RDU4 = BIT29,
    ETH_IMR_RDU5 = BIT30,
    ETH_IMR_RDU6 = BIT31,
} ETH_Interrupt_Mask_TypeDef;

/**
  \brief  The structure of Tx descriptor.
*/
typedef struct
{
    uint32_t dw1;    // offset 0
    uint32_t addr;   // offset 4
    uint32_t dw2;    // offset 8
    uint32_t dw3;    // offset 12
    uint32_t dw4;    // offset 16
} ETH_TxDescTypeDef;

/**
  \brief  The structure of Rx descriptor.
*/
typedef struct
{
    uint32_t dw1;    // offset 0
    uint32_t addr;   // offset 4
    uint32_t dw2;    // offset 8
    uint32_t dw3;    // offset 12
} ETH_RxDescTypeDef;

/** End of ETH_Exported_Constants
  * \}
  */

/*============================================================================*
 *                         Types
 *============================================================================*/
/** \defgroup ETH_Exported_Types ETH Exported Types
  * \brief
  * \{
  */

/**
 * \brief       ETH init structure definition.
 *
 * \ingroup     ETH_Exported_Types
 */
typedef struct
{
    ETH_Tx_Refclk_Phase_TypeDef ETH_TxRefclkPhase;
    ETH_Rx_Refclk_Phase_TypeDef ETH_RxRefclkPhase;
    ETH_Phy_Mode_TypeDef ETH_PhyMode;
    ETH_Mode_TypeDef ETH_Mode;
    ETH_Inter_Frame_Gap_Time_TypeDef ETH_InterFrameGapTime;
    uint8_t ETH_MacAddr[6];
    uint8_t ETH_ReceiveConfig;
    ETH_Rx_Jumbo_Config_TypeDef ETH_RxJumboConfig;
    ETH_Tx_Threshold_TypeDef ETH_TxThreshold;
    ETH_Rx_Threshold_TypeDef ETH_RxThreshold;
    ETH_Tx_Trigger_Level_TypeDef ETH_TxTriggerLevel;
    ETH_Rx_Trigger_Level_TypeDef ETH_RxTriggerLevel;
    ETH_Phy_Tx_Setup_Time_TypeDef ETH_PhyTxSetupTime;
    ETH_Phy_Rx_Setup_Time_TypeDef ETH_PhyRxSetupTime;
    uint8_t ETH_TxDescNum;
    uint8_t ETH_RxDescNum;
    uint32_t ETH_IntMaskAndStatus;
    volatile ETH_TxDescTypeDef *ETH_TxDesc;
    volatile ETH_RxDescTypeDef *ETH_RxDesc;
    volatile uint8_t *ETH_TxPktBuf;
    volatile uint8_t *ETH_RxPktBuf;
    uint8_t ETH_TxDescCurrentNum;
    uint8_t ETH_RxDescCurrentNum;
    uint8_t ETH_RxFrameStartDescIdx;
    uint32_t ETH_RxFrameLen;
    uint32_t ETH_RxSegmentCount;
    uint16_t ETH_TxAllocBufSize;
    uint16_t ETH_RxAllocBufSize;
    uint16_t ETH_TxBufSize;
    uint16_t ETH_RxBufSize;
} ETH_InitTypeDef;

/** End of ETH_Exported_Types
  * \}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** \defgroup ETH_Exported_Functions ETH Exported Functions
  * \brief
  * \{
  */

/**
  * \brief  Set the Tx/Rx descriptor number.
  * \param[in]  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \param[in]  tx_desc_no: The specified Tx descriptor number.
  * \param[in]  rx_desc_no: The specified Rx descriptor number.
  * \return None.
  */
void ETH_SetDescNum(ETH_InitTypeDef *ETH_InitStruct, uint8_t tx_desc_no, uint8_t rx_desc_no);

/**
  * \brief  Set the start address of Tx/Rx descriptor ring.
  * \param[in]  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \param[in]  tx_desc: The start address of Tx descriptor ring.
  * \param[in]  rx_desc: The start address of Rx descriptor ring.
  * \return None.
  */
void ETH_SetDescAddr(ETH_InitTypeDef *ETH_InitStruct, uint8_t *tx_desc, uint8_t *rx_desc);

/**
  * \brief  Set the start address of Tx/Rx packet buffer.
  * \param[in]  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \param[in]  tx_pkt_buf: The start address of Tx packet buffer.
  * \param[in]  rx_pkt_buf: The start address of Rx packet buffer.
  * \return None.
  */
void ETH_SetPktBuf(ETH_InitTypeDef *ETH_InitStruct, uint8_t *tx_pkt_buf, uint8_t *rx_pkt_buf);

/**
  * \brief  Set the ethernet MAC address.
  * \param[in]  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \param[in]  addr: The specified MAC address.
  * \return None.
  */
void ETH_SetMacAddr(ETH_InitTypeDef *ETH_InitStruct, uint8_t *addr);

/**
  * \brief  Get the ethernet MAC address.
  * \param[in]  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \param[in]  addr: The buffer of MAC address.
  * \return None.
  */
void ETH_GetMacAddr(ETH_InitTypeDef *ETH_InitStruct, uint8_t *addr);

/**
  * \brief  Set buffer size.
  * \param[in]  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \param[in]  tx_alloc_buf_size: Allocated tx buffer size.
  * \param[in]  rx_alloc_buf_size: Allocated rx buffer size.
  * \param[in]  tx_buf_size: The tx buffer size that can be used actually.
  * \param[in]  rx_buf_size: The rx buffer size that can be used actually.
  * \return None.
  */
void ETH_SetBufSize(ETH_InitTypeDef *ETH_InitStruct, uint16_t tx_alloc_buf_size,
                    uint16_t rx_alloc_buf_size, uint16_t tx_buf_size, uint16_t rx_buf_size);

/**
  * @brief  Init ETH clock.
  * @param[in]  None.
  * @return None.
  */
void ETH_ClkInit(void);

/**
  * \brief  Initialize the ethernet MAC controller and PHY RTL8201FR.
  * \param[in]  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \return None.
  */
void ETH_Init(ETH_InitTypeDef *ETH_InitStruct);

/**
  * \brief  Initialize ETH_InitTypeDef.
  * \param[in]  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \return None.
  */
void ETH_StructInit(ETH_InitTypeDef *ETH_InitStruct);

/**
  * \brief  Close the clock of ETH and disable intterrupt.
  * \param[in]  None.
  * \return None.
  */
void ETH_DeInit(void);

/**
  * \brief  Enable ethernet RX.
  * \param[in]  None.
  * \return None.
  */
void ETH_EnableRx(void);

/**
  * \brief  To send frame.
  * \param[in]  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \param[in]  FrameLength: The length of data to be sent.
  * \return The result of the operation.
  */
ETH_Status ETH_SendFrame(ETH_InitTypeDef *ETH_InitStruct, uint32_t FrameLength);

/**
  * \brief  To receive frame.
  * \param[in]  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \return receive frame result.
  */
ETH_Status ETH_ReceiveFrame(ETH_InitTypeDef *ETH_InitStruct);

/** End of ETH_Exported_Functions
  * \}
  */

/** End of ETH
  * \}
  */

#ifdef  __cplusplus
}
#endif

#endif /* RTL_ETHERNET_H */


