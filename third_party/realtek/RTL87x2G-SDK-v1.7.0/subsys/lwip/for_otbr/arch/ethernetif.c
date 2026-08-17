/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "lwip/opt.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/timeouts.h"
#include "lwip/prot/ethernet.h"
#include "lwip/etharp.h"
#include "lwip/ethip6.h"
#include "ethernetif.h"
#include <string.h>
#include <trace.h>
#include "os_mem.h"
#include "ethernet_driver.h"
#include "rtl_ethernet.h"
#include "app_section.h"

//#include "rtk_otbr_log.h"
/*============================================================================*
 *                              Local Variables
 *============================================================================*/
/* Network interface name */
#define IFNAME0 'r'
#define IFNAME1 't'

struct ethernetif
{
    struct eth_addr *ethaddr;
    /* Add whatever per-interface state that is needed here. */
};
/*============================================================================*
 *                              Global Variables
 *============================================================================*/
xSemaphoreHandle s_xSemaphore = NULL;

/*============================================================================*
 *                              Functions Declaration
 *============================================================================*/
static err_t low_level_output(struct netif *netif, struct pbuf *p) RAM_FUNCTION;
static struct pbuf *low_level_input(struct netif *netif) RAM_FUNCTION;
void ethernetif_input(void *pParams) RAM_FUNCTION;

/*============================================================================*
 *                              Local Functions
 *============================================================================*/
/******************************************************************
 * @brief  init hardware. Called from ethernetif_init.
 * @param  netif - the already initialized lwip network interface structure for this ethernetif
 * @return none
 */
static void low_level_init(struct netif *netif)
{
    DBG_DIRECT("[low_level_init]");
    //APP_PRINT_INFO0("[low_level_init]");

    int tmp_status = ETH_STATUS_OK;
    uint8_t mac[6];

    eth_init_driver();

    if (tmp_status == ETH_STATUS_OK)
    {
        /* Set netif link flag */
        netif->flags |= NETIF_FLAG_LINK_UP;
    }

#if LWIP_ARP || LWIP_ETHERNET
    /* set MAC hardware address length */
    netif->hwaddr_len = ETH_HWADDR_LEN;

    /*get mac*/
    ETH_GetMacAddr(&ETH_InitStruct, mac);

    /* set MAC hardware address */
    netif->hwaddr[0] =  mac[0];
    netif->hwaddr[1] =  mac[1];
    netif->hwaddr[2] =  mac[2];
    netif->hwaddr[3] =  mac[3];
    netif->hwaddr[4] =  mac[4];
    netif->hwaddr[5] =  mac[5];

    DBG_DIRECT("[low_level_init] MAC Addr= %02x:%02x:%02x:%02x:%02x:%02x",
               netif->hwaddr[0], netif->hwaddr[1], netif->hwaddr[2],
               netif->hwaddr[3], netif->hwaddr[4], netif->hwaddr[5]);
    /* maximum transfer unit */
    netif->mtu = NETIF_MTU;

#if LWIP_ARP
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
#else
    netif->flags |= NETIF_FLAG_BROADCAST;
#endif /* LWIP_ARP */

#endif /* LWIP_ARP || LWIP_ETHERNET */
    netif->flags |= NETIF_FLAG_IGMP | NETIF_FLAG_MLD6;

    s_xSemaphore = xSemaphoreCreateCounting(40, 0);
    /* create the task that handles the ETH_MAC */
    sys_thread_new("ETHIN",
                   ethernetif_input,
                   netif,
                   NETIF_IN_TASK_STACK_SIZE,
                   NETIF_IN_TASK_PRIORITY);

    ETH_EnableRx();
}

