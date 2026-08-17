
#include <string.h>
#include <stdlib.h>
#include "os_queue.h"
#include "usb_hid_driver.h"
#include "usb_hid_desc.h"
#include "usb_spec20.h"
#include "trace.h"
#include "hid.h"
#include "usb_hid.h"
#include "usb_pipe.h"
#include "rtl_gpio.h"
#if F_APP_CFU_FEATURE_SUPPORT
#include "app_hid_descriptor.h"
#endif
#include "app_section.h"

#define REPORT_ID_CFU_FEATURE           0x2A
#define REPORT_ID_CFU_FEATURE_EX        0x2B
#define REPORT_ID_CFU_OFFER_INPUT       0x2D
#define REPORT_ID_CFU_OFFER_OUTPUT      REPORT_ID_CFU_OFFER_INPUT
#define REPORT_ID_CFU_PAYLOAD_INPUT     0x2C
#define REPORT_ID_CFU_PAYLOAD_OUTPUT    REPORT_ID_CFU_FEATURE

#define USAGE_ID_CFU_PAYLOAD_OUTPUT     0x61
#define USAGE_ID_CFU_FEATURE            0x62
#define USAGE_ID_CFU_FEATURE_EX         0x65
#define USAGE_ID_CFU_PAYLOAD_INPUT_MIN  0x66
#define USAGE_ID_CFU_PAYLOAD_INPUT_MAX  0x69
#define USAGE_ID_CFU_OFFER_INPUT_MIN    0x8A
#define USAGE_ID_CFU_OFFER_INPUT_MAX    0x8D
#define USAGE_ID_CFU_OFFER_OUTPUT_MIN   0x8E
#define USAGE_ID_CFU_OFFER_OUTPUT_MAX   0x91

