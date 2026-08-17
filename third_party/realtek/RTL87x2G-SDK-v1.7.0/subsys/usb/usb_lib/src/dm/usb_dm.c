#include <stddef.h>
#include "os_task.h"
#include "rtl_nvic.h"
#include "rtl_errno.h"
#include "hal_usb.h"
#include "usb_dm.h"
#include "app_section.h"
#include "trace.h"
#include "usb_isr.h"
#include "usb_task.h"
#include "usb_dm_int.h"
#include "usb_bc12_api.h"
#include "os_sync.h"
#include <string.h>
#include "usb_composite_dev.h"
//#include "usb_bc12_api.h"
//#if TARGET_RTL87X3D
#include "pm.h"
//#endif

static T_USB_POWER_STATE usb_pwr_state = USB_PDN;
T_USB_POWER_STATE usb_pwr_state_before_suspend = USB_POWERED;
static USB_DM_CB usb_dm_cb = NULL;
static bool remote_wakeup_enable = false;

typedef enum
{
    USB_DM_MSG_START,
    USB_DM_MSG_STOP,
} T_USB_DM_MSG;

APP_RAM_TEXT_SECTION
int usb_dm_state_set(T_USB_POWER_STATE state)
{
    if (state != usb_pwr_state)
    {
        USB_PRINT_INFO2("usb_dm_state_set, pre:%d, new:%d", usb_pwr_state, state);
        usb_pwr_state = state;

        if (usb_dm_cb)
        {
            T_USB_DM_EVT_PARAM evt_param;
            memset(&evt_param, 0, sizeof(T_USB_DM_EVT_PARAM));
            evt_param.status_ind.state = state;
            /*reset done*/
            if (state == USB_DEFAULT)
            {
                uint8_t speed = (usb_composite_dev_enum_speed_get() == HAL_USB_SPEED_FULL) ? USB_SPEED_FULL :
                                USB_SPEED_HIGH;
                evt_param.status_ind.info.speed = speed;
            }
            else if (state == USB_SUSPENDED)
            {
                evt_param.status_ind.info.remote_wakeup_en = remote_wakeup_enable;
            }
            usb_dm_cb(USB_DM_EVT_STATUS_IND, &evt_param);
        }
    }

    return ESUCCESS;
}

APP_RAM_TEXT_SECTION
T_USB_POWER_STATE usb_dm_state_get(void)
{
    return usb_pwr_state;
}

APP_RAM_TEXT_SECTION
int usb_dm_remote_wakeup_enable(void)
{
    remote_wakeup_enable = true;
    return ESUCCESS;
}

APP_RAM_TEXT_SECTION
int usb_dm_remote_wakeup_disable(void)
{
    remote_wakeup_enable = false;
    return ESUCCESS;
}

//#if TARGET_RTL87X3D
void usb_dm_start_from_dlps(void)
{
    usb_dm_suspend_exit();
    usb_isr_enable();
    return;
}

POWER_CheckResult usb_dm_power_check(void)
{
    uint8_t usb_dm_state = usb_dm_state_get();

    if (usb_dm_state == USB_SUSPENDED || usb_dm_state == USB_PDN)
    {
        return POWER_CHECK_PASS;
    }
    return POWER_CHECK_FAIL;
}

void usb_dm_dlps_store(void)
{
//    USB_PRINT_TRACE0("usb_dm_dlps_store");
    hal_usb_wakeup_status_clear();
}

void usb_dm_dlps_restore(void)
{
    if (hal_usb_wakeup_status_get() == 0)
    {
//        USB_PRINT_TRACE0("usb_dm_dlps_restore: error flow");
    }
    else
    {
//        USB_PRINT_TRACE0(" USB DLPS restore: call usb_start_from_dlps_patch");

        usb_dm_start_from_dlps();
    }
//    usb_isr_enable();
}

void usb_dm_register_dlps_cb(void)
{
    usb_set_pon_domain();
    power_check_cb_register(usb_dm_power_check);// register cb

    //extern void usb_dlps_restore(void);
    power_stage_cb_register(usb_dm_dlps_store, POWER_STAGE_STORE);
    power_stage_cb_register(usb_dm_dlps_restore, POWER_STAGE_RESTORE);// register cb
}
//#endif

