/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _RTL_LPC_H_
#define _RTL_LPC_H_

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "utils/rtl_utils.h"
#if defined (CONFIG_SOC_SERIES_RTL87X2G)
#include "lpc/src/rtl87x2g/rtl_lpc_def.h"
#elif defined (CONFIG_SOC_SERIES_RTL8762J)
#include "lpc/src/rtl87x2j/rtl_lpc_def.h"
#endif


/** \defgroup LPC         LPC
  * \brief
  * \{
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/
/** \defgroup LPC_Exported_Constants LPC Exported Constants
  * \brief
  * \{
  */

/**
 * \defgroup    LPC_Channel LPC Channel
 * \{
 * \ingroup     LPC_Exported_Constants
 */
typedef enum
{
    LPC_CHANNEL_ADC0,    //!< LPC channel is adc0.
    LPC_CHANNEL_ADC1,    //!< LPC channel is adc1.
    LPC_CHANNEL_ADC2,    //!< LPC channel is adc2.
    LPC_CHANNEL_ADC3,    //!< LPC channel is adc3.
    LPC_CHANNEL_ADC4,    //!< LPC channel is adc4.
    LPC_CHANNEL_ADC5,    //!< LPC channel is adc5.
    LPC_CHANNEL_ADC6,    //!< LPC channel is adc6.
    LPC_CHANNEL_ADC7,    //!< LPC channel is adc7.
    LPC_CHANNEL_VBAT,    //!< LPC channel is vbat.
    LPC_CHANNEL_MAX,     //!< LPC channel maximum number.
} LPCChannelSel_TypeDef;

#define IS_LPC_CHANNEL(CHANNEL) (((CHANNEL) < LPC_CHANNEL_MAX)) //!< Specify the maximum LPC channel number.

/** End of LPC_Channel
  * \}
  */

/**
 * \defgroup    LPC_Edge LPC Edge
 * \{
 * \ingroup     LPC_Exported_constants
 */
typedef enum
{
    LPC_Vin_Below_Vth,    //!< The input voltage is below threshold voltage.
    LPC_Vin_Over_Vth,     //!< The input voltage is over threshold voltage.
} LPCEdge_TypeDef;

#define IS_LPC_EDGE(EDGE) (((EDGE) == LPC_Vin_Below_Vth) || \
                           ((EDGE) == LPC_Vin_Over_Vth)) //!< Check if the input parameter is valid.

/** End of LPC_Edge
  * \}
  */

#if LPC_SUPPORT_TRIGER_MODE
/**
 * \defgroup    LPC_Mode LPC Mode
 * \{
 * \ingroup     LPC_Exported_constants
 */
typedef enum
{
    LPC_DIVIDE_MODE,   //!< LPC divide mode.
    LPC_BYPASS_MODE,   //!< LPC bypass mode.
} LPCMode_TypeDef;

#define IS_LPC_MODE(MODE) (((MODE) == LPC_BYPASS_MODE) || \
                           ((MODE) == LPC_DIVIDE_MODE)) //!< Check if the input parameter is valid.

/** End of LPC_Mode
  * \}
  */

/**
 * \defgroup    LPC_Trigger_Mode LPC Trigger Mode
 * \{
 * \ingroup     LPC_Exported_constants
 */
typedef enum
{
    LPC_SINGLE_OUTPUT,   //!< LPC single output.
    LPC_DUAL_OUTPUT,     //!< LPC dual output.
} LPCTriggerMode_TypeDef;

#define IS_LPC_TRIGGER_MODE(MODE) (((MODE) == LPC_SINGLE_OUTPUT) || \
                                   ((MODE) == LPC_DUAL_OUTPUT)) //!< Check if the input parameter is valid.

/** End of LPC_Trigger_Mode
  * \}
  */

#endif

/**
 * \defgroup    LPC_Debounce_Divider LPC Debounce Divider
 * \{
 * \ingroup     LPC_Exported_constants
 */