#define MOUSE_REPORT_ID                 0x01
#define VENDOR_REPORT_ID                0x10
uint8_t HID_PROTOCOL2 = HID_REPORT_PROTOCOL;
//uint8_t IDLE_RATE = 0;
static const char report_descs[] =
{
#if F_APP_CFU_FEATURE_SUPPORT
    APP_CFU_HID_DESC_ATTRIB,
#endif
//    HID_REPORT_DESCS
    0x06, 0x0B, 0xFF,   // Usage Page (Vendor Defined 0xFF0B)
    0x0A, 0x04, 0x01,   // Usage (0x0104)
    0xA1, 0x01,         // Collection (Application)
    // 8-bit data
    0x15, 0x00,                         // Logical Minimum (0)
    0x26, 0xFF, 0x00,                   // Logical Maximum (255)
    0x75, 0x08,                         // Report Size (8)
    0x95, 0x3c,                         // Report Count (60)
    0x85, REPORT_ID_CFU_FEATURE,        // Report ID (0x2A)
    0x09, 0x60,                         // Usage (0x60)
    0x82, 0x02, 0x01,                   // Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Buffered Bytes)
    0x09, USAGE_ID_CFU_PAYLOAD_OUTPUT,  // Usage (0x61)
    0x92, 0x02, 0x01,                   // Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile,Buffered Bytes)
    0x09, USAGE_ID_CFU_FEATURE,         // Usage (0x62)
    0xB2, 0x02, 0x01,                   // Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile,Buffered Bytes)
    0x85, REPORT_ID_CFU_FEATURE_EX,     // Report ID (0x2B)
    0x09, USAGE_ID_CFU_FEATURE_EX,      // Usage (0x65)
    0xB2, 0x02, 0x01,                   // Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile,Buffered Bytes)
    // 32-bit data
    0x17, 0x00, 0x00, 0x00, 0x80,       // Logical Minimum (-2147483649)
    0x27, 0xFF, 0xFF, 0xFF, 0x7F,       // Logical Maximum (2147483646)
    0x75, 0x20,                         // Report Size (32)
    0x95, 0x04,                         // Report Count (4)
    0x85, REPORT_ID_CFU_PAYLOAD_INPUT,  // Report ID (0x2C)
    0x19, USAGE_ID_CFU_PAYLOAD_INPUT_MIN,// Usage Minimum (0x66)
    0x29, USAGE_ID_CFU_PAYLOAD_INPUT_MAX,// Usage Maximum (0x69)
    0x81, 0x02,                         // Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, REPORT_ID_CFU_OFFER_INPUT,    // Report ID (0x2D)
    0x19, USAGE_ID_CFU_OFFER_INPUT_MIN, // Usage Minimum (0x8A)
    0x29, USAGE_ID_CFU_OFFER_INPUT_MAX, // Usage Maximum (0x8D)
    0x81, 0x02,                         // Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x19, USAGE_ID_CFU_OFFER_OUTPUT_MIN,// Usage Minimum (0x8E)
    0x29, USAGE_ID_CFU_OFFER_OUTPUT_MAX,// Usage Maximum (0x91)
    0x91, 0x02,                         // Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,                               // End Collection

    0x05, 0x01,     /* USAGE_PAGE       (Generic Desktop) main item*/
    0x09, 0x02,     /* USAGE            (Mouse) */
    0xa1, 0x01,     /* COLLECTION       (Application) */
    0x85, MOUSE_REPORT_ID, /* REPORT_ID        (2) */
    0x09, 0x01,     /* USAGE            (Pointer) */
    0xa1, 0x00,     /* COLLECTION       (Physical) */
    0x05, 0x09,     /* USAGE_PAGE       (Button) */
    0x19, 0x01,     /* USAGE_MINIMUM    (Button 1) */
    0x29, 0x05,     /* USAGE_MAXIMUM    (Button 5) */
    0x15, 0x00,     /* LOGICAL_MINIMUM  (0) */
    0x25, 0x01,     /* LOGICAL_MAXIMUM  (1) */
    0x95, 0x05,     /* REPORT_COUNT     (5) */
    0x75, 0x01,     /* REPORT_SIZE      (1) */
    0x81, 0x02,     /* INPUT            (Data,Var,Abs) */
    0x95, 0x01,     /* REPORT_COUNT     (1) */
    0x75, 0x03,     /* REPORT_SIZE      (3) */
    0x81, 0x01,     /* INPUT            (Cnst,Var,Abs) */
    0x05, 0x01,     /* USAGE_PAGE       (Generic Desktop) main item*/
    0x09, 0x30,     /* USAGE            (X) */
    0x09, 0x31,     /* USAGE            (Y) */
    0x16, 0x01, 0x80,   /* LOGICAL_MINIMUM  (-32767) */
    0x26, 0xff, 0x7f,   /* LOGICAL_MAXIMUM  (32767) */
    0x75, 0x10,     /* REPORT_SIZE      (16) */
    0x95, 0x02,     /* REPORT_COUNT     (2) */
    0x81, 0x06,     /* INPUT            (Data,Var,Rel) */
    0x09, 0x38,     /* USAGE            (Wheel) */
    0x15, 0x81,     /* LOGICAL_MINIMUM  (-127) */
    0x25, 0x7f,     /* LOGICAL_MAXIMUM  (127) */
    0x75, 0x08,     /* REPORT_SIZE      (8) */
    0x95, 0x01,     /* REPORT_COUNT     (1) */
    0x81, 0x06,     /* INPUT            (Data,Var,Rel) */
    0x05, 0x0c,     /* USAGE_PAGE       (Consumer)*/
    0x0a, 0x38, 0x02,   /* USAGE            (AC Pan) */
    0x15, 0x81,     /* LOGICAL_MINIMUM  (-127) */
    0x25, 0x7f,     /* LOGICAL_MAXIMUM  (127) */
    0x75, 0x08,     /* REPORT_SIZE      (8) */
    0x95, 0x01,     /* REPORT_COUNT     (1) */
    0x81, 0x06,     /* INPUT            (Data,Var,Rel) */
    0xc0,           /* END_COLLECTION */
    0xc0,           /* END_COLLECTION */

    0x06, 0x01, 0xff, /* USAGE_PAGE       (vendor) */
    0x09, 0x01,     /* USAGE            (vendor) */
    0xa1, 0x01,     /* COLLECTION       (Application) */
    0x85, VENDOR_REPORT_ID, /* REPORT_ID        (0x10) */
    0x19, 0x00,     /* USAGE_MINIMUM    (0) */
    0x29, 0xff,     /* USAGE_MAXIMUM    (0xff) */
    0x15, 0x00,     /* LOGICAL_MINIMUM  (0) */
    0x26, 0xff, 0x00,     /* LOGICAL_MAXIMUM  (0xff) */
    0x75, 0x08,     /* REPORT_SIZE      (8) */
    0x95, 0x04,     /* REPORT_COUNT     (4) */
    0x81, 0x02,     /* INPUT            (Data,Var,Abs) */
    0xc0,           /* END_COLLECTION */
};

typedef struct _t_hid_ual
{
    struct _t_hid_ual *p_next;
    T_HID_CBS cbs;
} T_HID_UAL;

T_OS_QUEUE ual_list2;

static T_USB_INTERFACE_DESC hid_std_if_desc =
{
    .bLength            = sizeof(T_USB_INTERFACE_DESC),
    .bDescriptorType    = USB_DESC_TYPE_INTERFACE,
    .bInterfaceNumber   = 0,
    .bAlternateSetting  = 0,
    .bNumEndpoints      = 1,
    .bInterfaceClass    = USB_CLASS_CODE_HID,
    .bInterfaceSubClass = 1,  // Is boot interface supported
    .bInterfaceProtocol = 2,
    .iInterface         = 0,
};

static T_HID_CS_IF_DESC  hid_cs_if_desc =
{
    .bLength            = sizeof(T_HID_CS_IF_DESC),
    .bDescriptorType    = DESC_TYPE_HID,
    .bcdHID             = 0x0110,
    .bCountryCode       = 0,
    .bNumDescriptors    = 1,
    .desc[0]            =
    {
        .bDescriptorType = DESC_TYPE_REPORT,
        .wDescriptorLength = sizeof(report_descs),
    },

};

