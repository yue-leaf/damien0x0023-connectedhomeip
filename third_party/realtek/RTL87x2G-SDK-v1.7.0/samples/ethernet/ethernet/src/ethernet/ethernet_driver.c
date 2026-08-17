/**
*********************************************************************************************************
*               Copyright(c) 2022, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     ethernet_driver.c
* @brief    ethernet module driver
* @details
* @author   mandy
* @date     2022-12-13
* @version  v1.0
*********************************************************************************************************
*/

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "board.h"
#include "rtl_ethernet.h"
#include "ethernet_driver.h"
#include <string.h>
#include "rtl876x.h"
#include "vector_table.h"
#include <trace.h>
#include "utils.h"
#include "rtl_rcc.h"
#include <rtl_nvic.h>
#include <rtl_pinmux.h>
#include "sys_arch.h"
#include "os_mem.h"
#include "app_section.h"

/*============================================================================*
 *                              Local Variables
 *============================================================================*/
static uint8_t *pTmpTxDesc = NULL;
static uint8_t *pTmpRxDesc = NULL;
static uint8_t *pTmpTxPktBuf = NULL;
static uint8_t *pTmpRxPktBuf = NULL;

/*============================================================================*
 *                              Global Variables
 *============================================================================*/
ETH_InitTypeDef ETH_InitStruct;

/*============================================================================*
 *                              Functions Declaration
 *============================================================================*/
void ETH_Handler(void) RAM_FUNCTION;

/*============================================================================*
 *                              Local Functions
 *============================================================================*/
/******************************************************************
 * @brief  init eth pad and pinmux
 * @param  none
 * @return none
 */
