/**
*********************************************************************************************************
*               Copyright(c) 2022, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     ethernet_driver.h
* @brief    The header file of the peripheral ETHERNET driver.
* @details  This file provides all ETHERNET firmware functions.
* @author   mandy
* @date     2022-12-09
* @version  v1.0
* *********************************************************************************************************
*/
#ifndef ETHERNET_DRIVER_H_
#define ETHERNET_DRIVER_H_

#ifdef  __cplusplus
extern "C"
{
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "rtl876x.h"
#include "vector_table.h"
#include "rtl_ethernet.h"

/*============================================================================*
 *                         Macros
 *============================================================================*/
#define MAC_ADDR1 0x9c
#define MAC_ADDR2 0x5a
#define MAC_ADDR3 0x44
#define MAC_ADDR4 0x57
#define MAC_ADDR5 0xdb
#define MAC_ADDR6 0x50

#define ETH_TX_DESC_NUM             8
#define ETH_RX_DESC_NUM             8

#define ETH_TX_BUF_SIZE             1524
#define ETH_RX_BUF_SIZE             1524// 512//
#define ETH_TX_ALLOC_BUF_SIZE       1600
#define ETH_RX_ALLOC_BUF_SIZE       1600//600//
/*============================================================================*
 *                         Types
 *============================================================================*/

/*============================================================================*
*                        Export Global Variables
*============================================================================*/
extern ETH_InitTypeDef ETH_InitStruct;

/*============================================================================*
 *                         Functions
 *============================================================================*/
void eth_init_driver(void);

#ifdef  __cplusplus
}
#endif
#endif /* ETHERNET_DRIVER_H */
/******************* (C) COPYRIGHT 2022 Realtek Semiconductor *****END OF FILE****/