typedef enum
{
    LPC_DEBOUNCE_DIV_1,     //!< The debounce divider is 1.
    LPC_DEBOUNCE_DIV_2,     //!< The debounce divider is 2.
    LPC_DEBOUNCE_DIV_4,     //!< The debounce divider is 4.
    LPC_DEBOUNCE_DIV_8,     //!< The debounce divider is 8.
    LPC_DEBOUNCE_DIV_16,    //!< The debounce divider is 16.
    LPC_DEBOUNCE_DIV_32,    //!< The debounce divider is 32.
    LPC_DEBOUNCE_DIV_40,    //!< The debounce divider is 40.
    LPC_DEBOUNCE_DIV_64,    //!< The debounce divider is 64.
} LPCDebDiv_TypeDef;

#define IS_LPC_DEB_DIV(DIV) (((DIV) == LPC_DEBOUNCE_DIV_1) || \
                             ((DIV) == LPC_DEBOUNCE_DIV_2) || \
                             ((DIV) == LPC_DEBOUNCE_DIV_4) || \
                             ((DIV) == LPC_DEBOUNCE_DIV_8) || \
                             ((DIV) == LPC_DEBOUNCE_DIV_16) || \
                             ((DIV) == LPC_DEBOUNCE_DIV_32) || \
                             ((DIV) == LPC_DEBOUNCE_DIV_40) || \
                             ((DIV) == LPC_DEBOUNCE_DIV_64)) //!< Check if the input parameter is valid.

/** End of LPC_Debounce_Divider
  * \}
  */


#if LPC_SUPPORT_RAP_MODE
/**
 * \defgroup    LPC_Task TIM Task
 * \{
 * \ingroup     LPC_Exported_Constants
 */
typedef enum
{
    LPC_TASK_START = 0,
    LPC_TASK_STOP = 1,
} LPCTask_TypeDef;
/** End of LPC_Task
  * \}
  */

/**
 * \defgroup    LPC_Shortcut_Task LPC Shortcut Task
 * \{
 * \ingroup     LPC_Exported_Constants
 */
typedef enum
{
    LPC_SHORTCUT_TASK_STOP = 1,
} LPCShortcutTask_TypeDef;

/** End of LPC_Shortcut_Task
  * \}
  */

/**
 * \defgroup    LPC_Shortcut_Event LPC Shortcut Event
 * \{
 * \ingroup     LPCExported_Constants
 */
typedef enum
{
    LPC_SHORTCUT_EVENT_STATUS = 0,
} LPCShortcutEvent_TypeDef;

/** End of LPC_Shortcut_Event
  * \}
  */
#endif
/** End of LPC_Exported_Constants
  * \}
  */

/*============================================================================*
 *                         Types
 *============================================================================*/
/** \defgroup LPC_Exported_Types LPC Exported Types
  * \brief
  * \{
  */

/**
 * \brief       LPC init structure definition
 *
 * \ingroup     LPC_Exported_Types
 */
typedef struct
{
    LPCChannelSel_TypeDef LPC_Channel;       /*!< Specify the input LPC channel.
                                                  This parameter can be a value of \ref LPC_Channel. */

    LPCEdge_TypeDef LPC_Edge;                /*!< Specify the comparator output edge.
                                                  This parameter can be a value of \ref LPC_Edge. */
    LPCThreshold_TypeDef
    LPC_Threshold;                           /*!< Specify the threshold value of comparator voltage.
                                                  This parameter can be a value of \ref LPC_Threshold. */
#if  LPC_SUPPORT_DEBOUNCE_MODE
    FunctionalState LPC_DebouncEn;           /*!< Specify the LPC Debounce.
                                                  This parameter can be a value of DISABLE or ENABLE. */

    LPCDebDiv_TypeDef LPC_DebouncDiv;        /*!< Specify the LPC Debounce Divider.
                                                  This parameter can be a value of \ref LPC_Debounce_Divider. */

    uint32_t LPC_DebouncCnt;                 /*!< Set lpcomp debounce time debounce. Debounce time=(LPC_CLK/(LPC_DebouncCnt+1)). The LPC_CLK for the LPC is typically 32k.
                                                  This parameter can be a value of 0 ~ 255. */
#endif
#if  LPC_SUPPORT_TRIGER_MODE
    LPCMode_TypeDef LPC_Mode;                /*!< Specify the LPC mode.
                                                  This parameter can be a value of \ref LPC_Mode. */

    uint8_t LPC_ThresholdL;                  /*!< Specify the threshold value of comparator voltage. */

    LPCTriggerMode_TypeDef LPC_TrigerMode;   /*!< Specify the LPC0 triger mode.
                                                  This parameter can be a value of \ref LPC_Trigger_Mode. */
#endif
} LPC_InitTypeDef;

