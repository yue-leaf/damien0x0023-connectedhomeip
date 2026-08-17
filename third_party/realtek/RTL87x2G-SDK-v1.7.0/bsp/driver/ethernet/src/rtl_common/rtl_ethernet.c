/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "rtl_ethernet.h"
#include "rtl_rcc.h"
#include "app_section.h"
#include "clock.h"
#include "utils.h"
#include "trace.h"
#include "string.h"

/*============================================================================*
 *                           Public Functions
 *============================================================================*/
/**
  * \brief  Handle PHY register.
  * \param  operation: read or write. The value can be PHY_REG_READ_MODE or PHY_REG_WRITE_MODE.
  * \param  address: PHY's register address.
  * \param  data: The data to be written to the above register address.
  * \return The result of the operation or the read register data.
  */
uint16_t ETH_HandlePHYRegister(uint8_t operation, uint8_t address, uint16_t data)
{
    uint32_t i = 0;

    if (address > ETH_PHY_REG31_ADDR)
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "[ETH_HandlePHYRegister] Invalid parameter!!", 0);
        return ETH_STATUS_ERROR;
    }

    platform_delay_us(1);
    ETH->ETH_MIIAR.d32 = (operation << 31) | (ETH_PHY_ADDR << 26) | (address << 16) | data;

    if (PHY_REG_WRITE_MODE == operation)
    {
        while (ETH->ETH_MIIAR.b.flag)
        {
            platform_delay_us(1);
            i++;
            if (i > ETH_TIMEOUT_CNT_MAX)
            {
                ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR,
                               "[ETH_HandlePHYRegister] Wait write operation flag timeout!!", 0);
                break;
            }
        }

        return ETH_STATUS_OK;
    }
    else
    {
        while (ETH->ETH_MIIAR.b.flag == 0)
        {
            platform_delay_us(1);
            i++;
            if (i > ETH_TIMEOUT_CNT_MAX)
            {
                ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR,
                               "[ETH_HandlePHYRegister] Wait read operation flag timeout!!", 0);
                break;
            }
        }

        return (uint16_t)((ETH->ETH_MIIAR.d32) & 0xFFFF);
    }
}

/**
  * \brief  Set the Tx/Rx descriptor number.
  * \param  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \param  tx_desc_no: The specified Tx descriptor number.
  * \param  rx_desc_no: The specified Rx descriptor number.
  * \return None.
  */
void ETH_SetDescNum(ETH_InitTypeDef *ETH_InitStruct, uint8_t tx_desc_no, uint8_t rx_desc_no)
{
    if ((ETH_InitStruct == NULL) || (tx_desc_no == 0) || (rx_desc_no == 0))
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "[ETH_SetDescNum] Invalid parameter!!", 0);
        return;
    }
    if (((tx_desc_no * sizeof(ETH_TxDescTypeDef)) % 32) ||
        ((rx_desc_no * sizeof(ETH_RxDescTypeDef)) % 32))
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR,
                       "[ETH_SetDescNum] The size of Tx/Rx descriptor ring must be 32-Byte alignment!!", 0);
        return;
    }

    ETH_InitStruct->ETH_TxDescNum = tx_desc_no;
    ETH_InitStruct->ETH_RxDescNum = rx_desc_no;

    ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_SetDescNum] ETH_TxDescNum=%d,ETH_RxDescNum=%d", 2,
                   ETH_InitStruct->ETH_TxDescNum, ETH_InitStruct->ETH_RxDescNum);
}

/**
  * \brief  Set the start address of Tx/Rx descriptor ring.
  * \param  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \param  tx_desc: The start address of Tx descriptor ring.
  * \param  rx_desc: The start address of Rx descriptor ring.
  * \return None.
  */
void ETH_SetDescAddr(ETH_InitTypeDef *ETH_InitStruct, uint8_t *tx_desc, uint8_t *rx_desc)
{
    if ((ETH_InitStruct == NULL) || (tx_desc == NULL) || (rx_desc == NULL))
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "[ETH_SetDescAddr] Invalid parameter!!", 0);
        return;
    }
    if ((((uint32_t)tx_desc) & 0x1F) || (((uint32_t)rx_desc) & 0x1F))
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR,
                       "[ETH_SetDescAddr] The descriptor address must be 32-Byte alignment!!", 0);
        return;
    }

    ETH_InitStruct->ETH_TxDesc = (ETH_TxDescTypeDef *)tx_desc;
    ETH_InitStruct->ETH_RxDesc = (ETH_RxDescTypeDef *)rx_desc;
}

