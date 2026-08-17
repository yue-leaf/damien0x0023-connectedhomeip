/**
*********************************************************************************************************
*               Copyright(c) 2023, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      board.h
* @brief     header file of ethernet test demo
* @details
* @author    mandy
* @date      2023-4-25
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __BOARD_H__
#define __BOARD_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************
*                 Test Mode Config
*******************************************************/
#define NETCONN_TCP_SERVER_TEST     1
#define NETCONN_TCP_CLIENT_TEST     2
#define NETCONN_UDP_TEST            3
#define NETCONN_JPERF_CLIENT_TEST   4
#define NETCONN_JPERF_SERVER_TEST   5

#define TEST_MODE          NETCONN_TCP_SERVER_TEST

/*******************************************************
*                 IP Addr and Port Config
*******************************************************/
//static source ip addr
#define IP_ADDR0           192
#define IP_ADDR1           168
#define IP_ADDR2           1
#define IP_ADDR3           122

//netmask
#define NETMASK_ADDR0      255
#define NETMASK_ADDR1      255
#define NETMASK_ADDR2      255
#define NETMASK_ADDR3      0

//gateway addr
#define GW_ADDR0           192
#define GW_ADDR1           168
#define GW_ADDR2           1
#define GW_ADDR3           1

//dest ip addr
#define DEST_IP_ADDR0      192
#define DEST_IP_ADDR1      168
#define DEST_IP_ADDR2      1
#define DEST_IP_ADDR3      102

//source port num
#define LOCAL_PORT         5001

//dest port num
#define DEST_PORT          5001

/*******************************************************
*                 ETHERNET Pin Config
*******************************************************/
#define ETH_TX_EN          P10_1
#define ETH_TXD0           P10_0
#define ETH_TXD1           P9_7
#define ETH_CRS_DV         P9_5
#define ETH_RXD0           P9_4
#define ETH_RXD1           P9_3
#define ETH_REF_CLK        P9_6
#define ETH_RX_ERR         P10_2
#define ETH_MDC            P1_7
#define ETH_MDIO           P3_7

#ifdef __cplusplus
}
#endif

#endif  /* _BOARD_H_ */
/******************* (C) COPYRIGHT 2023 Realtek Semiconductor *****END OF FILE****/
