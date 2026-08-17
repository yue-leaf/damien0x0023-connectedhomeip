#include <string.h>
#include "app_task.h"
#include "board.h"
#include "trace.h"
uint8_t send_ok = 0;
#define configMINIMAL_SECURE_STACK_SIZE  1024

#define APP_TASK_PRIORITY               1   /* Task priorities. */
#define APP_TASK_STACK_SIZE             (512 * 4)

#define MAX_NUMBER_OF_GAP_MESSAGE       0x20
#define MAX_NUMBER_OF_IO_MESSAGE        0x20
#define MAX_NUMBER_OF_EVENT_MESSAGE     (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE)

#include "os_task.h"
#include "os_timer.h"
#include "os_mem.h"
#include "os_msg.h"

#define TEST_FTL_IO_MSG_VAL      0xAA
#define TEST_OS_IO_MSG_VAL       0xAB

void *app_task_handle;
void *evt_queue_handle;
void *io_queue_handle;
void *xTestTimerHandle = NULL;
uint32_t timers = 0;

/**
 * \brief    send msg queue to app task.
 *
 * \param[in]   p_handle   The handle to the message queue being peeked.
 *
 * \return           The status of the message queue peek.
 * \retval true      Message queue was peeked successfully.
 * \retval false     Message queue was failed to peek.
 */
bool app_send_msg_to_apptask(uint32_t *p_msg)
{
    uint32_t event = EVENT_IO_TO_APP;

    if (os_msg_send(io_queue_handle, p_msg, 0) == false)
    {
        APP_PRINT_INFO0("send_io_msg_to_app fail");
        return false;
    }
    if (os_msg_send(evt_queue_handle, &event, 0) == false)
    {
        APP_PRINT_INFO0("send_evt_msg_to_app fail");
        return false;
    }
    return true;
}
uint32_t timers1 = 0;
uint32_t cnt = 0;
void test_timer_cb(void *xTimer)
{
    timers ++;
    APP_PRINT_INFO1("Test Timer timeout %d times", timers);

    if (timers <= 1)
    {
        uint32_t send_val = TEST_FTL_IO_MSG_VAL;   //test FTL
        app_send_msg_to_apptask(&send_val);
    }
    else if (timers <= 11)
    {
        uint32_t send_val = TEST_OS_IO_MSG_VAL;    //test OS
        app_send_msg_to_apptask(&send_val);
    }
    else if (timers <= 31)
    {
        if (send_ok == 1)
        {
            extern  bool app_cfu_usb_send_data(uint8_t report_id, uint8_t *data, uint16_t len);
            uint8_t Report[7] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t ReportLen = 7;
            app_cfu_usb_send_data(0x01, Report, ReportLen);

            uint32_t send_val = TEST_OS_IO_MSG_VAL;    //test OS
            app_send_msg_to_apptask(&send_val);
        }

    }
#if  RESTART_EN
    else
    {
        timers1++;
        if (timers1 <= 1)
        {

            app_usb_restart();
            uint32_t send_val = TEST_OS_IO_MSG_VAL;    //test OS
            app_send_msg_to_apptask(&send_val);

        }
        else if (timers1 <= 10)
        {
            uint32_t send_val = TEST_OS_IO_MSG_VAL;    //test OS
            app_send_msg_to_apptask(&send_val);
        }
        else if (timers1 <= 20)
        {
            if (send_ok == 1)
            {
                extern  bool app_cfu_usb_send_data(uint8_t report_id, uint8_t *data, uint16_t len);
                uint8_t Report[7] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
                uint8_t ReportLen = 7;
                app_cfu_usb_send_data(0x01, Report, ReportLen);

                uint32_t send_val = TEST_OS_IO_MSG_VAL;    //test OS
                app_send_msg_to_apptask(&send_val);
            }
        }
        else
        {
            timers1 = 0;
            cnt++;
            USB_PRINT_INFO1("CNT= %d", cnt);
        }
    }
#else
    else
    {
        os_timer_stop(&xTestTimerHandle);
    }
#endif

}

void sw_timer_init(void)
{
    uint32_t timer_id;
    APP_PRINT_INFO0("sw_timer_init");

    bool retval = false;
    retval = os_timer_create(&xTestTimerHandle, "Test Timer",  1, \
                             100, true, test_timer_cb);
    if (!retval)
    {
        APP_PRINT_INFO1("create xTimerPeriodWakeupDlps retval=%d", retval);
    }
    else
    {
        os_timer_start(&xTestTimerHandle);
        APP_PRINT_INFO0("Start auto reload Test Timer: Period 1s");
    }
}

void test_heap(void)
{
    for (RAM_TYPE ram_type = RAM_TYPE_DATA_ON; ram_type < RAM_TYPE_NUM; ram_type ++)
    {
        void *p = os_mem_alloc(ram_type, 100);
        DBG_DIRECT("malloc: type %d, addr 0x%x", ram_type, p);
    }
}

/**
* @brief
*
*
* @param   pvParameters
* @return  void
*/
void app_main_task(void *p_param)
{
    uint32_t event, io_msg;

    /* This task calls secure side functions. So allocate a secure context for
     * it. */
    //must locate at the first line
    os_alloc_secure_ctx(configMINIMAL_SECURE_STACK_SIZE);

    APP_PRINT_INFO0("app task");

    os_msg_queue_create(&io_queue_handle, "io queue", MAX_NUMBER_OF_IO_MESSAGE,  sizeof(uint32_t));
    os_msg_queue_create(&evt_queue_handle, "evt queue", MAX_NUMBER_OF_EVENT_MESSAGE, sizeof(uint32_t));

//    gap_start_bt_stack(evt_queue_handle, io_queue_handle, MAX_NUMBER_OF_GAP_MESSAGE);
//    test_heap();
    extern void  app_usb_init(void);
    app_usb_init();
    while (true)
    {
        // Task code goes here.
        if (os_msg_recv(evt_queue_handle, &event, 0xFFFFFFFF) == true)
        {
            APP_PRINT_INFO1("app task: recv event type %u", event);
            if (event == EVENT_IO_TO_APP)
            {
                if (os_msg_recv(io_queue_handle, &io_msg, 0) == true)
                {
                    if (TEST_FTL_IO_MSG_VAL == io_msg)
                    {
#if (TEST_FTL == 1)
                        extern void test_ftl_for_gc(void);
                        test_ftl_for_gc();
#endif
                    }
                    else if (TEST_OS_IO_MSG_VAL == io_msg)
                    {
                        APP_PRINT_INFO0("app task: recv test os io msg success!");
                    }
                    else
                    {
                        APP_PRINT_INFO1("app task: recv io msg 0x%x", io_msg);
                    }

                }
            }
        }
    }
}

void task_init(void)
{
    os_task_create(&app_task_handle, "app", app_main_task, 0, APP_TASK_STACK_SIZE,
                   APP_TASK_PRIORITY);
}

