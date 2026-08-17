/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef IO_DLPS_H
#define IO_DLPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "board.h"
#include "rtl876x.h"

#if (PLATFORM_SUPPORT_POWER_MANAGER == 0)
#include "pm.h"
#endif

/**
  * @defgroup IO_DLPS IO DLPS
  * @brief IO DLPS driver module
  * @{
  */

/** @defgroup IO_DLPS_Exported_Types IO DLPS Exported Types
  * @{
  */

typedef void (*DLPS_IO_ExitDlpsCB)(void);
typedef void (*DLPS_IO_EnterDlpsCB)(void);

/** End of group IO_DLPS_Exported_Types
  * @}
  */



/*============================================================================*
 *                         Functions
 *============================================================================*/


/** @defgroup IO_DLPS_Exported_Functions IO DLPS Exported Functions
  * \ingroup  IO_DLPS
  * @{
  */

/**
  * @brief  Register io restore function in dlps mode
  */
extern void DLPS_IORegister(void);

#if USE_USER_DEFINE_DLPS_EXIT_CB

extern DLPS_IO_ExitDlpsCB User_IO_ExitDlpsCB;

/**
  * @brief  This API registers a user-defined callback for DLPS exit stage, which allows IO restore action by APP.
  * @param  func: User-defined DLPS exit stage callback function.
  */
__STATIC_INLINE void DLPS_IORegUserDlpsExitCb(DLPS_IO_ExitDlpsCB func)
{
    User_IO_ExitDlpsCB = func;
}

#endif /* USE_USER_DEFINE_DLPS_EXIT_CB */

#if USE_USER_DEFINE_DLPS_ENTER_CB

extern DLPS_IO_EnterDlpsCB User_IO_EnterDlpsCB;

/**
  * @brief  This API registers a user-defined callback for DLPS enter stage, which allows IO store action by APP.
  *         Do not perform time-consuming operations in the DLPS enter callback, as it may disturb the DLPS wake-up process.
  *         Since the OS schedule and interrupts are disabled during DLPS, it is forbidden to use OS APIs in the DLPS enter callback function.
  * @param  func: User-defined DLPS enter stage callback function.
  */
__STATIC_INLINE void DLPS_IORegUserDlpsEnterCb(DLPS_IO_EnterDlpsCB func)
{
    User_IO_EnterDlpsCB = func;
}

#endif

/** @} */ /* End of group IO_DLPS_Exported_Functions */
/** @} */ /* End of group IO_DLPS */

#ifdef __cplusplus
}
#endif

#endif /* IO_DLPS_H */


