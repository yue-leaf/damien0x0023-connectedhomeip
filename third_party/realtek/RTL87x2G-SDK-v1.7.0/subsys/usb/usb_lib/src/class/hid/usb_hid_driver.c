/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include <stdlib.h>
#include "usb_spec20.h"
#include "usb_composite_dev.h"
#include "hal_usb.h"
#include "usb_utils.h"
#include "rtl_errno.h"
#include "hid.h"
#include "usb_hid_driver.h"
#include "trace.h"
#include "usb_pipe.h"

static int fail_line = 0;
uint8_t IdleRate = 0;

typedef struct _ep_priv
{
    T_USB_ENDPOINT_DESC *fs_desc;
    T_USB_ENDPOINT_DESC *hs_desc;
    void *pipe;
    uint8_t ep_enable: 1;
} T_EP_PRIV;

typedef struct _idle_item
{
    struct _idle_item *p_next;
    uint8_t report_id;
    uint8_t idle_rate;
} T_IDLE_ITEM;

typedef struct _intf_priv
{
    void *report_desc;
    T_HID_CS_IF_DESC *hid_cs_if_desc;
} T_INTF_PRIV;

typedef struct _usb_hid_if
{
    struct _usb_hid_if *p_next;
    T_USB_INTERFACE intf;
    uint8_t cur_alt;
    uint8_t protocol;
    T_USB_UTILS_LIST idle_list;
    T_USB_HID_DRIVER_CBS cbs;
} T_USB_HID_IF;

typedef struct _usb_hid
{
    T_USB_UTILS_LIST if_list;
} T_USB_HID;

T_USB_HID *g_usb_hid = NULL;

int usb_hid_driver_set_report_xfer_done(T_HAL_USB_REQUEST_BLOCK *urb)
{
    UINT_OUT_FUNC set_report_cb = (UINT_OUT_FUNC)urb->priv;
    if (set_report_cb)
    {
        set_report_cb(urb->buf, urb->length);
    }
    else
    {
        USB_PRINT_WARN2("usb_hid_driver_set_report_xfer_done, buf: %x, length: %d",
                        urb->buf, urb->length);
    }
    return ESUCCESS;
}