static T_USB_ENDPOINT_DESC int_in_ep_desc_fs =
{
    .bLength           = sizeof(T_USB_ENDPOINT_DESC),
    .bDescriptorType   = USB_DESC_TYPE_ENDPOINT,
    .bEndpointAddress  = USB_DIR_IN | 0x02,
    .bmAttributes      = USB_EP_TYPE_INT,
    .wMaxPacketSize    = 0x40,
    .bInterval         = 1,
};

static T_USB_ENDPOINT_DESC int_in_ep_desc_hs =
{
    .bLength           = sizeof(T_USB_ENDPOINT_DESC),
    .bDescriptorType   = USB_DESC_TYPE_ENDPOINT,
    .bEndpointAddress  = USB_DIR_IN | 0x02,
    .bmAttributes      = USB_EP_TYPE_INT,
    .wMaxPacketSize    = 0x40,
    .bInterval         = 4,
};

static void *hid_if_descs_fs[] =
{
    (void *) &hid_std_if_desc,
    (void *) &hid_cs_if_desc,
    (void *) &int_in_ep_desc_fs,
    NULL,
};

static void *hid_if_descs_hs[] =
{
    (void *) &hid_std_if_desc,
    (void *) &hid_cs_if_desc,
    (void *) &int_in_ep_desc_hs,
    NULL,
};

void *usb_hid_data_pipe_open2(uint8_t ep_addr, T_USB_HID_ATTR attr, uint8_t pending_req_num,
                              USB_HID_DATA_PIPE_CB cb)
{
    T_USB_HID_DRIVER_ATTR driver_attr;
    memcpy(&driver_attr, &attr, sizeof(T_USB_HID_DRIVER_ATTR));
    return usb_hid_driver_data_pipe_open(ep_addr, driver_attr, pending_req_num, cb);
}

APP_RAM_TEXT_SECTION
bool usb_hid_data_pipe_send2(void *handle, void *buf, uint32_t len)
{

    return usb_hid_driver_data_pipe_send(handle, buf, len);
}

int usb_hid_data_pipe_close2(void *handle)
{
    return usb_hid_driver_data_pipe_close(handle);
}

int usb_hid_ual_register2(T_HID_CBS cbs)
{
    T_HID_UAL *ual_node = malloc(sizeof(T_HID_UAL));
    memcpy(&ual_node->cbs, &cbs, sizeof(T_HID_CBS));
    os_queue_in(&ual_list2, ual_node);
    return 0;
}

int usb_hid_get_report2(uint8_t report_id, void *buf, uint16_t *len)
{
    DBG_DIRECT("usb_hid_get_report");

    uint8_t *p_data = (uint8_t *)buf;
    p_data[0] = report_id;
    p_data[1] = 0x0;
    p_data[2] = 0x1;
    *len += 1;
    APP_PRINT_INFO2("[usb_hid_get_report] report_id = 0x%x, len = %d", report_id, *len);
    return true;
}

int usb_hid_set_report2(void *buf, uint16_t len)
{
    int ret = 0;
    uint8_t *p_data = (uint8_t *)buf;
    uint8_t report_id = p_data[0];
    APP_PRINT_INFO3("[usb_hid_set_report] report_id = 0x%x, len = %d, p_data = 0x %b", report_id,
                    len, TRACE_BINARY(len, p_data));
    return ret;
}

int usb_hid_get_protocol2(uint8_t *Protocol)
{
    int ret = 0;
    *Protocol = HID_PROTOCOL2;
    APP_PRINT_INFO1("usb_hid_get_protocol = %d", *Protocol);
    return ret;
}

int usb_hid_set_protocol2(uint8_t Protocol)
{
    int ret = 0;
    HID_PROTOCOL2 = Protocol;
    APP_PRINT_INFO1("usb_hid_set_protocol = %d", Protocol);
    return ret;
}

int usb_hid_get_idle2(void)
{
    int ret = 0;
    APP_PRINT_INFO0("usb_hid_get_idle ");
    return ret;
}

int usb_hid_set_idle2(uint8_t idle_rate)
{
    int ret = 0;
//    HID_PROTOCOL = idle_rate;
    APP_PRINT_INFO1("idle_rate = %d", idle_rate);
    return ret;
}

void *inst2;
void usb_hid_init2(void)
{
    inst2 = usb_hid_driver_inst_alloc();
//    os_queue_init(&ual_list);
    usb_hid_driver_if_desc_register(inst2, (void *)hid_if_descs_hs, (void *)hid_if_descs_fs,
                                    (void *)report_descs);

    T_USB_HID_DRIVER_CBS cbs;
    cbs.get_report = usb_hid_get_report2;
    cbs.set_report = usb_hid_set_report2;
    cbs.get_protocol = usb_hid_get_protocol2;
    cbs.set_protocol = usb_hid_set_protocol2;
    cbs.get_idle = usb_hid_get_idle2;
    cbs.set_idle = usb_hid_set_idle2;

    usb_hid_driver_cbs_register(inst2, &cbs);
//    usb_hid_driver_init();
}