/** End of LPC_Exported_Types
  * \}
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/
/** \defgroup LPC_Exported_Constants LPC Exported Constants
  * \brief
  * \{
  */

/**
 * \defgroup    LPC_Flags_Definition    LPC Flags Definition
 * \{
 * \ingroup     LPC_Exported_Constants
 */
#define LPC_FLAG_LPCOMP                     (BIT0)     //!< Voltage detection interrupt flag.
#define LPC_FLAG_LPCOMP_AON                 (BIT1)     //!< Wakeup voltage detection interrupt flag.

#define IS_LPC_FLAG(FLAG) (((FLAG) == LPC_FLAG_LPCOMP_AON) || \
                           ((FLAG) == LPC_FLAG_LPCOMP))     //!< Check if the input parameter is valid.
#define IS_LPC_CLEAR_FLAG(FLAG) ((FLAG) == LPC_FLAG_LPCOMP) //!< Check if the input parameter is valid.

/** End of LPC_Flags_Definition
  * \}
  */

/** End of LPC_Exported_Constants
  * \}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** \defgroup LPC_Exported_Functions LPC Exported Functions
  * \brief
  * \{
  */

/**
 * \brief  Reset LPC.
 *
 * \param[in] LPCx  Select the LPC peripheral. Refer to \ref LPC_Declaration.
 */
void LPC_DeInit(LPC_TypeDef *LPCx);

/**
 * \brief   Initialize LPC peripheral according to
 *          the specified parameters in LPC_InitStruct.
 *
 * \param[in] LPCx            Select the LPC peripheral. Refer to \ref LPC_Declaration.
 * \param[in] LPC_InitStruct  Pointer to a LPC_InitTypeDef structure which will be initialized.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_lpc_init(void)
 * {
 *     LPC_InitTypeDef LPC_InitStruct;
 *
 *     LPC_StructInit(&LPC_InitStruct);
 *     LPC_InitStruct.LPC_Channel   = LPC_CAPTURE_CHANNEL;
 *     LPC_InitStruct.LPC_Edge      = LPC_VOLTAGE_DETECT_EDGE;
 *     LPC_InitStruct.LPC_Threshold = LPC_VOLTAGE_DETECT_THRESHOLD;
 *     LPC_Init(LPC0,&LPC_InitStruct);
 *     LPC_Cmd(LPC0,ENABLE);
 *
 *     LPC_INTConfig(LPC_INT_LPCOMP, ENABLE);
 *
 *     NVIC_InitTypeDef NVIC_InitStruct;
 *     NVIC_InitStruct.NVIC_IRQChannel = LPC_IRQn;
 *     NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
 *     NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
 *     NVIC_Init(&NVIC_InitStruct);

 *     LPC_NvCmd(LPC0,ENABLE);
 * }
 * \endcode
 */
void LPC_Init(LPC_TypeDef *LPCx, LPC_InitTypeDef *LPC_InitStruct);

/**
 * \brief  Fill each LPC_InitStruct member with its default value.
 *
 * \note The default settings for the LPC_InitStruct member are shown in the following table:
 *       | LPC_InitStruct member        | Default value                        |
 *       |:----------------------------:|:------------------------------------:|
 *       | LPC_Channel                  | \ref LPC_CHANNEL_ADC0                |
 *       | LPC_Edge                     | \ref LPC_Vin_Below_Vth               |
 *       | LPC_DebouncEn                | DISABLE                              |
 *       | LPC_DebouncDiv               | \ref LPC_DEBOUNCE_DIV_1              |
 *       | LPC_DebouncCnt               | 0                                    |
 *       | LPC_Threshold                | \ref LPC_2000_mV                     |
 *
 * \param[in]  LPCx            Select the LPC peripheral. Refer to \ref LPC_Declaration.
 * \param[in]  LPC_InitStruct  Pointer to a LPC_InitTypeDef structure which will be initialized.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_lpc_init(void)
 * {
 *     LPC_InitTypeDef LPC_InitStruct;
 *
 *     LPC_StructInit(&LPC_InitStruct);
 *     LPC_InitStruct.LPC_Channel   = LPC_CAPTURE_CHANNEL;
 *     LPC_InitStruct.LPC_Edge      = LPC_VOLTAGE_DETECT_EDGE;
 *     LPC_InitStruct.LPC_Threshold = LPC_VOLTAGE_DETECT_THRESHOLD;
 *     LPC_Init(LPC0,&LPC_InitStruct);
 * }
 * \endcode
 */