/******************************************************************
 * @brief  send packet
 * @param  netif - the lwip network interface structure for this ethernetif
 * @param  p - the MAC packet to send
 * @return an err_t value if the packet couldn't be sent or ERR_OK if the packet could be sent
 */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    //RTK_OTBR_LOGI("%s start", __FUNCTION__);
    static sys_sem_t outsem = NULL;
    if (outsem == NULL)
    {
        if (ERR_OK == sys_sem_new(&outsem, 0))
        {
            sys_sem_signal(&outsem);
        }
        else
        {
            APP_PRINT_WARN0("[low_level_output] creat outsem fail");
        }
    }

    err_t errval;
    struct pbuf *q;

    uint8_t tx_serach_idx = ETH_InitStruct.ETH_TxDescCurrentNum;
    uint8_t *buffer = (uint8_t *)(ETH_InitStruct.ETH_TxDesc[tx_serach_idx].addr);
    uint32_t framelength = 0;
    uint32_t bufferoffset = 0;
    uint32_t byteslefttocopy = 0;
    uint32_t payloadoffset = 0;

    if ((((volatile uint32_t)(ETH_InitStruct.ETH_TxDesc[tx_serach_idx].dw1)) & ETH_TX_DESC_OWN) != 0)
    {
        APP_PRINT_WARN0("[low_level_output] Tx descriptor ring is full !!");
        errval = ERR_USE;
        goto error;
    }

    sys_sem_wait(&outsem);

    /* copy frame from pbufs to driver buffers */
    for (q = p; q != NULL; q = q->next)
    {
        /* Get bytes in current lwIP buffer */
        byteslefttocopy = q->len;
        payloadoffset = 0;

        //RTK_OTBR_LOGI("%s %d mid", __FUNCTION__, q->len);
        /* Check if the length of data to copy is bigger than Tx buffer size*/
        while ((byteslefttocopy + bufferoffset) > ETH_TX_BUF_SIZE)
        {
            /* Copy data to Tx buffer*/
            memcpy((uint8_t *)((uint8_t *)buffer + bufferoffset),
                   (uint8_t *)((uint8_t *)q->payload + payloadoffset),
                   (ETH_TX_BUF_SIZE - bufferoffset));

            /* Point to next descriptor */
            if (tx_serach_idx == ((ETH_InitStruct.ETH_TxDescNum) - 1))
            {
                tx_serach_idx = 0;
            }
            else
            {
                tx_serach_idx++;
            }

            if ((((volatile uint32_t)(ETH_InitStruct.ETH_TxDesc[tx_serach_idx].dw1)) & ETH_TX_DESC_OWN) != 0)
            {
                APP_PRINT_WARN0("[low_level_output] Tx descriptor ring is full !!");
            }

            buffer = (uint8_t *)(ETH_InitStruct.ETH_TxDesc[tx_serach_idx].addr);

            byteslefttocopy = byteslefttocopy - (ETH_TX_BUF_SIZE - bufferoffset);
            payloadoffset = payloadoffset + (ETH_TX_BUF_SIZE - bufferoffset);
            framelength = framelength + (ETH_TX_BUF_SIZE - bufferoffset);
            bufferoffset = 0;
        }

        /* Copy the remaining bytes */
        memcpy((uint8_t *)((uint8_t *)buffer + bufferoffset),
               (uint8_t *)((uint8_t *)q->payload + payloadoffset), byteslefttocopy);
        bufferoffset = bufferoffset + byteslefttocopy;
        framelength = framelength + byteslefttocopy;
    }

    /* Prepare transmit descriptors to give to DMA */
    ETH_SendFrame(&ETH_InitStruct, framelength);

    //RTK_OTBR_LOGI("%s %d end", __FUNCTION__, framelength);
    errval = ERR_OK;

error:
    sys_sem_signal(&outsem);

    return errval;
}

/******************************************************************
 * @brief  receive packet
 * @param  netif - the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet
 */
static struct pbuf *low_level_input(struct netif *netif)
{
    struct pbuf *p = NULL;
    struct pbuf *q = NULL;
    uint16_t len = 0;
    uint8_t *buffer;
    uint32_t bufferoffset = 0;
    uint32_t payloadoffset = 0;
    uint32_t byteslefttocopy = 0;
    uint32_t i = 0;
    uint8_t rx_serach_idx = 0, tmp_rx_serach_idx = 0;
    uint32_t tmp_seg_count = 0;
    bool is_first_copy = true;