/**
  * \brief  Set the start address of Tx/Rx packet buffer.
  * \param  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \param  tx_pkt_buf: The start address of Tx packet buffer.
  * \param  rx_pkt_buf: The start address of Rx packet buffer.
  * \return None.
  */
void ETH_SetPktBuf(ETH_InitTypeDef *ETH_InitStruct, uint8_t *tx_pkt_buf, uint8_t *rx_pkt_buf)
{
    if ((ETH_InitStruct == NULL) || (tx_pkt_buf == NULL) || (rx_pkt_buf == NULL))
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "[ETH_SetPktBuf] Invalid parameter!!", 0);
        return;
    }
    if ((((uint32_t)tx_pkt_buf) & 0x1F) || (((uint32_t)rx_pkt_buf) & 0x1F))
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR,
                       "[ETH_SetPktBuf] The packet buffer address must be 32-Byte alignment!!", 0);
        return;
    }

    ETH_InitStruct->ETH_TxPktBuf = tx_pkt_buf;
    ETH_InitStruct->ETH_RxPktBuf = rx_pkt_buf;
}

/**
  * \brief  Set the ethernet MAC address.
  * \param  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \param  addr: The specified MAC address.
  * \return None.
  */
void ETH_SetMacAddr(ETH_InitTypeDef *ETH_InitStruct, uint8_t *addr)
{
    if ((ETH_InitStruct == NULL) || (addr == NULL))
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "[ETH_SetMacAddr] Invalid parameter!!", 0);
        return;
    }

    memcpy((void *)(ETH_InitStruct->ETH_MacAddr), addr, MAC_ADDR_LEN);
}

/**
  * \brief  Get the ethernet MAC address.
  * \param  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \param  addr: The buffer of MAC address.
  * \return None.
  */
void ETH_GetMacAddr(ETH_InitTypeDef *ETH_InitStruct, uint8_t *addr)
{
    if ((ETH_InitStruct == NULL) || (addr == NULL))
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "[ETH_GetMacAddr] Invalid parameter!!", 0);
        return;
    }

    memcpy((void *)addr, ETH_InitStruct->ETH_MacAddr, MAC_ADDR_LEN);
}

/**
  * \brief  Set buffer size.
  * \param  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \param  tx_alloc_buf_size: Allocated tx buffer size.
  * \param  rx_alloc_buf_size: Allocated rx buffer size.
  * \param  tx_buf_size: The tx buffer size that can be used actually.
  * \param  rx_buf_size: The rx buffer size that can be used actually.
  * \return None.
  */
void ETH_SetBufSize(ETH_InitTypeDef *ETH_InitStruct, uint16_t tx_alloc_buf_size,
                    uint16_t rx_alloc_buf_size, uint16_t tx_buf_size, uint16_t rx_buf_size)
{
    if ((ETH_InitStruct == NULL) ||
        ((tx_alloc_buf_size - 8) < tx_buf_size) ||
        ((rx_alloc_buf_size - 8) < rx_buf_size))
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "[ETH_SetBufSize] Invalid parameter!!", 0);
        return;
    }

    ETH_InitStruct->ETH_TxAllocBufSize = tx_alloc_buf_size;
    ETH_InitStruct->ETH_RxAllocBufSize = rx_alloc_buf_size;
    ETH_InitStruct->ETH_TxBufSize = tx_buf_size;
    ETH_InitStruct->ETH_RxBufSize = rx_buf_size;
}

/**
  * @brief  Init ETH clock.
  * @param  None.
  * @return None.
  */
void ETH_ClkInit(void)
{
    ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_ClkInit]", 0);

    pm_ethernet_freq_set(CLK_PLL1_SRC, 125, 125);

    RCC_PeriphClockCmd(APBPeriph_ETH, APBPeriph_ETH_CLOCK, ENABLE);
}

/**
  * \brief  Initialize the ethernet MAC controller and PHY RTL8201FR.
  * \param  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \return None.
  */