void LPC_StructInit(LPC_InitTypeDef *LPC_InitStruct);

/**
 * \brief  Enable or disable LPC peripheral.
 *
 * \param[in]  LPCx      Select the LPC peripheral. Refer to \ref LPC_Declaration.
 * \param[in]  NewState  New state of LPC peripheral.
 *                       This parameter can be one of the following values:
 *                       - ENABLE: Enable LPC voltage detect.
 *                       - DISABLE: Disable LPC voltage detect.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_lpc_init(void)
 * {
 *     LPC_InitTypeDef LPC_InitStruct;
 *
 *     LPC_StructInit(&LPC_InitStruct);
 *     LPC_InitStruct.LPC_Channel   = LPC_CAPTURE_CHANNEL;
 *     LPC_InitStruct.LPC_Edge      = LPC_VOLTAGE_DETECT_EDGE;
 *     LPC_InitStruct.LPC_Threshold = LPC_VOLTAGE_DETECT_THRESHOLD;
 *     LPC_Init(LPC0, &LPC_InitStruct);
 *     LPC_Cmd(LPC0,ENABLE);
 * }
 * \endcode
 */
void LPC_Cmd(LPC_TypeDef *LPCx, FunctionalState NewState);

/**
 * \brief  Enable or disable the specified LPC interrupts.
 *
 * \param[in] LPCx      Select the LPC peripheral. Refer to \ref LPC_Declaration.
 * \param[in] LPC_INT   Specify the LPC interrupt source to be enabled or disabled.
 *                      This parameter can be one of the following values:
 *                      \arg LPC_INT_LPCOMP: Voltage detection interrupt.
 *                      \arg LPC_INT_LPCOMP_AON: Wakeup voltage detection interrupt.
 * \param[in] NewState  New state of the specified LPC interrupt.
 *                      This parameter can be one of the following values:
 *                      - ENABLE: Enable the LPC interrupt.
 *                      - DISABLE: Disable the LPC interrupt.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_lpc_init(void)
 * {
 *     LPC_InitTypeDef LPC_InitStruct;
 *
 *     LPC_StructInit(&LPC_InitStruct);
 *     LPC_InitStruct.LPC_Channel   = LPC_CAPTURE_CHANNEL;
 *     LPC_InitStruct.LPC_Edge      = LPC_VOLTAGE_DETECT_EDGE;
 *     LPC_InitStruct.LPC_Threshold = LPC_VOLTAGE_DETECT_THRESHOLD;
 *     LPC_Init(&LPC_InitStruct);
 *     LPC_Cmd(LPC0,ENABLE);
 *
 *     LPC_INTConfig(LPC0,LPC_INT_LPCOMP, ENABLE);
 *
 *     NVIC_InitTypeDef NVIC_InitStruct;
 *     NVIC_InitStruct.NVIC_IRQChannel = LPC0_IRQn;
 *     NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
 *     NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
 *     NVIC_Init(&NVIC_InitStruct);

 *     LPC_NvCmd(LPC0,ENABLE);
 * }
 * \endcode
 */
void LPC_INTConfig(LPC_TypeDef *LPCx, uint32_t LPC_INT, FunctionalState NewState);

