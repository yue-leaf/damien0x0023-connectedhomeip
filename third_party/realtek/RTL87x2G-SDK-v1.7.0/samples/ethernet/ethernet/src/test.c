/**
*********************************************************************************************************
*               Copyright(c) 2023, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     test.c
* @brief    test code
* @details
* @author   mandy
* @date     2023-4-25
* @version  v1.0
*********************************************************************************************************
*/

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "test.h"
#include "lwip/opt.h"
#include "os_mem.h"
#if LWIP_NETCONN
#include "lwip/sys.h"
#include "lwip/api.h"
#include "trace.h"
#include "lwip/arch.h"
#include "board.h"

/*============================================================================*
 *                              Local Variables
 *============================================================================*/
#if ((TEST_MODE == NETCONN_JPERF_CLIENT_TEST) || (TEST_MODE == NETCONN_JPERF_SERVER_TEST))
#define IPERF_BUFSZ         (4 * 1024)
#endif

/*============================================================================*
 *                              Global Variables
 *============================================================================*/

/*============================================================================*
 *                              Functions Declaration
 *============================================================================*/

/*============================================================================*
 *                              Local Functions
 *============================================================================*/
/******************************************************************
 * @brief  test thread
 * @param
 * @return none
 */
static void test_thread(void *arg)
{
#if (TEST_MODE == NETCONN_TCP_SERVER_TEST)
    struct netconn *conn = NULL, *newconn = NULL;
    err_t err;
    LWIP_UNUSED_ARG(arg);
    APP_PRINT_INFO0("[test_thread]");

    /* Create a new connection identifier. */
#if LWIP_IPV6
    conn = netconn_new(NETCONN_TCP_IPV6);
    netconn_bind(conn, IP6_ADDR_ANY, LOCAL_PORT);
#else /* LWIP_IPV6 */
    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, IP_ADDR_ANY, LOCAL_PORT);
#endif /* LWIP_IPV6 */
//    LWIP_ERROR("[test_thread] invalid conn", (conn != NULL), return;);

    APP_PRINT_INFO1("[test_thread] local port is %d", LOCAL_PORT);

    /* Tell connection to go into listening mode. */
    netconn_listen(conn);

    while (1)
    {
        /* Grab new connection. */
        err = netconn_accept(conn, &newconn);

        /* Process the new connection. */
        if (err == ERR_OK)
        {
            struct netbuf *buf;
            void *data;
            u16_t len;

            while ((err = netconn_recv(newconn, &buf)) == ERR_OK)
            {
                do
                {
                    netbuf_data(buf, &data, &len);
                    err = netconn_write(newconn, data, len, NETCONN_COPY);

                    if (err != ERR_OK)
                    {
                        APP_PRINT_INFO1("[test_thread] netconn_write: error \"%s\"\n", lwip_strerr(err));
                    }

                }
                while (netbuf_next(buf) >= 0);

                netbuf_delete(buf);
            }

            /* Close connection and discard connection identifier. */
            netconn_close(newconn);
            netconn_delete(newconn);
        }
    }
#elif (TEST_MODE == NETCONN_TCP_CLIENT_TEST)
    struct netconn *conn = NULL;
    int ret;
    ip4_addr_t ipaddr;

    uint8_t send_buf[] = "Realtek TCP Client test.\n";

    APP_PRINT_INFO5("[test_thread] DEST IP ADDR:%d.%d.%d.%d. PORT:%d", DEST_IP_ADDR0, DEST_IP_ADDR1,
                    DEST_IP_ADDR2, DEST_IP_ADDR3, DEST_PORT);

    while (1)
    {
        conn = netconn_new(NETCONN_TCP);
        if (conn == NULL)
        {
            APP_PRINT_INFO0("[test_thread] create conn failed!");
            vTaskDelay(10);
            continue;
        }

        IP4_ADDR(&ipaddr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3);

        ret = netconn_connect(conn, &ipaddr, DEST_PORT);
        if (ret == -1)
        {
            APP_PRINT_INFO0("[test_thread] Connect failed!");
            netconn_close(conn);
            vTaskDelay(10);
            continue;
        }

        APP_PRINT_INFO0("[test_thread] Connect to server successful!");

        while (1)
        {
            ret = netconn_write(conn, send_buf, sizeof(send_buf), 0);

            vTaskDelay(100);
        }
    }
#elif (TEST_MODE == NETCONN_UDP_TEST)
    struct netconn *conn = NULL;
    struct netbuf *buf;
    char buffer[4096];
    err_t err;
    LWIP_UNUSED_ARG(arg);

#if LWIP_IPV6
    conn = netconn_new(NETCONN_UDP_IPV6);
    netconn_bind(conn, IP6_ADDR_ANY, LOCAL_PORT);