void ETH_Init(ETH_InitTypeDef *ETH_InitStruct)
{
    uint32_t i, start_us;
    uint16_t tmp;

    /* reset PHY & disable MAC's auto-polling */
    i = 0;
    ETH->ETH_MIIAR.d32 = 0x84408000;
    while (ETH->ETH_MIIAR.b.flag)
    {
        platform_delay_us(1);
        i++;
        if (i > ETH_TIMEOUT_CNT_MAX)
        {
            ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "[ETH_Init] Wait write operation flag timeout!!", 0);
            break;
        }
    }
    platform_delay_ms(4000);

    /* reset MAC */
    ETH->ETH_CR.b.rst = 1;
    i = 0;
    do
    {
        i++;
        if (i > ETH_TIMEOUT_CNT_MAX)
        {
            ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "[ETH_Init] Wait reset MAC timeout!!", 0);
            break;
        }
    }
    while (ETH->ETH_CR.b.rst);

    /* RMII interface */
    ETH->ETH_MSR.b.reg_rmii2mii_en = 1;
    /* REFCLK on */
    ETH->ETH_MSR.b.refclk_on = 1;
    ETH->ETH_MSR.b.sel_rgmii = 0;

    /* set refclk phase */
    ETH->ETH_MSR.b.rx_refclk_phase = ETH_InitStruct->ETH_RxRefclkPhase;
    ETH->ETH_MSR.b.tx_refclk_phase = ETH_InitStruct->ETH_TxRefclkPhase;
    ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Init] MSR = 0x%x", 1, ETH->ETH_MSR.d32);

    ETH->ETH_TCR.b.inter_frm_gp_tm = ETH_InitStruct->ETH_InterFrameGapTime;
    ETH->ETH_TCR.b.lpbk = ETH_InitStruct->ETH_Mode;

    /* Rx settings */
    ETH->ETH_IDR0.d32 = ((ETH_InitStruct->ETH_MacAddr[0]) << 24) | ((
                                                                        ETH_InitStruct->ETH_MacAddr[1]) << 16) | ((ETH_InitStruct->ETH_MacAddr[2]) << 8) |
                        (ETH_InitStruct->ETH_MacAddr[3]);
    ETH->ETH_IDR4.d32 = ((ETH_InitStruct->ETH_MacAddr[4]) << 24) | ((
                                                                        ETH_InitStruct->ETH_MacAddr[5]) << 16);
    ETH->ETH_RCR.d32 = ETH_InitStruct->ETH_ReceiveConfig;
    ETH->ETH_CR.b.rx_jumbo = ETH_InitStruct->ETH_RxJumboConfig;

    ETH_CPU->ETH_ETHRNTRXCPU_DES_NUM1.d32 = ETH_CPU_ETH_0X130_DEFAULT_VALUE;

    /* I/O command */
    ETH_CPU->ETH_IO_CMD1.d32 = ETH_CPU_ETH_0X138_DEFAULT_VALUE;
    ETH_CPU->ETH_ETHER_IO_CMD.d32 =
        ETH_CPU_ETH_0X134_DEFAULT_VALUE;
    ETH_CPU->ETH_ETHER_IO_CMD.b.tsh = ETH_InitStruct->ETH_TxThreshold;
    ETH_CPU->ETH_ETHER_IO_CMD.b.rxfth = ETH_InitStruct->ETH_RxThreshold;
    ETH_CPU->ETH_ETHER_IO_CMD.b.tx_int_mitigation_2_0 = ETH_InitStruct->ETH_TxTriggerLevel;
    ETH_CPU->ETH_ETHER_IO_CMD.b.rx_int_mitigation_2_0 = ETH_InitStruct->ETH_RxTriggerLevel;

    ETH_CPU->ETH_TXFDP1.d32 = (uint32_t)(ETH_InitStruct->ETH_TxDesc);
    ETH_CPU->ETH_RXFDP1.d32 = (uint32_t)(ETH_InitStruct->ETH_RxDesc);

    /* initialize Tx descriptors */
    for (i = 0; i < (ETH_InitStruct->ETH_TxDescNum); i++)
    {
        if (i == ((ETH_InitStruct->ETH_TxDescNum) - 1))
        {
            ETH_InitStruct->ETH_TxDesc[i].dw1 = ETH_TX_DESC_EOR;
        }
        ETH_InitStruct->ETH_TxDesc[i].addr = (uint32_t)(ETH_InitStruct->ETH_TxPktBuf +
                                                        (i * ETH_InitStruct->ETH_TxAllocBufSize));
//        ETH_InitStruct->ETH_TxDesc[i].dw2 = (eth_vlan_hdr_remove << 25) | (ETH_C_VLAN_HDR & 0xFFFF);
        ETH_InitStruct->ETH_TxDesc[i].dw3 = 0;
        ETH_InitStruct->ETH_TxDesc[i].dw4 = 0;
    }

    /* initialize Rx descriptors */
    for (i = 0; i < (ETH_InitStruct->ETH_RxDescNum); i++)
    {
        if (i == ((ETH_InitStruct->ETH_RxDescNum) - 1))
        {
            ETH_InitStruct->ETH_RxDesc[i].dw1 = ETH_RX_DESC_OWN | ETH_RX_DESC_EOR |
                                                ETH_InitStruct->ETH_RxBufSize;
        }
        else
        {
            ETH_InitStruct->ETH_RxDesc[i].dw1 = ETH_RX_DESC_OWN | ETH_InitStruct->ETH_RxBufSize;
        }
        ETH_InitStruct->ETH_RxDesc[i].addr = (uint32_t)(ETH_InitStruct->ETH_RxPktBuf +
                                                        (i * ETH_InitStruct->ETH_RxAllocBufSize));
        ETH_InitStruct->ETH_RxDesc[i].dw2 = 0;
        ETH_InitStruct->ETH_RxDesc[i].dw3 = 0;
    }

    /* enable Tx & Rx */
    ETH_CPU->ETH_ETHER_IO_CMD.b.te = 1;
    ETH_CPU->ETH_ETHER_IO_CMD.b.re = 1;

    /* isr & imr */
    ETH->ETH_ISR_IMR.d32 = ETH_InitStruct->ETH_IntMaskAndStatus;

    /* enable auto-polling */
    ETH->ETH_MIIAR.b.disable_auto_polling = 0;

    /* Wait PHY's link is up */
    i = 0;
    do
    {
        /* 1st read */
        tmp = ETH_HandlePHYRegister(PHY_REG_READ_MODE, ETH_PHY_REG1_ADDR, ETH_PHY_REG_DEFAULT_VALUE);
        /* 2nd read */
        tmp = ETH_HandlePHYRegister(PHY_REG_READ_MODE, ETH_PHY_REG1_ADDR, ETH_PHY_REG_DEFAULT_VALUE);

        if (tmp & ETH_PHY_LINK_STATUS)
        {
            break;
        }
        else
        {
            i++;
            if (i > ETH_TIMEOUT_CNT_MAX)
            {
                ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "[ETH_Init] Wait PHY's link up timeout!!", 0);
                return;
            }
        }
    }
    while (1);

    /* Get PHY's link info. */
    tmp = ETH_HandlePHYRegister(PHY_REG_READ_MODE, ETH_PHY_REG0_ADDR, ETH_PHY_REG_DEFAULT_VALUE);
    ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Init] reg0 get phy link info = 0x%x", 1, tmp);
    if ((!(tmp & ETH_PHY_SPEED_MSB)) && (tmp & ETH_PHY_SPEED_LSB))
    {
        if (tmp & ETH_PHY_DUPLEX_MODE)
        {
            ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Init] PHY's link info: 100 Mb/s, Full duplex", 0);
        }
        else
        {
            ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Init] PHY's link info: 100 Mb/s, Half duplex", 0);
        }
    }
    else if ((!(tmp & ETH_PHY_SPEED_MSB)) && (!(tmp & ETH_PHY_SPEED_LSB)))
    {
        if (tmp & ETH_PHY_DUPLEX_MODE)
        {
            ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Init] PHY's link info: 10 Mb/s, Full duplex", 0);
        }
        else
        {
            ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Init] PHY's link info: 10 Mb/s, Half duplex", 0);
        }
    }

    /* Wait MAC's link is up */
    i = 0;
    do
    {
        if ((ETH->ETH_MSR.b.link_status) == ETH_LINK_UP)
        {
            break;
        }
        else
        {
            i++;
            if (i > ETH_TIMEOUT_CNT_MAX)
            {
                ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "[ETH_Init] Wait MAC's link up timeout!!", 0);
                return;
            }
        }
    }
    while (1);

    /* Get MAC's link info. */
    ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Init] get mac link info = 0x%x", 1, ETH->ETH_MSR.d32);
    switch ((ETH->ETH_MSR.b.speed_1000 << 1) | (ETH->ETH_MSR.b.speed_10))
    {
    case ETH_SPEED_100:
        if ((ETH->ETH_MSR.b.full_dupreg) == ETH_FULL_DUPLEX)
        {
            ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Init] MAC's link info: 100 Mb/s, Full duplex", 0);
        }
        else
        {
            ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Init] MAC's link info: 100 Mb/s, Half duplex", 0);
        }
        break;
    case ETH_SPEED_10:
        if ((ETH->ETH_MSR.b.full_dupreg) == ETH_FULL_DUPLEX)
        {
            ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Init] MAC's link info: 10 Mb/s, Full duplex", 0);
        }
        else
        {
            ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Init] MAC's link info: 10 Mb/s, Half duplex", 0);
        }
        break;
    default:
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Init] MAC's link speed: %d", 1,
                       ((ETH->ETH_MSR.b.speed_1000 << 1) | (ETH->ETH_MSR.b.speed_10)));
    }

    /* PHY RTL8201FR loopback setting */
    if (ETH_InitStruct->ETH_PhyMode)
    {
        tmp = ETH_HandlePHYRegister(PHY_REG_READ_MODE, ETH_PHY_REG0_ADDR, ETH_PHY_REG_DEFAULT_VALUE);
        ETH_HandlePHYRegister(PHY_REG_WRITE_MODE, ETH_PHY_REG0_ADDR, tmp | BIT14);
    }

    /* set tx/rx timing of PHY RTL8201FR */
    tmp = ETH_HandlePHYRegister(PHY_REG_READ_MODE, ETH_PHY_REG31_ADDR, ETH_PHY_REG_DEFAULT_VALUE);
    ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Init] default page = %d", 1, tmp);
    ETH_HandlePHYRegister(PHY_REG_WRITE_MODE, ETH_PHY_REG31_ADDR, 0x7);
    tmp = ETH_HandlePHYRegister(PHY_REG_READ_MODE, ETH_PHY_REG16_ADDR, ETH_PHY_REG_DEFAULT_VALUE);
    tmp &= ~(0xff0);
    tmp |= (((ETH_InitStruct->ETH_PhyRxSetupTime) << 4) | ((ETH_InitStruct->ETH_PhyTxSetupTime) <<
                                                           8));
    ETH_HandlePHYRegister(PHY_REG_WRITE_MODE, ETH_PHY_REG16_ADDR, tmp);
    tmp = ETH_HandlePHYRegister(PHY_REG_READ_MODE, ETH_PHY_REG16_ADDR, ETH_PHY_REG_DEFAULT_VALUE);
    ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Init] RMSR = 0x%x", 1, tmp);
    ETH_HandlePHYRegister(PHY_REG_WRITE_MODE, ETH_PHY_REG31_ADDR, 0x0);;
    ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Init] MSR = 0x%x", 1, ETH->ETH_MSR.d32);
}

