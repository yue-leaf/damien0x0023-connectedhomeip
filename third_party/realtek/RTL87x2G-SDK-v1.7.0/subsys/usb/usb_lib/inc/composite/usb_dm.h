#ifndef __USB_DM__
#define __USB_DM__
#include <stdbool.h>
#include <stdint.h>
//#include "usb_task.h"

/** \addtogroup USB_DM
  * \brief USB device manager.
  * @{
  */

/** @defgroup USB_Dev_Manager_Exported_Constants USB Device Manager Exported Constants
  * @{
  */

/**
 * usb_dm.h
 *
 * \brief   USB device manager event used in \ref USB_DM_CB.
 *
 */
typedef enum
{
    USB_DM_EVT_STATUS_IND                   = 0,
    USB_DM_EVT_BC12_DETECT                  = 1,
} T_USB_DM_EVT;

/**
 * usb_dm.h
 *
 * \brief   The USB power state obtained from event \ref T_USB_DM_EVT.
 *
 */
typedef enum  {USB_PDN = 0,
               USB_ATTACHED = 1,
               USB_POWERED = 2,
               USB_DEFAULT = 3,
               USB_ADDRESSED = 4,
               USB_CONFIGURED = 5,
               USB_SUSPENDED = 6,
              } T_USB_POWER_STATE;

/** End of group USB_Dev_Manager_Exported_Constants
  * @}
  */

/** @defgroup USB_Dev_Manager_Exported_Types USB Device Manager Exported Types
  * @{
  */

/**
 * \brief USB device manager event parameter status indication.
 * \param state: The current state.
 * \param info: Specific information depends on \ref state.
 *              If state is \ref USB_DEFAULT, it indicates the actual enumeration speed.
 * \param speed:  Indicates the actual enumeration speed.
 * \param remote_wakeup_en:  Indicates the remote wakeup status.
 *
 */
typedef struct _usb_dm_evt_param_status_ind
{
    T_USB_POWER_STATE state;
    union
    {
        uint8_t speed;
        uint8_t remote_wakeup_en;
    } info;
} T_USB_DM_EVT_PARAM_STATUS_IND;

/**
 * \brief The event parameter of bc12 detect.
 * \param type: The bc12 detect result.
 */

typedef struct _usb_dm_evt_param_bc12_det
{
    uint8_t type;
} T_USB_DM_EVT_PARAM_BC12_DET;

/**
 * \brief The event parameters.
 *
 */
typedef union _usb_dm_evt_param
{
    T_USB_DM_EVT_PARAM_STATUS_IND status_ind;
    T_USB_DM_EVT_PARAM_BC12_DET bc12_det;
} T_USB_DM_EVT_PARAM;


/**
 * usb_dm.h
 *
 * \brief  The USB device manager callback.
 *
 * \param T_USB_DM_EVT USB device manager event defined in \ref T_USB_DM_EVT.
 *
 * \param uint32_t Optional parameters depending on different event.
 *
 */
typedef bool (*USB_DM_CB)(T_USB_DM_EVT, T_USB_DM_EVT_PARAM *);

/**
 * usb_dm.h
 *
 * \brief   USB settings such as speed, etc.
 *
 */
typedef struct _t_usb_core_config
{
    uint8_t speed;
    struct
    {
        uint8_t uac_enable: 1;
        uint8_t hid_enable: 1;
        uint8_t rsv: 6;
    } class_set;
} T_USB_CORE_CONFIG;

/** End of group USB_Dev_Manager_Exported_Types
  * @}
  */

/** @defgroup USB_Dev_Manager_Exported_Functions USB Device Manager Exported Functions
  * @{
  */

/**
 * usb_dm.h
 *
 * \brief   The definition of USB speed.
 *
 */
#define USB_SPEED_FULL  0
/**
 * usb_dm.h
 *
 * \brief   The definition of USB speed.
 *
 */
#define USB_SPEED_HIGH  1

/**
 * usb_dm.h
 *
 * \brief  Initialize the USB core.
 *
 * \param config USB core settings in \ref T_USB_CORE_CONFIG.
 *
 */
void usb_dm_core_init(T_USB_CORE_CONFIG config);
/**
 * usb_dm.h
 *
 * \brief  Deinit the USB core.
 *
 * \param config USB core settings in \ref T_USB_CORE_CONFIG.
 *
 */
void usb_dm_core_deinit(void) ;
/**
 * usb_dm.h
 *
 * \brief  Start the USB core, this api will start the USB task.
 *
 * \param bc12_detect Do the bc12 detect if it is true.
 *
 */
void usb_dm_start(bool bc12_detect);


/**
 * usb_dm.h
 *
 * \brief  Stop the USB core, this api will stop the USB task.
 *
 */
void usb_dm_stop(void);

/**
 * usb_dm.h
 *
 * \brief   Register the USB device manager callback.
 *
 *
 * \param cb USB dm callback \ref USB_DM_CB.
 *
 */
void usb_dm_cb_register(USB_DM_CB cb);

/**
 * usb_dm.h
 *
 * \brief   Unregister the USB device manager callback.
 *
 */
void usb_dm_cb_unregister(void);

/** @} */ /* End of group USB_Dev_Manager_Exported_Functions */
/** @}*/
/** End of USB_DM
*/
#endif