void usb_dm_start(bool bc12_detect)
{
    T_USB_TASK_MSG msg = {.group = USB_TASK_MSG_GROUP_DM, .type = USB_DM_MSG_START, .var[0] = bc12_detect};
    usb_task_msg_send(&msg);
}

void usb_dm_core_init(T_USB_CORE_CONFIG config)
{
    hal_usb_speed_set((T_HAL_USB_SPEED)config.speed);
    hal_usb_init();
//#if TARGET_RTL87X3D
    usb_dm_register_dlps_cb();
//#endif
    usb_task_create();
}
void usb_dm_core_deinit(void)
{
    hal_usb_deinit();
    usb_task_release();
}
int usb_dm_suspend_enter(void)
{
    uint32_t s;
    s = os_lock();
    hal_usb_suspend_enter();
    usb_pwr_state_before_suspend = usb_dm_state_get();
    usb_dm_state_set(USB_SUSPENDED);
    os_unlock(s);

    return ESUCCESS;
}

APP_RAM_TEXT_SECTION
int usb_dm_suspend_exit(void)
{
    uint32_t s;
    s = os_lock();
    if (usb_dm_state_get() == USB_SUSPENDED)
    {
        if (hal_usb_suspend_exit() != 0)
        {
            usb_dm_state_set(USB_PDN);
        }
        else
        {
            extern T_USB_POWER_STATE usb_pwr_state_before_suspend;
            usb_dm_state_set(usb_pwr_state_before_suspend);
        }
    }
    extern  void usb_register_system_resource_by_project(void);
    usb_register_system_resource_by_project();

    os_unlock(s);

    return ESUCCESS;
}

void usb_dm_stop(void)
{
    T_USB_TASK_MSG msg = {.group = USB_TASK_MSG_GROUP_DM, .type = USB_DM_MSG_STOP, };
    usb_task_msg_send(&msg);

}
int usb_dm_msg_handle(T_USB_TASK_MSG *usb_msg)
{
    T_USB_DM_MSG type = (T_USB_DM_MSG)usb_msg->type;
    switch (type)
    {
    case USB_DM_MSG_START:
        {
            USB_PRINT_INFO0("USB_DM_MSG_START");
            bool bc12_detect = (bool)usb_msg->var[0];
            if (bc12_detect)
            {
                uint8_t bc12_type = usb_bc12_type_get();
                if (usb_dm_cb)
                {
                    T_USB_DM_EVT_PARAM evt_param;
                    memset(&evt_param, 0, sizeof(T_USB_DM_EVT_PARAM));
                    evt_param.bc12_det.type = bc12_type;
                    usb_dm_cb(USB_DM_EVT_BC12_DETECT, &evt_param);
                }
                if ((bc12_type  != BC12_TYPE_SDP_0P5A) &&
                    (bc12_type  != BC12_TYPE_CDP_1P5A))
                {
                    return -EFAULT;
                }
            }
            if (usb_dm_state_get() == USB_PDN)
            {
                usb_dm_state_set(USB_ATTACHED);
                hal_usb_phy_power_on();
                usb_dm_state_set(USB_POWERED);
                hal_usb_mac_init();
                usb_isr_enable();
                hal_usb_soft_attach();
            }

        }
        break;
    case USB_DM_MSG_STOP:
        {
            USB_PRINT_INFO0("USB_DM_MSG_STOP");
            if (usb_dm_state_get() == USB_SUSPENDED)
            {
                usb_composite_dev_remote_wakeup(0);
            }

            if (usb_dm_state_get() >= USB_POWERED)
            {

                hal_usb_soft_detach();
                usb_isr_disable();
                hal_usb_mac_deinit();
                hal_usb_phy_power_down();
                usb_dm_state_set(USB_PDN);

            }
        }
        break;
    default:
        break;
    }
    USB_PRINT_INFO1("usb_dm_msg_handle: %d", type);
    return 0;
}


void usb_dm_cb_register(USB_DM_CB cb)
{
    usb_dm_cb = cb;
}