/**
  * \brief  Initialize ETH_InitTypeDef.
  * \param  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \return None.
  */
void ETH_StructInit(ETH_InitTypeDef *ETH_InitStruct)
{
    ETH_InitStruct->ETH_InterFrameGapTime = ETH_IFG_3;
    ETH_InitStruct->ETH_Mode = ETH_NORMAL_MODE;
    ETH_InitStruct->ETH_PhyMode = ETH_PHY_NORMAL_MODE;
    ETH_InitStruct->ETH_PhyRxSetupTime = ETH_PHY_RX_SETUP_TIME_8NS;
    ETH_InitStruct->ETH_PhyTxSetupTime = ETH_PHY_TX_SETUP_TIME_6NS;
    ETH_InitStruct->ETH_ReceiveConfig = ETH_RX_CONFIG_AAP | ETH_RX_CONFIG_APM | ETH_RX_CONFIG_AM |
                                        ETH_RX_CONFIG_AB;
    ETH_InitStruct->ETH_RxJumboConfig = ETH_RX_JUMBO_ENABLE;
    ETH_InitStruct->ETH_RxRefclkPhase = ETH_RX_SAMPLE_ON_FALLING_EDGE;
    ETH_InitStruct->ETH_TxRefclkPhase = ETH_TX_CHANGE_ON_FALLING_EDGE;
    ETH_InitStruct->ETH_RxThreshold = ETH_RX_THRESHOLD_256B;
    ETH_InitStruct->ETH_TxThreshold = ETH_TX_THRESHOLD_256B;
    ETH_InitStruct->ETH_RxTriggerLevel = ETH_RX_TRIGGER_LEVEL_1_PKT;
    ETH_InitStruct->ETH_TxTriggerLevel = ETH_TX_TRIGGER_LEVEL_1_PKT;
    ETH_InitStruct->ETH_RxDescNum = 8;
    ETH_InitStruct->ETH_TxDescNum = 8;
    ETH_InitStruct->ETH_RxDesc = NULL;
    ETH_InitStruct->ETH_TxDesc = NULL;
    ETH_InitStruct->ETH_RxPktBuf = NULL;
    ETH_InitStruct->ETH_TxPktBuf = NULL;
    ETH_InitStruct->ETH_IntMaskAndStatus = ETH_IMR_LINK_CHANGE | ETH_IMR_TOK | ETH_IMR_RER_OVF |
                                           ETH_IMR_ROK | 0xFFFF;
    ETH_InitStruct->ETH_RxDescCurrentNum = 0;
    ETH_InitStruct->ETH_TxDescCurrentNum = 0;
    ETH_InitStruct->ETH_RxFrameStartDescIdx = 0;
    ETH_InitStruct->ETH_RxFrameLen = 0;
    ETH_InitStruct->ETH_RxSegmentCount = 0;
    ETH_InitStruct->ETH_TxAllocBufSize = 1600;
    ETH_InitStruct->ETH_RxAllocBufSize = 1600;
    ETH_InitStruct->ETH_TxBufSize = 1524;
    ETH_InitStruct->ETH_RxBufSize = 1524;

    return;
}