void eth_init_pad_pinmux(void)
{
    Pad_Config(ETH_TXD0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(ETH_TXD1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(ETH_CRS_DV, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(ETH_RX_ERR, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(ETH_RXD0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(ETH_RXD1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(ETH_REF_CLK, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(ETH_TX_EN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(ETH_MDC, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(ETH_MDIO, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);

    //set AON GPIO_NS HS_MUX[] to 1
    *(uint32_t *)(PAD_REG_BASE + 0x1C) |= BIT27; //P1_7
    *(uint32_t *)(PAD_REG_BASE + 0x3C) |= BIT27; //P3_7
    *(uint32_t *)(PAD_REG_BASE + 0xB4) |= BIT27; //P9_3
    *(uint32_t *)(PAD_REG_BASE + 0xB8) |= (BIT11 | BIT27); //P9_4 P9_5
    *(uint32_t *)(PAD_REG_BASE + 0xBC) |= (BIT11 | BIT27); //P9_6 P9_7
    *(uint32_t *)(PAD_REG_BASE + 0xC0) |= (BIT11 | BIT27); //P10_0 P10_1
    *(uint32_t *)(PAD_REG_BASE + 0xC4) |= BIT11; //P10_2

    //choose Ethernet MAC
    Pinmux_HS_Config(ETHERNET_HS_MUX);
}

/******************************************************************
 * @brief  deinit gamc pad and pinmux
 * @param  none
 * @return none
 */
void eth_deinit_pad_pinmux(void)
{

}

/******************************************************************
 * @brief  init eth related datas
 * @param  none
 * @return none
 */
void eth_init_data(void)
{
    APP_PRINT_INFO0("[eth_init_data] in eth_init_data func");

    //ethernet Tx/Rx buffer must be placed in internal ram
    pTmpTxDesc = (uint8_t *)os_mem_aligned_alloc(RAM_TYPE_DATA_ON, ETH_TX_DESC_NUM * ETH_TX_DESC_SIZE,
                                                 32);
    pTmpRxDesc = (uint8_t *)os_mem_aligned_alloc(RAM_TYPE_DATA_ON, ETH_RX_DESC_NUM * ETH_RX_DESC_SIZE,
                                                 32);
    pTmpTxPktBuf = (uint8_t *)os_mem_aligned_alloc(RAM_TYPE_DATA_ON,
                                                   ETH_TX_DESC_NUM * ETH_TX_ALLOC_BUF_SIZE, 32);
    pTmpRxPktBuf = (uint8_t *)os_mem_aligned_alloc(RAM_TYPE_DATA_ON,
                                                   ETH_RX_DESC_NUM * ETH_RX_ALLOC_BUF_SIZE, 32);

    if (pTmpTxDesc == NULL || pTmpRxDesc == NULL || pTmpTxPktBuf == NULL || pTmpRxPktBuf == NULL)
    {
        APP_PRINT_INFO0("[eth_init_data] TX/RX descriptor malloc fail");
        return;
    }

    memset(pTmpTxDesc, 0, ETH_TX_DESC_NUM * ETH_TX_DESC_SIZE);
    memset(pTmpRxDesc, 0, ETH_RX_DESC_NUM * ETH_RX_DESC_SIZE);
    memset(pTmpTxPktBuf, 0, ETH_TX_DESC_NUM * ETH_TX_ALLOC_BUF_SIZE);
    memset(pTmpRxPktBuf, 0, ETH_RX_DESC_NUM * ETH_RX_ALLOC_BUF_SIZE);
}

/******************************************************************
 * @brief  handle eth interrupt
 * @param  none
 * @return none
 */
void ETH_Handler(void)
{
    uint32_t tmp_status = ETH->ETH_ISR_IMR.d32;
    extern xSemaphoreHandle s_xSemaphore;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if ((tmp_status & ETH_ISR_ROK) && (ETH_InitStruct.ETH_IntMaskAndStatus & ETH_IMR_ROK))
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "[ETH_Handler] ROK", 0);

        xSemaphoreGiveFromISR(s_xSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

        ETH->ETH_ISR_IMR.b.s_rok = 1;
    }

    if (tmp_status & ETH_ISR_RDU)
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "[ETH_Handler] ETH_ISR_RDU", 0);
        xSemaphoreGiveFromISR(s_xSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    if ((tmp_status & ETH_ISR_RER_OVF) && (ETH_InitStruct.ETH_IntMaskAndStatus & ETH_IMR_RER_OVF))
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Handler] RER_OVF", 0);
        ETH->ETH_ISR_IMR.b.s_rer_ovf = 1;
    }

    if ((tmp_status & ETH_ISR_TOK) && (ETH_InitStruct.ETH_IntMaskAndStatus & ETH_IMR_TOK))
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ETH_Handler] TOK", 0);
        ETH_InitStruct.ETH_IntMaskAndStatus &= (~ETH_IMR_TOK);
        ETH->ETH_ISR_IMR.b.tok_or_ti = 0; //close interrupt
        ETH->ETH_ISR_IMR.b.s_tok_or_ti = 1;
    }

    if ((tmp_status & ETH_ISR_LINK_CHANGE) &&
        (ETH_InitStruct.ETH_IntMaskAndStatus & ETH_IMR_LINK_CHANGE))
    {
        APP_PRINT_INFO0("[ETH_Handler] LinkChg");
        if (!(ETH->ETH_MSR.b.link_status))
        {
            APP_PRINT_INFO0("[ETH_Handler] Link up");
        }
        else
        {
            APP_PRINT_INFO0("[ETH_Handler] Link down");
        }
        ETH->ETH_ISR_IMR.b.s_link_chg = 1;
    }

    __DSB();
}

/******************************************************************
 * @brief  config eth nvic
 * @param  none
 * @return none
 */
void eth_config_nvic(void)
{
    NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = ETH_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStruct);
}

/******************************************************************
 * @brief  init eth bsp driver
 * @param  none
 * @return none
 */
void eth_init_bsp_driver(void)
{
    APP_PRINT_INFO0("[eth_init_bsp_driver]");
    ETH_ClkInit();

    //enable nvic
    eth_config_nvic();

    ETH_StructInit(&ETH_InitStruct);

    //user code
    ETH_SetDescNum(&ETH_InitStruct, ETH_TX_DESC_NUM, ETH_RX_DESC_NUM);
    ETH_SetDescAddr(&ETH_InitStruct, pTmpTxDesc, pTmpRxDesc);
    ETH_SetPktBuf(&ETH_InitStruct, pTmpTxPktBuf, pTmpRxPktBuf);
    ETH_SetBufSize(&ETH_InitStruct, ETH_TX_ALLOC_BUF_SIZE, ETH_RX_ALLOC_BUF_SIZE, ETH_TX_BUF_SIZE,
                   ETH_RX_BUF_SIZE);

    ETH_Init(&ETH_InitStruct);
}
/*============================================================================*
*                              Global Functions
*============================================================================*/
/******************************************************************
 * @brief  init ethernet driver
 * @param  none
 * @return none
 */
void eth_init_driver(void)
{
    APP_PRINT_INFO0("[eth_init_driver]");

    eth_init_pad_pinmux();

    eth_init_data();

    //set local mac address
    uint8_t id[6] = {MAC_ADDR1, MAC_ADDR2, MAC_ADDR3, MAC_ADDR4, MAC_ADDR5, MAC_ADDR6};
    ETH_SetMacAddr(&ETH_InitStruct, id);

    eth_init_bsp_driver();
}

/******************* (C) COPYRIGHT 2022 Realtek Semiconductor Corporation *****END OF FILE****/
