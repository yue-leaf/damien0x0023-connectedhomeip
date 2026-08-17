#ifndef ETHERNETIF_H__
#define ETHERNETIF_H__

#ifdef  __cplusplus
extern "C"
{
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "lwip/err.h"
#include "lwip/netif.h"

/*============================================================================*
 *                         Macros
 *============================================================================*/
#define NETIF_MTU                          1500

/*============================================================================*
 *                         Types
 *============================================================================*/

typedef struct
{
    netif_status_callback_fn  status_callback;
    netif_status_callback_fn  link_callback;
} netif_callback_t;

/*============================================================================*
*                        Export Global Variables
*============================================================================*/

/*============================================================================*
 *                         Functions
 *============================================================================*/
err_t ethernetif_init(struct netif *netif);
void ethernetif_input(void *pParams);
void ethernetif_recv(struct netif *netif, int total_len);
void TCPIP_Init(netif_callback_t *netif_cb);

#ifdef  __cplusplus
}
#endif
#endif /* ETHERNETIF_H */