/**
  * \brief  Enable ethernet RX.
  * \param  None.
  * \return None.
  */
void ETH_EnableRx(void)
{
    /* enable Rx ring1 */
    ETH_CPU->ETH_IO_CMD1.b.rxring1 = 1;
}

/**
  * \brief  Close the clock of ETH and disable intterrupt.
  * \param  None.
  * \return None.
  */
void ETH_DeInit(void)
{
    /* disable interrupt & clear all pending interrupts */
    ETH->ETH_ISR_IMR.d32 = 0xFFFF;

    RCC_PeriphClockCmd(APBPeriph_ETH, APBPeriph_ETH_CLOCK, DISABLE);
}

/**
  * \brief  To send frame.
  * \param  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \param  FrameLength: The length of data to be sent.
  * \return The result of the operation.
  */
RAM_FUNCTION
ETH_Status ETH_SendFrame(ETH_InitTypeDef *ETH_InitStruct, uint32_t FrameLength)
{
    if ((ETH_InitStruct == NULL) || (FrameLength == 0))
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "[ETH_SendFrame] Invalid parameter!!", 0);
        return ETH_STATUS_ERROR;
    }

    uint8_t tx_desc_current_num = ETH_InitStruct->ETH_TxDescCurrentNum;
    uint32_t *p = (uint32_t *)(ETH_InitStruct->ETH_TxDesc[tx_desc_current_num].addr);
    uint32_t bufcount = 0, i = 0, size = 0;

    ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_SendFrame] tx_idx=%d", 1, tx_desc_current_num);

    /* check if current Tx descriptor is available */
    if ((((volatile uint32_t)(ETH_InitStruct->ETH_TxDesc[tx_desc_current_num].dw1)) &
         ETH_TX_DESC_OWN) != 0)
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_WARN, "[ETH_SendFrame] Tx descriptor ring is full!!", 0);
        return ETH_STATUS_ERROR;
    }

    ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_SendFrame] frame_length = %d", 1, FrameLength);

    /* Get the number of needed Tx buffers for the current frame */
    if (FrameLength > ETH_InitStruct->ETH_TxBufSize)
    {
        bufcount = FrameLength / ETH_InitStruct->ETH_TxBufSize;
        if (FrameLength % ETH_InitStruct->ETH_TxBufSize)
        {
            bufcount++;
        }
    }
    else
    {
        bufcount = 1U;
    }

    if (bufcount == 1U)
    {
        ETH_InitStruct->ETH_TxDesc[tx_desc_current_num].dw1 &= ETH_TX_DESC_EOR;
        ETH_InitStruct->ETH_TxDesc[tx_desc_current_num].dw1 |= (ETH_TX_DESC_OWN | ETH_TX_DESC_FS |
                                                                ETH_TX_DESC_LS | ETH_TX_DESC_CRC | FrameLength);
        if (tx_desc_current_num == ((ETH_InitStruct->ETH_TxDescNum) - 1))
        {
            ETH_InitStruct->ETH_TxDescCurrentNum = 0;
        }
        else
        {
            ETH_InitStruct->ETH_TxDescCurrentNum++;
        }
    }
    else
    {
        for (i = 0U; i < bufcount; i++)
        {
            ETH_InitStruct->ETH_TxDesc[tx_desc_current_num].dw1 &= ETH_TX_DESC_EOR;
            ETH_InitStruct->ETH_TxDesc[tx_desc_current_num].dw1 |= ETH_TX_DESC_CRC;
            /* Clear FIRST and LAST segment bits */
            ETH_InitStruct->ETH_TxDesc[tx_desc_current_num].dw1 &= ~(ETH_TX_DESC_FS | ETH_TX_DESC_LS);

            if (i == 0U)
            {
                /* Setting the first segment bit */
                ETH_InitStruct->ETH_TxDesc[tx_desc_current_num].dw1 |= ETH_TX_DESC_FS;
            }

            if (i == (bufcount - 1U))
            {
                /* Setting the last segment bit */
                ETH_InitStruct->ETH_TxDesc[tx_desc_current_num].dw1 |= ETH_TX_DESC_LS;
                size = FrameLength - (bufcount - 1U) * ETH_InitStruct->ETH_TxBufSize;
                ETH_InitStruct->ETH_TxDesc[tx_desc_current_num].dw1 |= (size & 0x1ffff);
            }
            else
            {
                /* Program size */
                ETH_InitStruct->ETH_TxDesc[tx_desc_current_num].dw1 |= (ETH_InitStruct->ETH_TxBufSize &
                                                                        0x1ffff);
            }

            /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
            ETH_InitStruct->ETH_TxDesc[tx_desc_current_num].dw1 |= ETH_TX_DESC_OWN;
            /* point to next descriptor */
            if (tx_desc_current_num == ((ETH_InitStruct->ETH_TxDescNum) - 1))
            {
                ETH_InitStruct->ETH_TxDescCurrentNum = 0;
            }
            else
            {
                ETH_InitStruct->ETH_TxDescCurrentNum++;
            }
            tx_desc_current_num = ETH_InitStruct->ETH_TxDescCurrentNum;
        }
    }

    ETH_InitStruct->ETH_IntMaskAndStatus |= ETH_IMR_TOK;
    ETH->ETH_ISR_IMR.b.tok_or_ti |= 1;
    ETH_CPU->ETH_ETHER_IO_CMD.b.tx_fn1st = 1;

    return ETH_STATUS_OK;
}