static int usb_hid_driver_if_ctrl_request_proc(T_USB_INTERFACE *interface,
                                               T_USB_DEVICE_REQUEST *ctrl_request,
                                               T_HAL_USB_REQUEST_BLOCK *ctrl_urb)
{
    T_USB_HID_IF *hid = USB_UTILS_CONTAINER_OF(interface, T_USB_HID_IF, intf);
    T_HAL_USB_REQUEST_BLOCK      *urb = ctrl_urb;
    uint16_t                    w_index = (ctrl_request->wIndex);
    uint16_t                    w_value = (ctrl_request->wValue);
    uint16_t                    w_length = (ctrl_request->wLength);
    int status = 0;
    uint16_t length;
    length = w_length;


    USB_PRINT_INFO5("usb_hid_driver_if_ctrl_request_proc, %02x.%02x v%04x i%04x l%d",
                    ctrl_request->bmRequestType, ctrl_request->bRequest, w_value, w_index, w_length);

    switch ((ctrl_request->bRequest) | (ctrl_request->bmRequestType << 8))
    {
    case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8 | HID_REQ_GET_REPORT):
        {
            USB_PRINT_INFO3("usb_hid_driver_if_ctrl_request_proc, HID_REQ_GET_REPORT w_value %x, w_index %x, w_length %x",
                            w_value, w_index,
                            w_length);

            if (hid->cbs.get_report)
            {
                if (hid->cbs.get_report(w_value & 0xFF, urb->buf, &length) != 0)
                {
                    USB_PRINT_INFO0("!hid->cbs.get_report");
                    goto stall;
                }
            }
            else
            {
                goto stall;
            }
        }
        break;
    case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8 | HID_REQ_SET_REPORT):
        {
            USB_PRINT_INFO3("usb_hid_driver_if_ctrl_request_proc, HID_REQ_SET_REPORT w_value %x, w_index %x, w_length %x",
                            w_value, w_index,
                            w_length);

            if (hid->cbs.set_report)
            {
                urb->priv = hid->cbs.set_report;
                urb->complete = usb_hid_driver_set_report_xfer_done;
            }
            else
            {
                goto stall;
            }
        }
        break;

    case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8 | HID_REQ_GET_PROTOCOL):
        {
            USB_PRINT_INFO3("usb_hid_driver_if_ctrl_request_proc, HID_REQ_GET_PROTOCOL w_value %x, w_index %x, w_length %x",
                            w_value, w_index,
                            w_length);
            if (hid->cbs.get_protocol)
            {
                if (hid->cbs.get_protocol(&hid->protocol) != 0)
                {
                    USB_PRINT_INFO0("!hid->cbs.get_protocol");
                    goto stall;
                }
                length = USB_UTILS_MIN(length, sizeof(hid->protocol));
                memcpy(urb->buf, &hid->protocol, length);
            }
            else
            {
                goto stall;
            }
            break;
        }
    case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8 | HID_REQ_SET_PROTOCOL):
        {
            USB_PRINT_INFO3("usb_hid_driver_if_ctrl_request_proc, HID_REQ_SET_PROTOCOL w_value %x, w_index %x, w_length %x",
                            w_value, w_index,
                            w_length);
            hid->protocol = w_value;

            if (hid->cbs.set_protocol)
            {
                if (hid->cbs.set_protocol(w_value) != 0)
                {
                    USB_PRINT_INFO0("!hid->cbs.get_report");
                    goto stall;
                }
            }
            else
            {
                goto stall;
            }

            break;
        }
    case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8 | HID_REQ_GET_IDLE):
        {
            USB_PRINT_INFO3("usb_hid_driver_if_ctrl_request_proc, HID_REQ_GET_IDLE w_value %x, w_index %x, w_length %x",
                            w_value, w_index,
                            w_length);
            uint8_t report_id = w_value & 0x00FF;
            bool found = false;
            T_IDLE_ITEM *idle_item = NULL;
            USB_UTILS_LIST_FOREACH(&hid->idle_list, T_IDLE_ITEM *, idle_item)
            {
                if (idle_item->report_id == report_id)
                {
                    found = true;

                    if (hid->cbs.get_idle)
                    {
                        if (hid->cbs.get_idle(&idle_item->idle_rate) != 0)
                        {

                            USB_PRINT_INFO1("!hid->cbs.get_idle ,idle_rate = %d ", idle_item->idle_rate);
                            goto stall;
                        }
                    }

                    length = USB_UTILS_MIN(length, sizeof(idle_item->idle_rate));
                    memcpy(urb->buf, &idle_item->idle_rate, length);

                    break;
                }
            }
            if (!found)
            {
                goto stall;
            }

            break;
        }
    case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8 | HID_REQ_SET_IDLE):
        {
            USB_PRINT_INFO3("usb_hid_driver_if_ctrl_request_proc, HID_REQ_SET_IDLE w_value %x, w_index %x, w_length %x",
                            w_value, w_index,
                            w_length);
            uint8_t report_id = w_value & 0x00FF;
            uint8_t idle_rate =  w_value >> 8;
            bool found = false;
            T_IDLE_ITEM *idle_item = NULL;
            USB_UTILS_LIST_FOREACH(&hid->idle_list, T_IDLE_ITEM *, idle_item)
            {
                if (idle_item->report_id == report_id || report_id == 0)
                {
                    idle_item->idle_rate = idle_rate;
                    found = true;
                    if (report_id != 0)
                    {
                        break;
                    }
                }
            }
            if (!found)
            {
                goto stall;
            }
            else
            {
                if (hid->cbs.set_idle)
                {
                    if (hid->cbs.set_idle(idle_rate) != 0)
                    {
                        USB_PRINT_INFO0("!hid->cbs.set_report");
                        goto stall;
                    }
                }
            }
            break;
        }
    case ((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8 | USB_REQ_CODE_GET_DESCRIPTOR):
        {
            switch (w_value >> 8)
            {
            case HID_DT_HID:
                {
                    T_INTF_PRIV *intf_priv = (T_INTF_PRIV *)interface->priv;
                    USB_PRINT_INFO0("hid_if_ctrl_request_proc: USB_REQ_GET_DESCRIPTOR, HID_DT_HID");
                    length = USB_UTILS_MIN(length, intf_priv->hid_cs_if_desc->bLength);
                    memcpy(urb->buf, intf_priv->hid_cs_if_desc, length);
                    status = length;
                    goto done;
                }
            case HID_DT_REPORT: //(standard Get_descriptor, REPORT_desctiptor type)
                {
                    T_INTF_PRIV *intf_priv = (T_INTF_PRIV *)interface->priv;
                    length = USB_UTILS_MIN(length, intf_priv->hid_cs_if_desc->desc[0].wDescriptorLength);
                    memcpy(urb->buf, intf_priv->report_desc, length);
                    USB_PRINT_INFO3("usb_hid_driver_if_ctrl_request_proc, USB_REQ_GET_DESCRIPTOR: REPORT, length:%d, report_desc:%x, len:%x",
                                    length, intf_priv->report_desc, intf_priv->hid_cs_if_desc->desc[0].wDescriptorLength);
                    status = length;
                    goto done;
                }

            default:
                USB_PRINT_ERROR0("usb_hid_driver_if_ctrl_request_proc, Unknown descriptor request");
                goto stall;
            }
//            break;
        }

    default:
        goto stall;

    }

    urb->length = length;
    if (length > 0 && length < w_length)
    {
        urb->zlp = (length % usb_composite_dev_ep0_mps_get() == 0);
    }
    status = hal_usb_ep0_trx(urb->ep_handle, urb);

    if (status < 0)
    {
        USB_PRINT_ERROR0("usb_hid_driver_if_ctrl_request_proc, hal_usb_ep0_trx error");
    }

done:
    return status;

stall:
    return -EOPNOTSUPP;
}

static int usb_hid_driver_descs_parse(T_USB_INTERFACE *interface, T_USB_DESC_HDR **descs,
                                      uint8_t speed)
{
    T_USB_DESC_HDR **desc = descs;
    T_USB_EP *ep = NULL;
    T_EP_PRIV *ep_priv = NULL;
    int ret = ESUCCESS;
    bool found = false;

    for (; *desc; desc++)
    {
        if ((*desc)->bDescriptorType == USB_DESC_TYPE_INTERFACE)
        {
            //allocate interface id
            T_USB_DESC_HDR *cur_desc = *desc;
            ((T_USB_INTERFACE_DESC *)cur_desc)->bInterfaceNumber = interface->if_num;
            T_INTF_PRIV *intf_priv = (T_INTF_PRIV *)interface->priv;
            intf_priv->hid_cs_if_desc = (T_HID_CS_IF_DESC *)(*(desc + 1));
        }
        else if ((*desc)->bDescriptorType == USB_DESC_TYPE_ENDPOINT)
        {
            T_USB_DESC_HDR *ep_desc = *desc;
            USB_UTILS_LIST_FOREACH((&interface->eps), T_USB_EP *, ep)
            {
                ep_priv = ep->priv;
                if (ep_priv->fs_desc->bEndpointAddress == ((T_USB_ENDPOINT_DESC *)ep_desc)->bEndpointAddress)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                ep = malloc(sizeof(T_USB_EP));
                if (ep == NULL)
                {
                    fail_line = __LINE__;
                    ret = -ENOMEM;
                    goto end;
                }
                memset(ep, 0, sizeof(T_USB_EP));
                ep_priv = malloc(sizeof(T_EP_PRIV));
                if (ep_priv == NULL)
                {
                    fail_line = __LINE__;
                    free(ep);
                    ret = -ENOMEM;
                    goto end;
                }
                memset(ep_priv, 0, sizeof(T_EP_PRIV));
                ep->priv = ep_priv;
                ep->addr = ((T_USB_ENDPOINT_DESC *)*desc)->bEndpointAddress;
                ep->ep_handle = hal_usb_ep_handle_get(((T_USB_ENDPOINT_DESC *)*desc)->bEndpointAddress);
                USB_UTILS_LIST_INSERT_TAIL(&interface->eps, ep);
            }
            if (speed == HAL_USB_SPEED_FULL)
            {
                ep_priv->fs_desc = (T_USB_ENDPOINT_DESC *)*desc;
            }
            else if (speed == HAL_USB_SPEED_HIGH)
            {
                ep_priv->hs_desc = (T_USB_ENDPOINT_DESC *)*desc;
            }
        }
    }
end:

#if LOG_PRINT
    USB_PRINT_INFO2("usb_hid_driver_descs_parse, ret:%d, fail line:%d", ret, fail_line);
#endif
    return ret;
}

