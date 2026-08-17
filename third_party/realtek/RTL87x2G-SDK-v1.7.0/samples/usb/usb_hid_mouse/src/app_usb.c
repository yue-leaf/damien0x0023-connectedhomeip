#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "pm.h"
#include "usb_dm.h"
#include "trace.h"
#include "os_msg.h"
#include "app_msg.h"
#include "os_task.h"
#include "usb_dev.h"
#include "usb_hid.h"

#define USB_FULL_SPEED  0x00
#define USB_HIGH_SPEED  0x01

typedef enum
{
    USB_EVT_PLUG,
    USB_EVT_UNPLUG,
    USB_EVT_PWR_STATUS_CHG,
    USB_EVT_BC12_DETECT,
} T_USB_EVT;

#define USB_DM_EVT_USB_START 0x01
#define USB_DM_EVT_USB_STOP  0x02

#define USB_DM_EVT_Q_NUM     0x20
typedef struct _store_param
{
    uint32_t pre_cpu_clk;
    T_SYSTEM_POWER_MODE pre_pwr_mode;
} T_STORE_PARAM;

static void *tpl_handle = NULL;
extern uint8_t send_ok;


#define CFU_MAX_TRANSMISSION_UNIT                   0x10
// To minimize the time it takes to send data, set CFU_MAX_PENDING_REQ_NUM to 0
#define CFU_MAX_PENDING_REQ_NUM                     0x0A

static void *app_cfu_tpl_open(void)
{
    T_USB_HID_ATTR attr =
    {
        .zlp = 1,
        .high_throughput = 1,
        .congestion_ctrl = HID_CONGESTION_CTRL_DROP_CUR,
        .rsv = 0,
        .mtu = CFU_MAX_TRANSMISSION_UNIT
    };
    return usb_hid_data_pipe_open(HID_INT_IN_EP_1, attr, CFU_MAX_PENDING_REQ_NUM, NULL);
}


static bool app_usb_dm_cb(T_USB_DM_EVT evt, T_USB_DM_EVT_PARAM *param)
{
    APP_PRINT_INFO1("app_usb_dm_cb = %d", param->status_ind.state);

    if (param->status_ind.state == 5)
    {
        send_ok = 1;

    }
    else
    {
        send_ok = 0;
    }
    return true;
}


void app_usb_start(void)
{
    extern void usb_isr_set_priority(uint8_t priority);
    usb_isr_set_priority(2);
    usb_dm_start(false);
}


void app_usb_stop(void)
{
    usb_dm_stop();
}
typedef struct
{
    uint8_t Button;
    int16_t X; // -32767 ~ 32767. -32768 is illegal.
    int16_t Y; // -32767 ~ 32767
    int8_t V_Wheel; // -127 ~ 127
    int8_t H_Wheel; // -127 ~ 127
} __attribute__((packed)) MouseReport_t;

bool app_cfu_get_report(uint8_t report_id, MouseReport_t *data, uint16_t *len)
{
    report_id = 0x01;
    static const MouseReport_t Report1 = { .X = 1};
    static const MouseReport_t Report0 = { .X = 0};
    static uint32_t i = 0;
    *data = (i % 64 == 0) ? Report1 : Report0;

    *len = sizeof(MouseReport_t) + 1;

    return true;
}
//bool app_hid_get_protocol (uint8_t *hid_protocol)
//{
//    *hid_protocol  = HID_PROTOCOL;
//     DBG_DIRECT("get_protocol = %d ", *hid_protocol);
//       DBG_DIRECT("get_protocol = %d ", *hid_protocol);
//    return true;
//}
//bool app_hid_set_protocol (uint8_t hid_protocol)
//{
//    HID_PROTOCOL = hid_protocol;
//     DBG_DIRECT("set_protocol = %d ", hid_protocol);
//     DBG_DIRECT("set_protocol = %d ", hid_protocol);
//    return true;
//}

bool app_usb_spd_cb(uint8_t speed)
{
    DBG_DIRECT("speed = %d", speed);
    return true;
}
void app_usb_init(void)
{
    usb_dm_cb_register(app_usb_dm_cb);
    usb_spd_cb_register(app_usb_spd_cb);

    T_USB_CORE_CONFIG config = {.speed = USB_HIGH_SPEED, .class_set = {.hid_enable = 1, .uac_enable = 0}};
    usb_dm_core_init(config);

    usb_dev_init();
//    usb_hid_init2();
    usb_hid_init();
    app_usb_start();
}

bool app_cfu_xmit_close()
{
    bool ret = false;
    if (tpl_handle != NULL)
    {
        ret = usb_hid_data_pipe_close(tpl_handle);
        tpl_handle = NULL;
    }
    return ret;
}

void app_usb_restart(void)
{
    app_usb_stop();
    app_usb_start();
}


static bool app_cfu_xmit(void *buf, uint32_t len)
{
    APP_PRINT_INFO2("app_cfu_tpl_xmit:%d-%b", len, TRACE_BINARY(len, buf));
    if (tpl_handle == NULL)
    {
        tpl_handle = app_cfu_tpl_open();
    }

    return usb_hid_data_pipe_send(tpl_handle, buf, len);
}


bool app_cfu_usb_send_data(uint8_t report_id, uint8_t *data, uint16_t len)
{
    uint8_t *header = NULL;
    uint16_t data_len = sizeof(report_id) + len;
    bool ret = false;

    header = malloc(data_len);
    if (!header)
    {
        APP_PRINT_ERROR0("app_cfu_usb_send_data: no memory for data");
        return ret;
    }
    header[0] = report_id;
    memcpy(&header[1], data, len);
    app_cfu_xmit(header, data_len);
    free(header);
    return ret;
}


bool app_cfu_usb_send_boot_data(uint8_t *data, uint16_t len)
{
    uint8_t *header = NULL;
    uint16_t data_len = len;
    bool ret = false;
    DBG_DIRECT("  len = %d  ", len);
    header = malloc(data_len);
    if (!header)
    {
        APP_PRINT_ERROR0("app_cfu_usb_send_data: no memory for data");
        return ret;
    }
    memcpy(&header[0], data, len);
    app_cfu_xmit(header, data_len);
    free(header);
    return ret;
}