    /* get received frame */
    if (ETH_ReceiveFrame(&ETH_InitStruct) != ETH_STATUS_OK)
    {
        ETH_DBG_BUFFER(MODULE_APP, LEVEL_WARN, "[low_level_input] receive frame failed\n", 0);
        return NULL;
    }

    tmp_seg_count = ETH_InitStruct.ETH_RxSegmentCount;
    /* Clear Segment_Count */
    ETH_InitStruct.ETH_RxSegmentCount = 0;
    tmp_rx_serach_idx = ETH_InitStruct.ETH_RxFrameStartDescIdx;
    rx_serach_idx = tmp_rx_serach_idx;

    ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[low_level_input] low_level_input rx_idx=%d\n", 1,
                   rx_serach_idx);
    /* Obtain the size of the packet and put it into the "len" variable. */
    len = ETH_InitStruct.ETH_RxFrameLen;
    buffer = (uint8_t *)((ETH_InitStruct.ETH_RxDesc[rx_serach_idx].addr) + 2);//2byte offset

    ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[low_level_input] receive frame %d len buffer : %p\n", 2,
                   len, buffer);
    if (len > 0)
    {
        /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    }

    if (p != NULL)
    {
        bufferoffset = 0;
        for (q = p; q != NULL; q = q->next)
        {
            byteslefttocopy = q->len;
            payloadoffset = 0;

            while (((byteslefttocopy + bufferoffset) > ETH_RX_BUF_SIZE) ||
                   ((is_first_copy == true) && (byteslefttocopy >= (ETH_RX_BUF_SIZE - 2))))
            {
                /* Copy data to pbuf */
                if ((is_first_copy == true) && (byteslefttocopy >= (ETH_RX_BUF_SIZE - 2)))
                {
                    memcpy((uint8_t *)((uint8_t *)q->payload + payloadoffset),
                           (uint8_t *)((uint8_t *)buffer + bufferoffset),
                           (ETH_RX_BUF_SIZE - 2 - bufferoffset));

                    byteslefttocopy = byteslefttocopy - (ETH_RX_BUF_SIZE - 2 - bufferoffset);
                    payloadoffset = payloadoffset + (ETH_RX_BUF_SIZE - 2 - bufferoffset);
                }
                else
                {
                    memcpy((uint8_t *)((uint8_t *)q->payload + payloadoffset),
                           (uint8_t *)((uint8_t *)buffer + bufferoffset),
                           (ETH_RX_BUF_SIZE - bufferoffset));

                    byteslefttocopy = byteslefttocopy - (ETH_RX_BUF_SIZE - bufferoffset);
                    payloadoffset = payloadoffset + (ETH_RX_BUF_SIZE - bufferoffset);
                }

                is_first_copy = false;

                /* Point to next descriptor */
                if (rx_serach_idx == ((ETH_InitStruct.ETH_RxDescNum) - 1))
                {
                    rx_serach_idx = 0;
                }
                else
                {
                    rx_serach_idx++;
                }
                buffer = (uint8_t *)(ETH_InitStruct.ETH_RxDesc[rx_serach_idx].addr);
                bufferoffset = 0;
            }
            /* Copy remaining data in pbuf */
            memcpy((uint8_t *)((uint8_t *)q->payload + payloadoffset),
                   (uint8_t *)((uint8_t *)buffer + bufferoffset), byteslefttocopy);
            bufferoffset = bufferoffset + byteslefttocopy;
        }
    }

    /* Release descriptors to DMA */
    /* Point to first descriptor */
    rx_serach_idx = tmp_rx_serach_idx;
    /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
    for (i = 0; i < tmp_seg_count; i++)
    {
        ETH_InitStruct.ETH_RxDesc[rx_serach_idx].dw1 &= ETH_RX_DESC_EOR;
        ETH_InitStruct.ETH_RxDesc[rx_serach_idx].dw1 |= (ETH_RX_DESC_OWN | ETH_RX_BUF_SIZE);
        ETH_InitStruct.ETH_RxDesc[rx_serach_idx].dw2 = 0;
        ETH_InitStruct.ETH_RxDesc[rx_serach_idx].dw3 = 0;
        if (rx_serach_idx == ((ETH_InitStruct.ETH_RxDescNum) - 1))
        {
            rx_serach_idx = 0;
        }
        else
        {
            rx_serach_idx++;
        }
    }

    return p;
}