/**
  * \brief  To receive frame.
  * \param  ETH_InitStruct: The pointer to ETH_InitTypeDef.
  * \return receive frame result.
  */
RAM_FUNCTION
ETH_Status ETH_ReceiveFrame(ETH_InitTypeDef *ETH_InitStruct)
{
    if (ETH_InitStruct == NULL)
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "[ETH_ReceiveFrame] Invalid parameter!!", 0);
        return ETH_STATUS_ERROR;
    }

    uint8_t rx_desc_current_num = ETH_InitStruct->ETH_RxDescCurrentNum;
    uint8_t *buffer;
    bool is_rx_data_received = false;

    ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_ReceiveFrame] rx_idx=%d\n", 1, rx_desc_current_num);

    while ((((volatile uint32_t)(ETH_InitStruct->ETH_RxDesc[rx_desc_current_num].dw1)) &
            ETH_RX_DESC_OWN) == 0)
    {
        is_rx_data_received = true;
        /* Check if last segment */
        if (((ETH_InitStruct->ETH_RxDesc[rx_desc_current_num].dw1 & ETH_RX_DESC_LS) != (uint32_t)RESET))
        {
            /* increment segment count */
            ETH_InitStruct->ETH_RxSegmentCount++;
            ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_ReceiveFrame] end frame, seg_count=%d,frame_len=%d", 2,
                           ETH_InitStruct->ETH_RxSegmentCount, (ETH_InitStruct->ETH_RxDesc[rx_desc_current_num].dw1) & 0xFFF);
            /* Check if last segment is first segment: one segment contains the frame */
            if (ETH_InitStruct->ETH_RxSegmentCount == 1U)
            {
                ETH_InitStruct->ETH_RxFrameStartDescIdx = rx_desc_current_num;
                ETH_InitStruct->ETH_RxFrameLen = 0;
            }

            /* Get the Frame Length of the received packet: substruct 4 bytes of the CRC */
            if (((ETH_InitStruct->ETH_RxDesc[rx_desc_current_num].dw1) & 0xFFF) >= 4)
            {
                ETH_InitStruct->ETH_RxFrameLen += ((ETH_InitStruct->ETH_RxDesc[rx_desc_current_num].dw1) &
                                                   0xFFF) - 4;
            }
            else
            {
                ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_ReceiveFrame] rx data len error", 0);
            }

            /* point to next descriptor */
            if (rx_desc_current_num != ((ETH_InitStruct->ETH_RxDescNum) - 1))
            {
                ETH_InitStruct->ETH_RxDescCurrentNum++;
            }
            else
            {
                ETH_InitStruct->ETH_RxDescCurrentNum = 0;
            }

            /* Return function status */
            return ETH_STATUS_OK;
        }
        /* Check if first segment */
        else if ((ETH_InitStruct->ETH_RxDesc[rx_desc_current_num].dw1 & ETH_RX_DESC_FS) !=
                 (uint32_t)RESET)
        {
            ETH_InitStruct->ETH_RxFrameStartDescIdx = rx_desc_current_num;
            ETH_InitStruct->ETH_RxSegmentCount = 1U;

            ETH_InitStruct->ETH_RxFrameLen = ((ETH_InitStruct->ETH_RxDesc[rx_desc_current_num].dw1) &
                                              0xFFF);

            ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                           "[ETH_ReceiveFrame] start frame frame_len=%d,rx_frame_len=%d", 2,
                           (ETH_InitStruct->ETH_RxDesc[rx_desc_current_num].dw1) & 0xFFF, ETH_InitStruct->ETH_RxFrameLen);

            if (rx_desc_current_num != ((ETH_InitStruct->ETH_RxDescNum) - 1))
            {
                ETH_InitStruct->ETH_RxDescCurrentNum++;
            }
            else
            {
                ETH_InitStruct->ETH_RxDescCurrentNum = 0;
            }
        }
        /* Check if intermediate segment */
        else
        {
            ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                           "[ETH_ReceiveFrame] middle frame frame_len=%d,rx_frame_len=%d", 2,
                           (ETH_InitStruct->ETH_RxDesc[rx_desc_current_num].dw1) & 0xFFF, ETH_InitStruct->ETH_RxFrameLen);
            ETH_InitStruct->ETH_RxSegmentCount++;

            ETH_InitStruct->ETH_RxFrameLen += ((ETH_InitStruct->ETH_RxDesc[rx_desc_current_num].dw1) &
                                               0xFFF);

            if (rx_desc_current_num != ((ETH_InitStruct->ETH_RxDescNum) - 1))
            {
                ETH_InitStruct->ETH_RxDescCurrentNum++;
            }
            else
            {
                ETH_InitStruct->ETH_RxDescCurrentNum = 0;
            }
        }
        rx_desc_current_num = ETH_InitStruct->ETH_RxDescCurrentNum;
    }

    if (is_rx_data_received == false)
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_ReceiveFrame] BIT31=1\n", 0);
        ETH->ETH_ISR_IMR.b.s_rer_ovf = 1;
    }

    return ETH_STATUS_ERROR;
}

