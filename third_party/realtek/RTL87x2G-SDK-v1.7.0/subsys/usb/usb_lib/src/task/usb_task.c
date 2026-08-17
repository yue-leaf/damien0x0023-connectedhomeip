#include <stddef.h>
#include "os_msg.h"
#include "os_task.h"
#include "trace.h"
#include "usb_task.h"
#include "usb_isr.h"
#include "usb_dm_int.h"

static void *usb_task_handle = NULL;
static void *usb_task_msg_queue = NULL;

int usb_task_msg_send(T_USB_TASK_MSG *msg)
{
    return os_msg_send(usb_task_msg_queue, msg, 0);
}

static void usb_task_msg_handle(T_USB_TASK_MSG *msg)
{
    uint8_t group = msg->group;

    if (group == USB_TASK_MSG_GROUP_HAL)
    {
        usb_isr_msg_handle(msg);
    }
    else if (group == USB_TASK_MSG_GROUP_DM)
    {
        usb_dm_msg_handle(msg);
    }
    else
    {
        USB_PRINT_ERROR1("usb_task_msg_handle, invalid group:0x%x", group);
    }
}

static void usb_task(void *param)
{
    os_alloc_secure_ctx(1024);
    T_USB_TASK_MSG usb_msg;

    while (1)
    {
        if (os_msg_recv(usb_task_msg_queue, &usb_msg, 0xFFFFFFFFUL) == true)
        {
            usb_task_msg_handle(&usb_msg);
        }
    }
}


int usb_task_create(void)
{
    os_msg_queue_create(&usb_task_msg_queue, "usb task queue", 0x20, sizeof(T_USB_TASK_MSG));

    return (int)os_task_create(&usb_task_handle, "usb_task", usb_task, NULL,
                               1024 * 2, 3);
}

int usb_task_release(void)
{
    int ret = (int)os_task_delete(usb_task_handle);
    usb_task_handle = NULL;
    os_msg_queue_delete(usb_task_msg_queue);
    return ret;
}
