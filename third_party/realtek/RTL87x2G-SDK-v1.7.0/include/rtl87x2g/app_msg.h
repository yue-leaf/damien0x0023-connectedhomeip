/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _APP_MSG_H_
#define _APP_MSG_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
  * @defgroup APP_MSG APP Message
  * @brief Message definition for user application task.
  * @{
  */

/**
  * @defgroup APP_MSG_Exported_Types APP Message Exported Types
  * @{
  */

/**
 * @brief     Event group definitions.
 * @details   Event code is 1 byte code to define messages type exchanging from/to different layers.
 *            First half byte is for group define, and second half byte is for code define.
 *            Group code:
 *            0: from stack layer
 *            0x1: from peripheral layer
 *            0x2: from audio lib
 *            0xA: from instrument
 */
typedef enum
{
    EVENT_GROUP_STACK = 0x00,       /**< message group from BT layer */
    EVENT_GROUP_IO = 0x01,          /**< message group from IO layer */
    EVENT_GROUP_FRAMEWORK = 0x02,   /**< message group from Framework layer */
    EVENT_GROUP_APP = 0x03,         /**< message group from APP layer */
    EVENT_GROUP_INSTRUMENT = 0x0A,  /**< message group from instrument layer */
} T_EVENT_GROUP;

/**  @brief     Event type definitions.
*/
typedef enum
{
    EVENT_GAP_MSG = 0x01,  /**< message from GAP layer for stack */
    EVENT_GAP_TIMER = 0x02, /**< message from GAP layer for timer */
    EVENT_HCI_MSG   = 0x03, /**< message from HCI layer for test */

    EVENT_IO_TO_APP = 0x11, /**< message from IO to user application */

    EVENT_SYS_MSG = 0x21,   /**< message from SYS timer to user application */
    EVENT_DSP_MSG = 0x22,    /**< message from DSP to user application */
    EVENT_CODEC_MSG = 0x23,  /**< message from CODEC to user application */
    EVENT_ANC_MSG           = 0x24, /**< message from ANC to user application */
    EVENT_NOTIFICATION_MSG  = 0x25, /**< message from Notification to user application */
    EVENT_BIN_LOADER_MSG = 0x26,    /**< message from bin load to user application */
    EVENT_AUDIO_PLUGIN_MSG  = 0x27, /**< message from external device to Audio Plugin */
    EVENT_AUDIO_PATH_MSG    = 0x28, /**< message from Audio Plugin to Audio Path */
    EVENT_APP_TIMER_MSG = 0x30,     /**< message from APP timer to user application */
    EVENT_CMD_FROM_8852B = 0xA1,    /**< message from 8852B to user application */
} T_EVENT_TYPE;

/** @brief Get event code group definition */
#define EVENT_GROUP(event_code) (event_code >> 4)

/**  @brief IO type definitions for IO message, may extend as requested. */
typedef enum
{
    IO_MSG_TYPE_BT_STATUS,  /**< BT status change with subtype @ref GAP_MSG_TYPE */
    IO_MSG_TYPE_KEYSCAN,    /**< Key scan message with subtype @ref T_IO_MSG_KEYSCAN */
    IO_MSG_TYPE_QDECODE,    /**< subtype to be defined */
    IO_MSG_TYPE_UART,       /**< Uart message with subtype @ref T_IO_MSG_UART */
    IO_MSG_TYPE_KEYPAD,     /**< subtype to be defined */
    IO_MSG_TYPE_IR,         /**< subtype to be defined */
    IO_MSG_TYPE_GDMA,       /**< subtype to be defined */
    IO_MSG_TYPE_ADC,        /**< subtype to be defined */
    IO_MSG_TYPE_D3DG,       /**< subtype to be defined */
    IO_MSG_TYPE_SPI,        /**< subtype to be defined */
    IO_MSG_TYPE_MOUSE_BUTTON,   /**< subtype to be defined */
    IO_MSG_TYPE_GPIO,       /**< Gpio message with subtype @ref T_IO_MSG_GPIO */
    IO_MSG_TYPE_MOUSE_SENSOR,   /**< subtype to be defined */
    IO_MSG_TYPE_PPT_DFU,
    IO_MSG_TYPE_TIMER,      /**< App timer message with subtype @ref T_IO_MSG_TIMER */
    IO_MSG_TYPE_RESET_WDG_TIMER, /**< reset watchdog timer */
    IO_MSG_TYPE_RESET_AON_WDG_TIMER, /**< reset AON watchdog timer */
    IO_MSG_TYPE_WRISTBNAD,  /**< wristband message with subtype @ref T_IO_MSG_WRISTBAND */
    IO_MSG_TYPE_MESH_STATUS,    /**< subtype to be defined */
    IO_MSG_TYPE_KEYBOARD_BUTTON, /**< subtype to be defined */
    IO_MSG_TYPE_ANCS,            /**< ANCS message */
    IO_MSG_TYPE_CONSOLE,    /**< Console message with subtype @ref T_IO_CONSOLE */
    IO_MSG_TYPE_LE_MGR,
    IO_MSG_TYPE_GAP_TOUCH,/**< Cap Touch message with subtype @ref T_IO_MSG_GAP_TOUCH */
    IO_MSG_TYPE_LATENCY,
    IO_MSG_TYPE_WORK_MODE_MONITOR,
    IO_MSG_TYPE_OS_MONITOR,
    IO_MSG_TYPE_BAT_DETECT,
    IO_MSG_TYPE_USB_DEV,
    IO_MSG_TYPE_USB_UAC,
    IO_MSG_TYPE_USB_HID,
    IO_MSG_TYPE_USB_CDC,
    IO_MSG_TYPE_BAT_LPC,        /**< LPC send low power message */
    IO_MSG_TYPE_LE_AUDIO,
    IO_MSG_TYPE_REPORT_RATE_CHANGE,
    IO_MSG_TYPE_LED_ON,
    IO_MSG_TYPE_LED_OFF,
    IO_MSG_TYPE_ADV_ON,
    IO_MSG_TYPE_ADV_OFF,
    IO_MSG_TYPE_MESH_LIGHT,    /**< mesh light message */
    IO_MSG_TYPE_DPI,
    IO_MSG_TYPE_PPT_RECEIVE,
    IO_MSG_TYPE_FMNA,           /**< Findmy specific message with subtype @ref T_IO_MSG_TYPE_FMNA */
    IO_MSG_TYPE_DRIVER_DATA,
    IO_MSG_TYPE_IR_START_SEND_REPEAT_CODE, /**< IR send repeat code message */
    IO_MSG_TYPE_IR_SEND_COMPLETE,    /**< IR send complete message */
    IO_MSG_TYPE_IR_LEARN_DATA,       /**< IR learn data message */
    IO_MSG_TYPE_IR_LEARN_STOP,       /**< IR learn stop message */
    IO_MSG_TYPE_START_PPT_RECONNECT,
    IO_MSG_TYPE_DFU,
    IO_MSG_TYPE_DFU_VALID_FW,
    IO_MSG_TYPE_USB_WAKEUP,     /**< USB remote wakeup event with subtype @ref T_IO_MSG_USB_WAKEUP_EVENT */
    IO_MSG_TYPE_OTHERS,         /**< other message for general purpose */
    IO_MSG_TYPE_CAN_SEND,
    IO_MSG_TYPE_CAN_RECEIVE,
    IO_MSG_TYPE_ELECTRIC_METER,
    IO_MSG_TYPE_GUI,
    IO_MSG_TYPE_ESL_COMMS,
    IO_MSG_TYPE_PERIPHERALS_HAL,
    IO_MSG_TYPE_ZB_LIGHT,        /**< Zigbee light control message */
} T_IO_MSG_TYPE;

/**  @brief IO subtype definitions for @ref T_IO_CONSOLE type */
typedef enum
{
    IO_MSG_CONSOLE_STRING_RX    = 0x01, /**< Console CLI RX event */
    IO_MSG_CONSOLE_STRING_TX    = 0x02, /**< Console CLI TX event */
    IO_MSG_CONSOLE_BINARY_RX    = 0x03, /**< Console protocol RX event */
    IO_MSG_CONSOLE_BINARY_TX    = 0x04, /**< Console protocol TX event */
} T_IO_CONSOLE;

/**  @brief IO subtype definitions for @ref IO_MSG_TYPE_KEYSCAN type */
typedef enum
{
    IO_MSG_KEYSCAN_RX_PKT        = 1, /**< Keyscan RX data event */
    IO_MSG_KEYSCAN_MAX           = 2, /**<  */
    IO_MSG_KEYSCAN_ALLKEYRELEASE = 3, /**< All keys are released event */
    IO_MSG_KEYSCAN_STUCK         = 4, /**< key stuck message */
} T_IO_MSG_KEYSCAN;

/**  @brief IO subtype definitions for @ref IO_MSG_TYPE_UART type */
typedef enum
{
    IO_MSG_UART_RX              = 0x10, /**< UART RX event */

    IO_MSG_UART_TX              = 0x20, /**< UART TX event */

    IO_MSG_UART_RX_DONE         = 0x21, /**< UART RX done event */

    IO_MSG_UART_DATA_PARSER_SUCCESS = 0x22, /**< UART data parser done event */

    IO_MSG_UART_DATA_PARSER_FAILED = 0x23, /**< UART data parser done event */
} T_IO_MSG_UART;

/**  @brief IO subtype definitions for @ref IO_MSG_TYPE_GPIO type */
typedef enum
{
    IO_MSG_GPIO_KEY,               /**< KEY GPIO event */
    IO_MSG_GPIO_UART_WAKE_UP,      /**< UART WAKE UP event */
    IO_MSG_GPIO_CHARGER,           /**< CHARGER event */
    IO_MSG_GPIO_NFC,               /**< NFC event */
    IO_MSG_GPIO_USB_AUDIO_PLUG,    /**< USB PLUG IN event */
    IO_MSG_GPIO_USB_AUDIO_UNPLUG,  /**< USB PLUG OUT event */
    IO_MSG_GPIO_ADAPTOR_PLUG,      /**< ADAPTOR PLUG IN event */
    IO_MSG_GPIO_ADAPTOR_UNPLUG,    /**< ADAPTOR PLUG OUT event */
    IO_MSG_GPIO_CHARGERBOX_DETECT, /**< CHARGERBOX DETECT event */
    IO_MSG_GPIO_CASE_DETECT,       /**< CASE DETECT event */
    IO_MSG_GPIO_USB_AUDIO_CHECK,   /**< USB CHECK event */
    IO_MSG_GPIO_GSENSOR,           /**< Gsensor detect event */
    IO_MSG_GPIO_SENSOR_LD,         /**< sensor light detect event */
    IO_MSG_GPIO_SENSOR_LD_IO_DETECT,/**< sensor light IO detect event */
    IO_MSG_GPIO_ADAPTOR_DAT,       /**< ADAPTOR DATA event */
    IO_MSG_GPIO_RTK_CHARGERBOX_DETECT, /**< CHARGERBOX COMMAND event */
    IO_MSG_GPIO_FMNA_TRIG_PRESS,   /**< Findmy TRIG PIN press event */
    IO_MSG_GPIO_FMNA_TRIG_RELEASE, /**< Findmy TRIG PIN release event */
    IO_MSG_GPIO_FMNA_CUST_ADV_PIN, /**< Findmy customized PIN event */
} T_IO_MSG_GPIO;

/**  @brief IO subtype definitions for @ref IO_MSG_TYPE_GAP_TOUCH type */
typedef enum
{
    IO_MSG_GAP_TOUCH_CH_INFO,           /**< CHANNEL event */
    IO_MSG_GAP_TOUCH_OVER_N_NOISE,      /**< N_NOISE event */
    IO_MSG_GAP_TOUCH_OVER_P_NOISE,      /**< P_NOISE event */
} T_IO_MSG_GAP_TOUCH;

/**  @brief IO subtype definitions for @ref IO_MSG_TYPE_TIMER type */
typedef enum
{
    IO_MSG_TIMER_ALARM,
    IO_MSG_TIMER_RWS
} T_IO_MSG_TIMER;

/**  @brief IO subtype definitions for @ref IO_MSG_TYPE_FMNA type */
typedef enum
{
    IO_MSG_FMNA_KEYROLL,
    IO_MSG_FMNA_DISCONNECT,
    IO_MSG_FMNA_RECONNECT_TIMEOUT,
    IO_MSG_FMNA_UT_START,
    IO_MSG_FMNA_SN_READ,
    IO_MSG_FMNA_MT,
    IO_MSG_FMNA_RESET,
} T_IO_MSG_TYPE_FMNA;
/**  @brief IO subtype definitions for @ref IO_MSG_TYPE_MESH_LIGHT type */
typedef enum
{
    IO_MSG_MESH_LIGHT_OFF_GUI,        /**< MESH LIGHT TURN OFF FROM UI event */
    IO_MSG_MESH_LIGHT_ON_GUI,         /**< MESH LIGHT TURN ON FROM UI event */
} T_IO_MSG_MESH_LIGHT;

/**  @brief IO subtype definitions for @ref IO_MSG_TYPE_USB_WAKEUP type */
typedef enum
{
    IO_MSG_USB_WAKEUP_ENTER_DLPS,       /**< Trigger DONGLE Enter DLPS event */
    IO_MSG_USB_WAKEUP_EXIT_DLPS         /**< Notify  DONGLE Exit  DLPS event */
} T_IO_MSG_USB_WAKEUP_EVENT;

/**  @brief IO subtype definitions for @ref IO_MSG_TYPE_ZB_LIGHT type */
typedef enum
{
    IO_MSG_ZB_LIGHT_ONOFF,  /**< Zigbee OnOff control, param = 0 (off) / 1 (on) */
    IO_MSG_ZB_LIGHT_LEVEL,  /**< Zigbee Level control, param = level value (0-254) */
} T_IO_MSG_ZB_LIGHT;

/**  @brief IO message definition for communications between tasks*/
typedef struct
{
    uint16_t type;
    uint16_t subtype;
    union
    {
        uint32_t  param;
        void     *buf;
    } u;
} T_IO_MSG;


/** End of APP_MSG_Exported_Types
  * @}
  */

/** End of APP_MSG
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* _APP_MSG_H_ */