/******************************************************************
 * @brief  receive packet
 * @param  pParams - the lwip network interface structure for this ethernetif
 * @return none
 */
void ethernetif_input(void *pParams)
{
    DBG_DIRECT("ethernetif_input");
    //APP_PRINT_INFO0("ethernetif_input");
    struct netif *netif;
    struct pbuf *p = NULL;
    netif = (struct netif *) pParams;

    while (1)
    {
        if (xSemaphoreTake(s_xSemaphore, portMAX_DELAY) == pdTRUE)
        {
            /* move received packet into a new pbuf */
            taskENTER_CRITICAL();
TRY_GET_NEXT_FRAGMENT:
            ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ethernetif_input] low level input", 0);

            p = low_level_input(netif);
            taskEXIT_CRITICAL();
            /* points to packet payload, which starts with an Ethernet header */
            if (p != NULL)
            {
                taskENTER_CRITICAL();
                /* full packet send to tcpip_thread to process */
                if (netif->input(p, netif) != ERR_OK)
                {
                    APP_PRINT_INFO0("[ethernetif_input] IP input error");
                    pbuf_free(p);
                    p = NULL;
                }
                else
                {
                    ETH_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ethernetif_input] try again", 0);
                    xSemaphoreTake(s_xSemaphore, 0);
                    goto TRY_GET_NEXT_FRAGMENT;
                }
                taskEXIT_CRITICAL();
            }
        }
    }
}

#if !LWIP_ARP
static err_t low_level_output_arp_off(struct netif *netif, struct pbuf *q, const ip4_addr_t *ipaddr)
{
    err_t errval;
    errval = ERR_OK;

    return errval;

}
#endif /* LWIP_ARP */

/*============================================================================*
*                              Global Functions
*============================================================================*/
/******************************************************************
 * @brief  init ethernetif
 * @param  netif - the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized or ERR_MEM if private data couldn't be allocated
 */
err_t ethernetif_init(struct netif *netif)
{
    DBG_DIRECT("ethernetif_init");
    //APP_PRINT_INFO0("[ethernetif_init]");
    struct ethernetif *ethernetif;

    LWIP_ASSERT("netif != NULL", (netif != NULL));

    ethernetif = os_mem_alloc(RAM_TYPE_DATA_ON, sizeof(struct ethernetif));

    if (ethernetif == NULL)
    {
        APP_PRINT_ERROR0("[ethernetif_init] out of memory\n");
        return ERR_MEM;
    }

    LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */
    netif->state = ethernetif;
    netif->name[0] = "rt"[0];
    netif->name[1] = "rt"[1];

#if LWIP_IPV4
#if LWIP_ARP || LWIP_ETHERNET
#if LWIP_ARP
    netif->output = etharp_output;
#else
    netif->output = low_level_output_arp_off;
#endif /* LWIP_ARP */
#endif /* LWIP_ARP || LWIP_ETHERNET */
#endif /* LWIP_IPV4 */

#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */

    netif->linkoutput = low_level_output;
    netif->ip6_autoconfig_enabled = 1;

    /* initialize the hardware */
    low_level_init(netif);
    ethernetif->ethaddr = (struct eth_addr *) & (netif->hwaddr[0]);

    return ERR_OK;
}
