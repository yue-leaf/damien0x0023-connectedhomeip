/**
*****************************************************************************************
*     Copyright(c) 2020, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     ppt_pf.h
  * @brief    Head file for 2.4G module platform related driver.
  * @details  data structs and external functions declaration.
  * @author   bill
  * @date     2023-10-27
  * @version  v0.1
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _PPT_PF_H_
#define _PPT_PF_H_

/* Add Includes here */
#include "debug_port.h"

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

/** @addtogroup PPT_PF
  * @{
  */

/** @defgroup PPT_PF_Exported_Macros Exported Macros
  * @brief
  * @{
  */
#ifndef PPT_PF_TIMER_NUM
#define PPT_PF_TIMER_NUM                    2 //!< Number of ppt platform timers.
#endif
/** @} End of PPT_PF_Exported_Macros */

/** @defgroup PPT_PF_Exported_Types Exported Types
  * @brief
  * @{
  */

#if PPT_PF_TIMER_NUM
/**
  * @brief PPT platform timer callback function pointer type.

  * @param[in] idx: Timer index.
  */
typedef void (*ppt_pf_timer_cb_t)(uint8_t idx);
#endif
/** @} End of PPT_PF_Exported_Types */

/** @defgroup PPT_PF_Exported_Functions Exported Functions
  * @brief
  * @{
  */

/**
  * @brief Initialize the ppt platform timer.
  *
  * The ppt platform timer is a special hardware timer who is bonded to the radio mac.

  * @param[in] idx: Timer index.
  */
void ppt_pf_timer_init(uint8_t idx);

/**
  * @brief Start the ppt platform timer.

  * @param[in] idx: Timer index.
  * @param[in] period_us: Timer period.
  * @param[in] cb: Timer timeout callback.
  */
void ppt_pf_timer_start(uint8_t idx, uint32_t period_us, ppt_pf_timer_cb_t cb);

/**
  * @brief Stop ppt platform timer.

  * @param[in] idx: Timer index.
  */
void ppt_pf_timer_stop(uint8_t idx);

/** @} End of PPT_PF_Exported_Functions */

/** @} End of PPT_PF */

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif
