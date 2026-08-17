#include <stddef.h>
#include "usb_composite_dev.h"
#include "usb_dev_driver.h"

void usb_dev_driver_dev_desc_register(T_USB_DEVICE_DESC *desc)
{
    usb_composite_dev_init(desc);
}

void usb_dev_driver_string_desc_register(T_STRING_TAB *string_tbl[])
{
    uint16_t language = 0;
    uint8_t id = 0;
    const char *s = NULL;

    while (*string_tbl)
    {
        T_STRING *strings = (*string_tbl)->strings;
        language = (*string_tbl)->language;
        while (strings)
        {
            id = strings->id;
            s = strings->s;
            if (id != 0)
            {
                usb_composite_dev_string_add(language, id, s);
            }
            else
            {
                break;
            }
            strings++;
        }
        string_tbl++;
    }
}

void usb_dev_driver_dev_desc_unregister(void)
{
    usb_composite_dev_deinit();
}

void usb_dev_driver_string_desc_unregister(T_STRING_TAB *string_tbl[])
{
    uint16_t language = 0;
    uint8_t id = 0;
    const char *s = NULL;

    while (*string_tbl)
    {
        T_STRING *strings = (*string_tbl)->strings;
        language = (*string_tbl)->language;
        while (strings)
        {
            id = strings->id;
            s = strings->s;
            if (id != 0)
            {
                usb_composite_dev_string_remove(language, id, s);
            }
            else
            {
                break;
            }
            strings++;
        }
        string_tbl++;
    }
}

void usb_dev_driver_cfg_desc_register(T_USB_CONFIG_DESC *desc)
{
    usb_composite_dev_cfg_add(desc);
}

void usb_dev_driver_cfg_desc_unregister(T_USB_CONFIG_DESC *desc)
{
    usb_composite_dev_cfg_remove(desc);
}
static USB_SPD_CB usb_spd_cb = NULL;
static USB_SOF_CB usb_sof_cb = NULL;
void usb_spd_set(uint8_t speed)
{
    if (usb_spd_cb)
    {
        usb_spd_cb(speed);
    }

}
void usb_sof(void)
{
    if (usb_sof_cb)
    {
        usb_sof_cb();
    }
}
uint32_t usb_get_frame_number(void)
{
    return (hal_get_usb_frame_number());
}
void usb_spd_cb_register(USB_SPD_CB cb)
{
    usb_spd_cb = cb;
}
void usb_sof_cb_register(USB_SOF_CB cb)
{
    usb_sof_cb = cb;
}
