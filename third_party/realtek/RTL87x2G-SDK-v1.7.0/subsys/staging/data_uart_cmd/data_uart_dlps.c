#include <trace.h>
#include <board.h>
#include <rtl_pinmux.h>


#if F_BT_DLPS_EN

#include "pm.h"
#include "io_dlps.h"

extern void *evt_queue_handle;
extern void *io_queue_handle;

bool can_enter_dlps = true;

void data_uart_dlps_exit_cb(void)
{
    Pad_ControlSelectValue(DATA_UART_TX_PIN, PAD_PINMUX_MODE);
    Pad_ControlSelectValue(DATA_UART_RX_PIN, PAD_PINMUX_MODE);
    data_uart_init(evt_queue_handle, io_queue_handle);
}

void data_uart_dlps_enter_cb(void)
{
    Pad_ClearAllWakeupINT();
    Pad_ControlSelectValue(DATA_UART_TX_PIN, PAD_SW_MODE);
    Pad_ControlSelectValue(DATA_UART_RX_PIN, PAD_SW_MODE);
    System_WakeUpPinEnable(DATA_UART_RX_PIN, PAD_WAKEUP_POL_LOW, PAD_WAKEUP_DEB_DISABLE);
}

POWER_CheckResult data_uart_dlps_check_cb(void)
{
    if (can_enter_dlps)
    {
        return POWER_CHECK_PASS;
    }
    else
    {
        return POWER_CHECK_FAIL;
    }
}

void System_Handler(void)
{
    if (System_WakeUpInterruptValue(DATA_UART_RX_PIN) == SET)
    {
        can_enter_dlps = false;
        Pad_ClearWakeupINTPendingBit(DATA_UART_RX_PIN);
    }
}

void data_uart_can_enter_dlps(bool enter)
{
    can_enter_dlps = enter;
}

void data_uart_dlps_init(void)
{
    if (power_check_cb_register(data_uart_dlps_check_cb) == false)
    {
        APP_PRINT_ERROR0("data_uart_dlps_init: power_check_cb_register register failed");
    }
    DLPS_IORegUserDlpsEnterCb(data_uart_dlps_enter_cb);
    DLPS_IORegUserDlpsExitCb(data_uart_dlps_exit_cb);
    DLPS_IORegister();
}
#endif