#else /* LWIP_IPV6 */
    conn = netconn_new(NETCONN_UDP);
    netconn_bind(conn, IP_ADDR_ANY, LOCAL_PORT);
#endif /* LWIP_IPV6 */

//    LWIP_ERROR("[test_thread] udpecho: invalid conn", (conn != NULL), return;);

    APP_PRINT_INFO1("[test_thread] local port is %d", LOCAL_PORT);

    while (1)
    {
        err = netconn_recv(conn, &buf);
        if (err == ERR_OK)
        {
            /*  no need netconn_connect here, since the netbuf contains the address */
            if (netbuf_copy(buf, buffer, sizeof(buffer)) != buf->p->tot_len)
            {
                LWIP_DEBUGF(LWIP_DBG_ON, "netbuf_copy failed\n");
            }
            else
            {
                buffer[buf->p->tot_len] = '\0';
                err = netconn_send(conn, buf);
                if (err != ERR_OK)
                {
                    LWIP_DEBUGF(LWIP_DBG_ON, "netconn_send failed: %d\n", (int)err);
                }
                else
                {
                    LWIP_DEBUGF(LWIP_DBG_ON, "got %s\n", buffer);
                }
            }
            netbuf_delete(buf);
        }
    }
#elif (TEST_MODE == NETCONN_JPERF_CLIENT_TEST)
    struct netconn *conn = NULL;
    int i;
    int ret;
    uint8_t *send_buf;
    uint64_t sentlen;
    u32_t tick1, tick2;
    ip4_addr_t ipaddr;

    send_buf = (uint8_t *) os_mem_alloc(RAM_TYPE_DATA_ON, IPERF_BUFSZ);
    if (!send_buf)
    {
        return ;
    }

    for (i = 0; i < IPERF_BUFSZ; i ++)
    {
        send_buf[i] = i & 0xff;
    }

    while (1)
    {
        conn = netconn_new(NETCONN_TCP);

        if (conn == NULL)
        {
            APP_PRINT_INFO0("[test_thread] create conn failed!");
            vTaskDelay(10);
            continue;
        }

        IP4_ADDR(&ipaddr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3);

        ret = netconn_connect(conn, &ipaddr, DEST_PORT);

        if (ret == -1)
        {
            APP_PRINT_INFO0("[test_thread] Connect failed!\n");
            netconn_close(conn);
            vTaskDelay(10);
            continue;
        }

        APP_PRINT_INFO0("[test_thread] Connect to iperf server successful!");

        tick1 = sys_now();
        while (1)
        {
//          tick2 = sys_now();

//          if(tick2 - tick1 >= configTICK_RATE_HZ * 5)
//          {
//              float f;
//              f = (float)(sentlen*configTICK_RATE_HZ/125/(tick2 - tick1));
//              f /= 1000.0f;
//              APP_PRINT_INFO1("send speed = %.4f Mbps!\n", f);

//              tick1 = tick2;
//              sentlen = 0;
//          }
            ret = netconn_write(conn, send_buf, IPERF_BUFSZ, NETCONN_COPY);
//          if (ret == ERR_OK)
//          {
//              sentlen += IPERF_BUFSZ;
//          }
//          vTaskDelay(100);
        }
        netconn_close(conn);
        netconn_delete(conn);
    }
#elif (TEST_MODE == NETCONN_JPERF_SERVER_TEST)
    struct netconn *conn, *newconn;
    err_t err;
    void *recv_data;

    recv_data = (void *)os_mem_alloc(RAM_TYPE_DATA_ON, IPERF_BUFSZ);
    if (recv_data == NULL)
    {
        APP_PRINT_INFO0("[test_thread] No memory\n");
    }

    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, IP_ADDR_ANY, LOCAL_PORT);

//    LWIP_ERROR("test_thread: invalid conn", (conn != NULL), return;);

    /* Tell connection to go into listening mode. */
    netconn_listen(conn);

    while (1)
    {
        /* Grab new connection. */
        err = netconn_accept(conn, &newconn);

        /* Process the new connection. */
        if (err == ERR_OK)
        {
            struct netbuf *buf;
            u16_t len;

            while ((err = netconn_recv(newconn, &buf)) == ERR_OK)
            {
                do
                {
                    netbuf_data(buf, &recv_data, &len);
                }
                while (netbuf_next(buf) >= 0);

                netbuf_delete(buf);
            }

            /* Close connection and discard connection identifier. */
            netconn_close(newconn);
            netconn_delete(newconn);
        }
    }
#endif
}

/*============================================================================*
*                              Global Functions
*============================================================================*/
/******************************************************************
 * @brief  init test thread
 * @param  none
 * @return none
 */
void test_init(void)
{
    sys_thread_new("test_thread", test_thread, NULL, 2048, 2);
}

#endif /* LWIP_NETCONN */

/******************* (C) COPYRIGHT 2023 Realtek Semiconductor Corporation *****END OF FILE****/
