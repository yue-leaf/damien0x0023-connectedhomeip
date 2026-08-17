/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <string.h>

#include "trace.h"
#include "app_task.h"
#include "io_wdt.h"
#include "rtl_wdt.h"

/*============================================================================*
 *                              Variables
 *============================================================================*/

/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
  * @brief    All the application messages are pre-handled in this function
  * @note     All the IO MSGs are sent to this function, then the event handling
  *           function shall be called according to the MSG type.
  * @param[in] io_msg  IO message data
  * @return   void
  */
void app_handle_io_msg(uint32_t io_msg)
{

    switch (io_msg)
    {

    case IO_MSG_TYPE_GPIO:
        {
            APP_PRINT_INFO0("[app] app_handle_io_msg: feeding dog, restart wdg timer.");
            wdt_feed();
            //Add user code here!
        }
        break;
    default:
        break;
    }
}




/** @} */ /* End of group PERIPH_SEVER_CALLBACK */

/** @} */ /* End of group PERIPH_APP */
