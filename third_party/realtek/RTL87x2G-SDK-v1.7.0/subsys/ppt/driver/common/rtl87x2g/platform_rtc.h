/**
*********************************************************************************************************
*               Copyright(c) 2022, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file         platform_rtc.h
* @brief        Platfrom RTC implementation head file.
* @details
* @author       js.lin
* @date         2022-8-31
* @version      v0.1
*********************************************************************************************************
*/

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef __PLATFORM_RTC_H
#define __PLATFORM_RTC_H

/*============================================================================*
 *                               Header Files
 *============================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include "rtl876x.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PF_RTC_BIT_NUM
#define PF_RTC_BIT_NUM              BIT64(32)
#endif

/*============================================================================*
 *                              Types
 *============================================================================*/
/** @defgroup PLATFORM_RTC_Exported_Types Platform RTC Exported Types
  * @{
  */

typedef enum PFRTCComparator
{
    PF_RTC_COMP0        = 0x01,
    PF_RTC_COMP1        = 0x02,
    PF_RTC_COMP2        = 0x04,
    PF_RTC_COMP3        = 0x08,
} PFRTCComparator;

/** @} */ /* End of group PLATFORM_RTC_Exported_Types */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup PLATFORM_RTC_Exported_Functions Platform RTC Exported Functions
  * @{
  */

extern void (*platform_rtc_aon_init)(void);
extern void (*platform_rtc_non_aon_init)(void);
extern void (*platform_rtc_reset)(void);
extern void (*platform_rtc_run_cmd)(bool);
extern uint32_t (*platform_rtc_get_counter)(void);
extern void (*platform_rtc_set_comp_intr_config)(PFRTCComparator, bool, bool);
extern void (*platform_rtc_set_comp_wk_config)(PFRTCComparator, bool);
extern void (*platform_rtc_set_intr_group_config)(bool);
extern void (*platform_rtc_set_wk_group_config)(bool);
extern bool (*platform_rtc_get_intr_group_config)(void);
extern bool (*platform_rtc_get_wk_group_config)(void);
extern void (*platform_rtc_set_comp)(PFRTCComparator, uint32_t);
extern uint32_t (*platform_rtc_get_comp)(PFRTCComparator);
extern PFRTCComparator(*platform_rtc_get_comp_intr_status)(void);
extern PFRTCComparator(*platform_rtc_get_comp_wk_status)(void);
extern void (*platform_rtc_set_comp_manual_intr)(PFRTCComparator);
extern PFRTCComparator(*platform_rtc_get_comp_manual_intr)(void);
extern void (*platform_rtc_clear_comp_intr_status)(PFRTCComparator);
extern void (*platform_rtc_clear_comp_wk_status)(PFRTCComparator);

/** @} */ /* End of group PLATFORM_RTC_Exported_Functions */

#ifdef __cplusplus
}
#endif

#endif /* __PLATFORM_RTC_H */
