/**
*********************************************************************************************************
*               Copyright(c) 2023, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     common.c
* @brief    init tcpip
* @details
* @author   mandy
* @date     2023-4-25
* @version  v1.0
*********************************************************************************************************
*/

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "board.h"
#include "trace.h"
#include "lwip/tcpip.h"
#include "ethernetif.h"
#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif

/*============================================================================*
 *                              Local Macros
 *============================================================================*/

/*============================================================================*
 *                              Local Variables
 *============================================================================*/
struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;
uint8_t IP_ADDRESS[4];
uint8_t NETMASK_ADDRESS[4];
uint8_t GATEWAY_ADDRESS[4];

/*============================================================================*
 *                              Global Variables
 *============================================================================*/

/*============================================================================*
 *                              Functions Declaration
 *============================================================================*/

/*============================================================================*
 *                              Local Functions
 *============================================================================*/

/*============================================================================*
*                              Global Functions
*============================================================================*/
/******************************************************************
 * @brief  init TCPIP
 * @param  none
 * @return none
 */
void TCPIP_Init(void)
{
    APP_PRINT_INFO0("[TCPIP_Init]");

    tcpip_init(NULL, NULL);

#if LWIP_DHCP
    ip_addr_set_zero_ip4(&ipaddr);
    ip_addr_set_zero_ip4(&netmask);
    ip_addr_set_zero_ip4(&gw);
#else
    IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
    IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
    IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
#endif

    netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

    netif_set_default(&gnetif);

    if (netif_is_link_up(&gnetif))
    {
        netif_set_up(&gnetif);
    }
    else
    {
        netif_set_down(&gnetif);
    }

#if LWIP_DHCP
    int err;

    err = dhcp_start(&gnetif);
    if (err == ERR_OK)
    {
        APP_PRINT_INFO0("[TCPIP_Init] lwip dhcp init success");
    }
    else
    {
        APP_PRINT_INFO0("[TCPIP_Init] lwip dhcp init fail");
    }
    while (ip_addr_cmp(&(gnetif.ip_addr), &ipaddr))
    {
        vTaskDelay(1);
    }
#endif

    APP_PRINT_INFO4("[TCPIP_Init] local IP addr:%d.%d.%d.%d\n\n", \
                    ((gnetif.ip_addr.addr) & 0x000000ff),       \
                    (((gnetif.ip_addr.addr) & 0x0000ff00) >> 8),  \
                    (((gnetif.ip_addr.addr) & 0x00ff0000) >> 16), \
                    ((gnetif.ip_addr.addr) & 0xff000000) >> 24);
}

/******************* (C) COPYRIGHT 2023 Realtek Semiconductor Corporation *****END OF FILE****/