static int usb_hid_driver_report_desc_parse(T_USB_INTERFACE *interface)
{
    int ret = ESUCCESS;
    T_USB_HID_IF *hid = USB_UTILS_CONTAINER_OF(interface, T_USB_HID_IF, intf);
    T_INTF_PRIV *intf_priv = (T_INTF_PRIV *)interface->priv;
    uint8_t *report_desc = (uint8_t *)intf_priv->report_desc;
    uint16_t report_len = intf_priv->hid_cs_if_desc->desc[0].wDescriptorLength;
    T_IDLE_ITEM *idle_item = NULL;
    idle_item = malloc(sizeof(T_IDLE_ITEM));
    if (idle_item == NULL)
    {
        ret = -ENOMEM;
        return ret;
    }
    idle_item->report_id = 0;
    USB_UTILS_LIST_INSERT_TAIL(&hid->idle_list, idle_item);
    for (uint16_t i = 0; i < report_len; i++)
    {
        if (*(report_desc + i) == HID_REPORT_ID_FIELD)
        {
            idle_item = malloc(sizeof(T_IDLE_ITEM));
            if (idle_item == NULL)
            {
                ret = -ENOMEM;
                return ret;
            }
            idle_item->report_id = *(report_desc + i + 1);
            USB_UTILS_LIST_INSERT_TAIL(&hid->idle_list, idle_item);
        }
    }
    return ret;
}
static int usb_hid_driver_if_create(T_USB_INTERFACE *interface)
{
    int ret = ESUCCESS;

    ret = usb_hid_driver_descs_parse(interface, interface->descs_fs, HAL_USB_SPEED_FULL);
    ret += usb_hid_driver_descs_parse(interface, interface->descs_hs, HAL_USB_SPEED_HIGH);
    ret += usb_hid_driver_report_desc_parse(interface);
    return ret;
}
static int usb_hid_driver_if_release(T_USB_INTERFACE *interface)
{
    int ret = ESUCCESS;
    T_USB_EP *ep = NULL;
    USB_UTILS_LIST_FOREACH((&interface->eps), T_USB_EP *, ep)
    {
        if (ep->priv)
        {
            free(ep->priv);
            ep->priv = NULL;
        }
        free(ep);
    }
    return ret;
}
static int usb_hid_driver_if_alt_get(T_USB_INTERFACE *interface, T_HAL_USB_REQUEST_BLOCK *ctrl_urb)
{
    T_USB_HID_IF *hid = USB_UTILS_CONTAINER_OF(interface, T_USB_HID_IF, intf);
    memset(ctrl_urb->buf, hid->cur_alt, 1);
    USB_PRINT_INFO1("usb_hid_driver_if_alt_get, alt:%d", hid->cur_alt);
    return 1;
}

static int usb_hid_driver_if_alt_set(T_USB_INTERFACE *interface, T_HAL_USB_REQUEST_BLOCK *ctrl_urb,
                                     uint8_t alt)
{
    T_USB_HID_IF *hid = USB_UTILS_CONTAINER_OF(interface, T_USB_HID_IF, intf);
    int ret = ESUCCESS;
    T_USB_EP *ep_item = NULL;
    uint8_t speed = usb_composite_dev_enum_speed_get();

    USB_UTILS_LIST_FOREACH((&interface->eps), T_USB_EP *, ep_item)
    {
        T_USB_ENDPOINT_DESC *ep_desc = NULL;
        T_EP_PRIV *ep_priv = ep_item->priv;
        if (speed == HAL_USB_SPEED_FULL)
        {
            ep_item->desc = ep_priv->fs_desc;
        }
        else if (speed == HAL_USB_SPEED_HIGH)
        {
            ep_item->desc = ep_priv->hs_desc;
        }
        else
        {
            USB_PRINT_ERROR1("usb_hid_driver_if_alt_set, unsupported speed: %d", speed);
        }
        ep_desc = ep_item->desc;

        hal_usb_ep_disable(ep_item->ep_handle);

        ret += hal_usb_ep_enable(ep_item->ep_handle, ep_desc);
        if (ret < 0)
        {
            fail_line = __LINE__;
            goto end;
        }
        ep_priv->ep_enable = 1;
        if ((ep_desc->bEndpointAddress & USB_DIR_MASK) == USB_DIR_OUT)
        {
            usb_pipe_recv(ep_priv->pipe);
        }
    }
    hid->cur_alt = alt;
end:
    USB_PRINT_INFO4("usb_hid_driver_if_alt_set, alt = %d, interface:%d, ret:%d, fail line:%d", alt,
                    interface->if_num, ret, fail_line);
    return ret;
}

int usb_hid_driver_if_suspend(T_USB_INTERFACE *interface)
{
    int ret = ESUCCESS;
    T_USB_EP *ep = NULL;
//    T_EP_PRIV *ep_priv = NULL;
    USB_UTILS_LIST_FOREACH((&interface->eps), T_USB_EP *, ep)
    {
//        ep_priv = ep->priv;
//        ret += hal_usb_ep_disable(ep->ep_handle);
//        ep_priv->ep_enable = 0;
    }
    return ret;
}

