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
#include "lwip/netifapi.h"

#include "app_section.h"



/*============================================================================*
 *                              Local Macros
 *============================================================================*/

typedef enum
{
    LNK_LOCAL = 0,
    ON_LNK = 1,
} NETIF_ADDR_IDX;


/*============================================================================*
 *                              Local Variables
 *============================================================================*/
struct netif gnetif;
ip_addr_t ipaddr;
ip_addr_t netmask;
ip_addr_t gw;
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

void eth_set_on_link_addr(ip6_addr_t *on_link_addr)
{
    char *ip6_addr_str = ip6addr_ntoa(&on_link_addr);
    DBG_DIRECT("eth_set_on_link_addr: on_link_addr %s", ip6_addr_str);
    ip_addr_copy_from_ip6(gnetif.ip6_addr[ON_LNK], *on_link_addr);
    netif_ip6_addr_set_state(&gnetif, ON_LNK, IP6_ADDR_TENTATIVE);
}




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
    DBG_DIRECT("TCPIP_Init");

    DBG_DIRECT("[TCPIP_Init] 000");
    tcpip_init(NULL, NULL);

#if 1
    DBG_DIRECT("[TCPIP_Init] ipv4");
#if LWIP_DHCP
    ip_addr_set_zero_ip4(&ipaddr);
    ip_addr_set_zero_ip4(&netmask);
    ip_addr_set_zero_ip4(&gw);
#else
    IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
    IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
    IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
#endif

    netif_add(&gnetif,
#if LWIP_IPV4
              &ipaddr, &netmask, &gw,
#endif
              NULL, &ethernetif_init, &tcpip_input);

#if 1


    netif_create_ip6_linklocal_address(&gnetif, 1);

    ip6_addr_t on_link_addr = {};
    ip6addr_aton("fd23:7911:1122:2222::f", &on_link_addr);
    eth_set_on_link_addr(&on_link_addr);

    /*
        dhcp6_enable_stateless(&gnetif);
    */
    netif_set_default(&gnetif);


    DBG_DIRECT("[TCPIP_Init] 333");

    if (!netif_is_up(&gnetif))
    {
        netif_set_up(&gnetif);
        netif_set_link_up(&gnetif);
    }

    /*
        if (netif_is_link_up(&gnetif))
        {
            netif_set_up(&gnetif);
        }
        else
        {
            netif_set_down(&gnetif);
        }
    */
#if LWIP_DHCP
    int err;
    int count = 0;

    err = dhcp_start(&gnetif);
    if (err == ERR_OK)
    {
        DBG_DIRECT("[TCPIP_Init] lwip dhcp init success");
    }
    else
    {
        DBG_DIRECT("[TCPIP_Init] lwip dhcp init fail");
    }

    while ((dhcp_supplied_address(&gnetif) == 0) &&
           (count <= 40))
        //while(gnetif.ip6_addr[1].u_addr.ip4.addr == 0 ||
        //      gnetif.ip6_addr[1].u_addr.ip4.addr == 0x111111FD &&
        //      count <= 20)
    {
        vTaskDelay(250); //250ms
        count++;
        //vTaskDelay(5000); //5000ms
    }
#endif
#endif
#endif
#if 1
    //netif_ip6_addr_set_state(&gnetif, 0, IP6_ADDR_PREFERRED);

    DBG_DIRECT("[TCPIP_Init] local IPv4 addr0:%s",
               ip4addr_ntoa(netif_ip4_addr(&gnetif)));
    for (u8_t i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
    {
        if (!ip6_addr_isinvalid(netif_ip6_addr_state(&gnetif, i)))
        {
            char buf[40];
            DBG_DIRECT("IPv6[%d]: %s [state: %s]\n", i,
                       ip6addr_ntoa_r(netif_ip6_addr(&gnetif, i), buf, sizeof(buf)),
                       (netif_ip6_addr_state(&gnetif, i) & IP6_ADDR_VALID) ? "valid" : "tentative");
        }
    }
    /*
        DBG_DIRECT("[TCPIP_Init] local IPv4 addr0:%02x.%02x.%02x.%02x\n\n", \
                   ((gnetif.ip6_addr[0].u_addr.ip4.addr) & 0x000000ff), \
                   (((gnetif.ip6_addr[0].u_addr.ip4.addr) & 0x0000ff00) >> 8),  \
                   (((gnetif.ip6_addr[0].u_addr.ip4.addr) & 0x00ff0000) >> 16), \
                   ((gnetif.ip6_addr[0].u_addr.ip4.addr) & 0xff000000) >> 24);

        DBG_DIRECT("[TCPIP_Init] local IPv4 addr1:%02x.%02x.%02x.%02x\n\n", \
                   ((gnetif.ip6_addr[1].u_addr.ip4.addr) & 0x000000ff), \
                   (((gnetif.ip6_addr[1].u_addr.ip4.addr) & 0x0000ff00) >> 8),  \
                   (((gnetif.ip6_addr[1].u_addr.ip4.addr) & 0x00ff0000) >> 16), \
                   ((gnetif.ip6_addr[1].u_addr.ip4.addr) & 0xff000000) >> 24);

        DBG_DIRECT("[TCPIP_Init] local IPv6 addr0:%08x.%08x.%08x.%08x\n\n", \
                        ((gnetif.ip6_addr[0].u_addr.ip6.addr[0])),       \
                        (((gnetif.ip6_addr[0].u_addr.ip6.addr[1]))),  \
                        (((gnetif.ip6_addr[0].u_addr.ip6.addr[2]))), \
                        ((gnetif.ip6_addr[0].u_addr.ip6.addr[3])));

        DBG_DIRECT("[TCPIP_Init] local IPv6 addr1:%08x.%08x.%08x.%08x\n\n", \
                        ((gnetif.ip6_addr[1].u_addr.ip6.addr[0])),       \
                        (((gnetif.ip6_addr[1].u_addr.ip6.addr[1]))),  \
                        (((gnetif.ip6_addr[1].u_addr.ip6.addr[2]))), \
                        ((gnetif.ip6_addr[1].u_addr.ip6.addr[3])));
    */
#endif
}

/******************* (C) COPYRIGHT 2023 Realtek Semiconductor Corporation *****END OF FILE****/
