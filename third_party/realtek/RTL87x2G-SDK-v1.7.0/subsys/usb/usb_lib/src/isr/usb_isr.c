#include <string.h>
#include "rtl_nvic.h"
#include "vector_table.h"
#include "hal_usb.h"
#include "usb_dm_int.h"
#include "app_section.h"
#include "trace.h"
#include "usb_isr.h"
#include "usb_utils.h"
#include "aon_reg.h"
#include "access.h"
#ifdef TARGET_RTL87X3E
#define USB_IP_IRQn         USB_IRQn
#define USB_IP_VECTORn      USB_VECTORn
#endif

static uint8_t setup_pkt[8] = {0,};
static T_USB_DEVICE_DRIVER *device_driver = NULL;
static uint8_t usb_isr_pending = 0;
static uint8_t USB_PRIORITY = 4;
typedef enum
{
    USB_ISR_MSG_UNDEFINED,
    USB_ISR_MSG_RESET,
    USB_ISR_MSG_SPEED_ENUM_DONE,
    USB_ISR_MSG_SETUP,
    USB_ISR_MSG_SUSPEND,
    USB_ISR_MSG_RESUME,
    USB_ISR_MSG_SOF,
    USB_ISR_MSG_XFER_DONE,
} T_USB_ISR_MSG;

void usb_isr_device_driver_register(T_USB_DEVICE_DRIVER *driver)
{
    device_driver = driver;
}

void usb_isr_device_driver_unregister(T_USB_DEVICE_DRIVER *driver)
{
    device_driver = NULL;
}

USB_USER_SPEC_SECTION
static void usb_common_isr_enter(void)
{
#if LOG_PRINT
    USB_PRINT_INFO0("usb_common_isr_enter");
#endif
    NVIC_DisableIRQ(USB_IRQn);
}




static void usb_common_isr_handler(T_HAL_USB_COMMON_ISR isr, T_HAL_USB_ISR_PARAM *param)
{
    T_USB_TASK_MSG msg = {.group = USB_TASK_MSG_GROUP_HAL, .type = USB_ISR_MSG_UNDEFINED, };
    bool msg_send = true;

    switch (isr)
    {
    case HAL_USB_COMMON_ISR_RESET:
        {
            msg.type = USB_ISR_MSG_RESET;
        }
        break;

    case HAL_USB_COMMON_ISR_ENUM_DONE:
        {
            msg.type = USB_ISR_MSG_SPEED_ENUM_DONE;
            msg.var[0] = param->enum_done.speed;
        }
        break;

    case HAL_USB_COMMON_ISR_SETUP:
        {
            msg.type = USB_ISR_MSG_SETUP;
            memcpy(setup_pkt, param->setup.setup_pkt, 8);
            msg.complex.buf = setup_pkt;
            msg.complex.len = sizeof(setup_pkt);
        }
        break;

    case HAL_USB_COMMON_ISR_SUSPEND:
        {
//            usb_dm_suspend_enter();
            msg.type = USB_ISR_MSG_SUSPEND;
//            msg_send = false;

        }
        break;

    case HAL_USB_COMMON_ISR_RESUME:
        {
            msg.type = USB_ISR_MSG_RESUME;
        }
        break;
    case HAL_USB_COMMON_ISR_SOF:
        {
            msg.type = USB_ISR_MSG_SOF;
        }
        break;
    case HAL_USB_COMMON_ISR_XFER_DONE:
        {
            msg.type = USB_ISR_MSG_XFER_DONE;
            msg.complex.buf = (uint8_t *)param->xfer_done.urb;
            msg.complex.len = sizeof(T_HAL_USB_REQUEST_BLOCK);
        }
        break;

    default:
        msg_send = false;
        break;
    }

    if (msg_send)
    {
#if LOG_PRINT
        USB_PRINT_INFO1("usb_common_isr_handler:%d", msg.group);
#endif
        usb_task_msg_send(&msg);
        usb_isr_pending++;
    }
}

USB_USER_SPEC_SECTION
static void usb_common_isr_exit(void)
{
#if LOG_PRINT
    USB_PRINT_INFO1("usb_common_isr_exit ,usb_isr_pending = %d", usb_isr_pending);
#endif
    if (usb_isr_pending == 0)
    {
        NVIC_EnableIRQ(USB_IRQn);
    }
}