int usb_hid_driver_if_resume(T_USB_INTERFACE *interface)
{
    int ret = ESUCCESS;
    T_USB_EP *ep = NULL;
    T_EP_PRIV *ep_priv = NULL;
    USB_UTILS_LIST_FOREACH((&interface->eps), T_USB_EP *, ep)
    {
        ep_priv = ep->priv;
        if (!ep_priv->ep_enable)
        {
            ret += hal_usb_ep_enable(ep->ep_handle, ep->desc);
            ep_priv->ep_enable = 1;
        }
        if ((ep->addr & USB_DIR_MASK) == 0)
        {
            usb_pipe_recv(ep_priv->pipe);
        }
    }
    return ret;
}
int usb_hid_driver_if_sof(T_USB_INTERFACE *interface)
{
    int ret = ESUCCESS;
    extern void usb_sof(void) ;
    usb_sof();
    return ret;
}
T_USB_EP *usb_hid_driver_ep_find_by_addr(uint8_t addr)
{
    T_USB_HID_IF *hid = NULL;
    T_USB_INTERFACE *interface = NULL;
    T_USB_EP *ep;

    USB_UTILS_LIST_FOREACH((&g_usb_hid->if_list), T_USB_HID_IF *, hid)
    {
        interface = &hid->intf;
        USB_UTILS_LIST_FOREACH((&interface->eps), T_USB_EP *, ep)
        {
            if (ep->addr == addr)
            {
                return ep;
            }
        }
    }
    return NULL;
}

void *usb_hid_driver_inst_alloc(void)
{
    T_USB_HID_IF *hid = (T_USB_HID_IF *)malloc(sizeof(T_USB_HID_IF));
    memset(hid, 0, sizeof(T_USB_HID_IF));
    return hid;
}

int usb_hid_driver_inst_free(void *inst)
{
    free(inst);
    return ESUCCESS;
}

int usb_hid_driver_if_desc_register(void *inst, void *hs_desc, void *fs_desc, void *report_desc)
{
    int ret = ESUCCESS;
    T_INTF_PRIV *intf_priv = NULL;
    T_USB_HID_IF *hid = (T_USB_HID_IF *)inst;

    hid->intf.priv = (T_INTF_PRIV *)malloc(sizeof(T_INTF_PRIV));
    if (hid->intf.priv == NULL)
    {
        fail_line = __LINE__;
        ret = -ENOMEM;
        goto end;
    }
    memset(hid->intf.priv, 0, sizeof(T_INTF_PRIV));
    hid->intf.if_num = 0;
    hid->intf.descs_fs = fs_desc;
    hid->intf.descs_hs = hs_desc;
    hid->intf.ctrl_request_proc = usb_hid_driver_if_ctrl_request_proc;
    hid->intf.create = usb_hid_driver_if_create;
    hid->intf.release = usb_hid_driver_if_release;
    //hid->intf.release = NULL;
    hid->intf.alt_get = usb_hid_driver_if_alt_get;
    hid->intf.alt_set = usb_hid_driver_if_alt_set;
    hid->intf.suspend = usb_hid_driver_if_suspend;
    hid->intf.resume = usb_hid_driver_if_resume;
    hid->intf.sof = usb_hid_driver_if_sof;
    USB_UTILS_LIST_INIT(&hid->intf.eps);
    USB_UTILS_LIST_INIT(&hid->idle_list);
    USB_UTILS_LIST_INIT((&hid->intf.eps));
    intf_priv = (T_INTF_PRIV *)hid->intf.priv;
    intf_priv->report_desc = report_desc;

    if (g_usb_hid == NULL)
    {
        g_usb_hid = (T_USB_HID *)malloc(sizeof(T_USB_HID));
        if (g_usb_hid == NULL)
        {
            fail_line = __LINE__;
            ret = -ENOMEM;
            goto end;
        }
        memset(g_usb_hid, 0, sizeof(T_USB_HID));
        USB_UTILS_LIST_INIT(&g_usb_hid->if_list);
    }
    USB_UTILS_LIST_INSERT_TAIL(&g_usb_hid->if_list, hid);

end:

#if LOG_PRINT
    USB_PRINT_INFO2("usb_hid_driver_if_desc_register, result:%d, fail line:%d", ret, fail_line);
#endif
    return ret;
}

