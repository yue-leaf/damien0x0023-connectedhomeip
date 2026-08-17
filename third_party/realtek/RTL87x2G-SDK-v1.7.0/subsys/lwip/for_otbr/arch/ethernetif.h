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
#define NETIF_IN_TASK_STACK_SIZE           1024
#define NETIF_IN_TASK_PRIORITY             3

/*============================================================================*
 *                         Types
 *============================================================================*/

/*============================================================================*
*                        Export Global Variables
*============================================================================*/

/*============================================================================*
 *                         Functions
 *============================================================================*/
err_t ethernetif_init(struct netif *netif);
void ethernetif_input(void *pParams);
void ethernetif_output(void *argument);
void ethernetif_update_config(struct netif *netif);

#ifdef  __cplusplus
}
#endif
#endif /* ETHERNETIF_H */