void usb_isr_msg_handle(T_USB_TASK_MSG *usb_msg)
{
    T_USB_ISR_MSG type = (T_USB_ISR_MSG)usb_msg->type;
    if (usb_dm_state_get() == USB_PDN)
    {
        USB_PRINT_INFO1("usb_isr_msg_handle, usb already power down:%d", usb_isr_pending);
        usb_isr_pending--;
        return;
    }
    switch (type)
    {
    case USB_ISR_MSG_RESET:
        {
            if (device_driver && device_driver->reset)
            {
                device_driver->reset();
            }
        }
        break;

    case USB_ISR_MSG_SPEED_ENUM_DONE:
        {
            uint8_t speed = (uint8_t)usb_msg->var[0];
            if (device_driver && device_driver->speed_enum_done)
            {
                device_driver->speed_enum_done(speed);
            }
        }
        break;

    case USB_ISR_MSG_SETUP:
        {
            if (device_driver && device_driver->setup)
            {
                uint8_t *setup_pkt = usb_msg->complex.buf;
                device_driver->setup(setup_pkt);
            }
        }
        break;

    case USB_ISR_MSG_SUSPEND:
        {
            if (device_driver && device_driver->suspend)
            {
                device_driver->suspend();
            }
            usb_dm_suspend_enter();

        }
        break;

    case USB_ISR_MSG_RESUME:
        {
            if (device_driver && device_driver->resume)
            {
                device_driver->resume();
//                extern T_USB_POWER_STATE usb_pwr_state_before_suspend;
//                USB_PRINT_INFO1("usb_dm_state_set(usb_pwr_state_before_suspend) = %d" ,usb_pwr_state_before_suspend );
//                usb_dm_state_set(usb_pwr_state_before_suspend);
            }
        }
        break;
    case USB_ISR_MSG_SOF:
        {
            if (device_driver && device_driver->sof)
            {
                device_driver->sof();
            }
        }
        break;
    case USB_ISR_MSG_XFER_DONE:
        {
            T_HAL_USB_REQUEST_BLOCK *urb = (T_HAL_USB_REQUEST_BLOCK *)usb_msg->complex.buf;
            if (urb && urb->complete)
            {
                urb->complete(urb);
            }
        }
        break;

    default:
        break;
    }
#if LOG_PRINT
    USB_PRINT_INFO2("usb_isr_msg_handle:%d, %d", type, usb_isr_pending);
#endif
    usb_isr_pending--;
    if (usb_isr_pending == 0)
    {
        NVIC_EnableIRQ(USB_IRQn);
    }
}


APP_RAM_TEXT_SECTION
void usb_suspendn_isr_handler(void)
{
    usb_dm_suspend_exit();
}


void Utmi_Suspend_N_Handler(void)
{
    USB_PRINT_INFO0("Utmi_Suspend_N_Handler");
    SoC_VENDOR->u_004.REG_LOW_PRI_INT_STATUS |= BIT31;
    /* prevent false alarm */
    uint8_t usb_dm_state = usb_dm_state_get();

    if (usb_dm_state == USB_SUSPENDED)
    {
        USB_PRINT_INFO0("USB Resume event");
        usb_dm_suspend_exit();
    }

}