int usb_hid_driver_if_desc_unregister(void *inst)
{
    T_USB_HID_IF *hid = NULL;
    bool found = false;
    int ret = ESUCCESS;

    USB_UTILS_LIST_FOREACH((&g_usb_hid->if_list), T_USB_HID_IF *, hid)
    {
        if (hid == inst)
        {
            found = true;
            free(hid->intf.priv);
            hid->intf.priv = NULL;
            USB_UTILS_LIST_REMOVE(&g_usb_hid->if_list, hid);
        }
    }

    if (!found)
    {
        fail_line = __LINE__;
        ret = -ENXIO;
    }

    USB_PRINT_INFO2("usb_hid_driver_inst_free, result:%d, fail line:%d", ret, fail_line);
    return ret;
}

void *usb_hid_driver_data_pipe_open(uint8_t ep_addr, T_USB_HID_DRIVER_ATTR attr,
                                    uint8_t pending_req_num, USB_HID_DRIVER_CB cb)
{
    T_USB_PIPE_ATTR pipe_attr;
    T_USB_EP *ep = usb_hid_driver_ep_find_by_addr(ep_addr);
    void *pipe = NULL;
    if (ep == NULL)
    {
        USB_PRINT_ERROR0("usb_hid_driver_data_pipe_open, ep NULL");
        return NULL;
    }

    memcpy(&pipe_attr, &attr, sizeof(T_USB_PIPE_ATTR));
    pipe = usb_pipe_open(&(ep->desc), ep_addr, pipe_attr, pending_req_num, cb);
    ((T_EP_PRIV *)ep->priv)->pipe = pipe;
    return pipe;
}

int usb_hid_driver_data_pipe_close(void *handle)
{
    return usb_pipe_close(handle);
}

USB_USER_SPEC_SECTION
int usb_hid_driver_data_pipe_send(void *handle, void *buf, uint32_t len)
{
    return usb_pipe_send(handle, buf, len);
}

int usb_hid_driver_cbs_register(void *inst, T_USB_HID_DRIVER_CBS *cbs)
{
    T_USB_HID_IF *hid = (T_USB_HID_IF *)inst;
    hid->cbs.get_report = cbs->get_report;
    hid->cbs.set_report = cbs->set_report;
    hid->cbs.set_protocol = cbs->set_protocol;
    hid->cbs.get_protocol = cbs->get_protocol;
    hid->cbs.set_idle = cbs->set_idle;
    hid->cbs.get_idle = cbs->get_idle;
    return ESUCCESS;
}

int usb_hid_driver_cbs_unregister(void *inst)
{
    T_USB_HID_IF *hid = (T_USB_HID_IF *)inst;
    memset(&hid->cbs, 0, sizeof(T_USB_HID_DRIVER_CBS));
    return ESUCCESS;
}

int usb_hid_driver_remote_wakeup(bool force)
{
    int ret = ESUCCESS;
    T_USB_EP *ep = NULL;
    T_USB_INTERFACE *interface = NULL;
    T_USB_HID_IF *hid = NULL;
    T_EP_PRIV *ep_priv = NULL;

    ret = usb_composite_dev_remote_wakeup(force);

    USB_UTILS_LIST_FOREACH((&g_usb_hid->if_list), T_USB_HID_IF *, hid)
    {
        interface = &hid->intf;
        USB_UTILS_LIST_FOREACH((&interface->eps), T_USB_EP *, ep)
        {
            ep_priv = ep->priv;
            if (!ep_priv->ep_enable)
            {
                ret += hal_usb_ep_enable(ep->ep_handle, ep->desc);
                ep_priv->ep_enable = 1;
            }
            if ((ep->addr & USB_DIR_MASK) == 0)
            {
                usb_pipe_recv(ep_priv->pipe);
            }
        }
    }

    return ret;
}

int usb_hid_driver_init(void)
{
    T_USB_HID_IF *hid = NULL;
    T_USB_INTERFACE *interface = NULL;

    USB_UTILS_LIST_FOREACH((&g_usb_hid->if_list), T_USB_HID_IF *, hid)
    {
        interface = &hid->intf;
        usb_composite_dev_interface_add(interface, 1);
    }

    return ESUCCESS;
}

int usb_hid_driver_deinit(void)
{
    T_USB_HID_IF *hid = NULL;
    T_USB_INTERFACE *interface = NULL;

    USB_UTILS_LIST_FOREACH((&g_usb_hid->if_list), T_USB_HID_IF *, hid)
    {
        interface = &hid->intf;
        usb_composite_dev_interface_remove(interface, 1);
    }

    return ESUCCESS;
}