/**
 * \brief     Enable or disable wakeup signal to power sequence.
 *
 * \param[in] LPCx      Select the LPC peripheral. Refer to \ref LPC_Declaration.
 * \param[in] NewState  New state of the specified LPC wakeup interrupt.
 *                      This parameter can be one of the following values:
 *                      - ENABLE: Enable LPC wakeup signal to power sequence.
 *                      - DISABLE: Disable LPC wakeup signal to power sequence.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_lpc_init(void)
 * {
 *     LPC_InitTypeDef LPC_InitStruct;
 *
 *     LPC_StructInit(&LPC_InitStruct);
 *     LPC_InitStruct.LPC_Channel   = LPC_CAPTURE_CHANNEL;
 *     LPC_InitStruct.LPC_Edge      = LPC_VOLTAGE_DETECT_EDGE;
 *     LPC_InitStruct.LPC_Threshold = LPC_VOLTAGE_DETECT_THRESHOLD;
 *     LPC_Init(LPC0,&LPC_InitStruct);
 *     LPC_Cmd(LPC0,ENABLE);
 *
 *     LPC_INTConfig(LPC0, LPC_INT_LPCOMP, ENABLE);
 *     LPC_INTCmd(LPC0,ENABLE);
 *     LPC_WKCmd(LPC0,ENABLE);
 *
 *     NVIC_InitTypeDef NVIC_InitStruct;
 *     NVIC_InitStruct.NVIC_IRQChannel = RTC_IRQn;
 *     NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
 *     NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
 *     NVIC_Init(&NVIC_InitStruct);
 * }
 * \endcode
 */
void LPC_WKCmd(LPC_TypeDef *LPCx, FunctionalState NewState);

/**
 * \brief  Get the specified LPC interrupt flag status.
 *
 * \param[in] LPCx      Select the LPC peripheral. Refer to \ref LPC_Declaration.
 * \param[in] LPC_FLAG  Specify the LPC interrupt flags.
 *                      This parameter can be one of the following values:
 *                      \arg LPC_FLAG_LPCOMP: Voltage detection interrupt flag.
 *                      \arg LPC_FLAG_LPCOMP_AON: Wakeup voltage detection interrupt flag.
 *
 * \return  The status of \ref LPC_Flags_Definition.
 *          - SET: The LPC flag has been set.
 *          - RESET: The LPC flag has been reset.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * if (LPC_GetFlagStatus(LPC0, LPC_FLAG_LPCOMP_AON) == SET)
 * {
 *     DBG_DIRECT("LPC AON");
 * }
 * \endcode
 */
FlagStatus LPC_GetFlagStatus(LPC_TypeDef *LPCx, uint32_t LPC_FLAG);

/**
 * \brief  Clear the specified LPC flag.
 *
 * \param[in] LPCx      Select the LPC peripheral. Refer to \ref LPC_Declaration.
 * \param[in] LPC_FLAG  Specify the LPC flag.
 *                      This parameter can be one of the following values:
 *                      - LPC_FLAG_LPCOMP: LPC counter comparator interrupt flag.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * if (LPC_GetFlagStatus(LPC0, LPC_FLAG_LPCOMP) == SET)
 * {
 *     LPC_ClearFlag(LPC0, LPC_FLAG_LPCOMP);
 * }
 * \endcode
 */
void LPC_ClearFlag(LPC_TypeDef *LPCx, uint32_t LPC_FLAG);

/**
 * \brief  Get the specified LPC interrupt status.
 *
 * \param[in] LPCx     Select the LPC peripheral. Refer to \ref LPC_Declaration.
 * \param[in] LPC_INT  Specify the LPC interrupts.
 *                     This parameter can be one of the following values:
 *                     \arg LPC_INT_LPCOMP: voltage detection interrupt.
 *
 * \return  The status of LPC interrupt.
 *          - SET: The LPC interrupt status has been set.
 *          - RESET: The LPC interrupt status has been reset.
 *
 */
ITStatus LPC_GetINTStatus(LPC_TypeDef *LPCx, uint32_t LPC_INT);

#if LPC_SUPPORT_RAP_MODE

void LPC_TaskTrigger(LPC_TypeDef *LPCx, LPCTask_TypeDef LPC_Task);
void LPC_RAPModeCmd(LPC_TypeDef *LPCx, FunctionalState NewState);
void LPC_RAPQactiveForceCmd(LPC_TypeDef *LPCx, FunctionalState NewState);
void LPC_ShortcutCmd(LPC_TypeDef *LPCx, uint32_t Task, uint32_t Event, FunctionalState NewState);

#endif
/** End of LPC_Exported_Functions
  * \}
  */

/** End of LPC
  * \}
  */

#ifdef __cplusplus
}
#endif

#endif /* _RTL_LPC_H_ */