void usb_isr_suspendn_enable(void)
{
#ifdef _USB_SUPPORT_DLPS_
    AON_REG7X_SYS_TYPE reg7x;
    reg7x.d32 = HAL_READ32(SYSTEM_REG_BASE, AON_REG7X_SYS);
//    DBG_DIRECT("usb debug 1 %x: ", reg7x.d32);
    reg7x.usb_wakeup_sel = 1;
    reg7x.USB_WKPOL = 0;
    reg7x.USB_WKEN = 1;
    HAL_WRITE32(SYSTEM_REG_BASE, AON_REG7X_SYS, reg7x.d32);
//    reg7x.d32 = HAL_READ32(SYSTEM_REG_BASE, AON_REG7X_SYS);
//    DBG_DIRECT("usb debug 2 %x: ", reg7x.d32);
#endif
    // 0x04
    // BIT31 => for suspend_n interrupt
    //SoC_VENDOR->u_004.REG_LOW_PRI_INT_STATUS
    // 0x08
    SoC_VENDOR->u_008.REG_LOW_PRI_INT_MODE |= BIT31;// edge trigger
    // 0x18, pol
    SoC_VENDOR->u_018.INTERRUPT_EDGE_OPTION &= ~(BIT31); // resing edge trigger
    // 0x0C
    SoC_VENDOR->u_00C.REG_LOW_PRI_INT_EN |= BIT31;  // Note: must disable at disablw flow
    //  __disable_irq();
    extern void Utmi_Suspend_N_Handler(void);
    RamVectorTableUpdate(USB_SUSPEND_N_VECTORn, (IRQ_Fun)Utmi_Suspend_N_Handler);

    NVIC_InitTypeDef nvic_init_struct;
    nvic_init_struct.NVIC_IRQChannel         = USB_SUSPEND_N_IRQn;
    nvic_init_struct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    nvic_init_struct.NVIC_IRQChannelPriority = 4;
    NVIC_Init(&nvic_init_struct);
}

void usb_isr_suspendn_disable(void)
{
    // 0x0C
    SoC_VENDOR->u_00C.REG_LOW_PRI_INT_EN &= (~BIT31);

    NVIC_InitTypeDef nvic_init_struct;
    nvic_init_struct.NVIC_IRQChannel         = USB_SUSPEND_N_IRQn;
    nvic_init_struct.NVIC_IRQChannelCmd      = (FunctionalState)DISABLE;
    nvic_init_struct.NVIC_IRQChannelPriority = 6;
    NVIC_Init(&nvic_init_struct);
}

//USB_USER_SPEC_SECTION
static const HAL_USB_COMMON_ISR_HOOKS usb_common_isr_hooks =
{
    .enter = usb_common_isr_enter,
    .handler = usb_common_isr_handler,
    .exit = usb_common_isr_exit,
};

static HAL_USB_SUSPENDN_ISR_HOOKS usb_suspendn_isr_hooks =
{
    .enter = NULL,
    .handler = usb_suspendn_isr_handler,
    .exit = NULL,
};

void usb_isr_enable(void)
{
    hal_usb_common_isr_handler_update((HAL_USB_COMMON_ISR_HOOKS *)&usb_common_isr_hooks);
    hal_usb_suspendn_isr_handler_update((HAL_USB_SUSPENDN_ISR_HOOKS *)&usb_suspendn_isr_hooks);
    NVIC_SetPriority(USB_IRQn, USB_PRIORITY);
    NVIC_EnableIRQ(USB_IRQn);
    usb_isr_suspendn_enable();
    hal_usb_global_isr_enable();
}
void usb_sof_interrupt(bool enable)
{
    hal_usb_sof_interrupt_config(enable);
}
void usb_isr_set_priority(uint8_t priority)
{
    NVIC_SetPriority(USB_IRQn, priority);
    USB_PRIORITY = priority;
//    NVIC_EnableIRQ(USB_IRQn);
}

void usb_isr_disable(void)
{
    hal_usb_global_isr_disable();
    usb_isr_suspendn_disable();
    NVIC_DisableIRQ(USB_IRQn);
}
void usb_register_system_resource_by_project(void)
{
//    USB_PRINT_TRACE0("usb_register_system_resource");
//    NVIC_DisableIRQ(USB_IRQn);

//    // normal OTG IP interrupt
//    extern void USB_Handler(void);
//    //  __disable_irq();
//    RamVectorTableUpdate(USB_VECTORn, (IRQ_Fun)USB_Handler);
//    NVIC_InitTypeDef nvic_init_struct;
//    nvic_init_struct.NVIC_IRQChannel         = USB_IRQn;
//    nvic_init_struct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
//    nvic_init_struct.NVIC_IRQChannelPriority = 4;
////    NVIC_SetIRQNonSecure(nvic_init_struct.NVIC_IRQChannel);
//    NVIC_Init(&nvic_init_struct);


#ifndef _TODO_BEE3PRO_SUSPENDN_BUG_
    usb_isr_suspendn_enable();
#endif

}