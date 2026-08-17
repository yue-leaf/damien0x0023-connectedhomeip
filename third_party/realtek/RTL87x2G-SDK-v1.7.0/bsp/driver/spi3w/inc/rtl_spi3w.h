/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef RTL_SPI3W_H
#define RTL_SPI3W_H

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "utils/rtl_utils.h"
#if defined (CONFIG_SOC_SERIES_RTL87X2G)
#include "SPI3W/src/rtl87x2g/rtl_spi3w_def.h"
#elif defined (CONFIG_SOC_SERIES_RTL87X3E)
#include "SPI3W/src/rtl87x3e/rtl_spi3w_def.h"
#elif defined (CONFIG_SOC_SERIES_RTL87X3D)
#include "SPI3W/src/rtl87x3d/rtl_spi3w_def.h"
#elif defined (CONFIG_SOC_SERIES_RTL8762J)
#include "SPI3W/src/rtl87x2j/rtl_spi3w_def.h"
#endif

/** \defgroup SPI3W    SPI3W
  * \brief
  * \{
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/
/** \defgroup SPI3W_Exported_Constants SPI3W Exported Constants
  * \brief
  * \{
  */

/**
 * \defgroup    SPI3W_Mode SPI3W Mode
 * \{
 * \ingroup     SPI3W_Exported_Constants
 */
typedef enum
{
    SPI3W_2WIRE_MODE,   //!< 2-wire mode, disable SPI3W.
    SPI3W_3WIRE_MODE,   //!< 3-wire mode, enable SPI3W.
} SPI3WMode_TypeDef;

#define IS_SPI3W_MODE(MODE) ((MODE) == SPI3W_2WIRE_MODE || (MODE) == SPI3W_3WIRE_MODE) //!< Check if the input parameter is valid.

/** End of SPI3W_Mode
  * \}
  */

/**
 * \defgroup    SPI3W_OE_Delay_Config SPI3W OE Delay Config
 * \{
 * \ingroup     SPI3W_Exported_Constants
 */
typedef enum
{
    SPI3W_OE_DELAY_NONE,   //!< Turn off the extension of the delay.
    SPI3W_OE_DELAY_1T,     //!< Extend the SPI3W_OE by spi3w_dly_cycle + 1 clock cycles.
} SPI3WDelay_TypeDef;

#define IS_SPI3W_OE_DELAY_CFG(CFG) ((CFG) == SPI3W_OE_DELAY_1T || (CFG) == SPI3W_OE_DELAY_NONE) //!< Check if the input parameter is valid.

/** End of SPI3W_OE_Delay_Config
  * \}
  */

/**
 * \defgroup    SPI3W_End_Extend_Mode SPI3W End Extend Mode
 * \{
 * \ingroup     SPI3W_Exported_Constants
 */
typedef enum
{
    SPI3W_NORMAL_MODE,    //!< SPI3W is in normal mode.
    SPI3W_EXTEND_MODE,    //!< SPI3W is in extend mode.
} SPI3WExtMode_TypeDef;

#define IS_SPI3W_END_EXTEND_MODE(MODE) ((MODE) == SPI3W_EXTEND_MODE || (MODE) == SPI3W_NORMAL_MODE) //!< Check if the input parameter is valid.

/** End of SPI3W_End_Extend_Mode
  * \}
  */

/**
 * \defgroup    SPI3W_Read_Cycle_Delay SPI3W Read Cycle Delay
 * \{
 * \ingroup     SPI3W_Exported_Constants
 */
#define IS_SPI3W_READ_CYCLE_DELAY(DELAY) (DELAY <= 0x1f)     //!< SPI3W Read Cycle Delay.

/** End of SPI3W_Read_Cycle_Delay
  * \}
  */

/**
 * \defgroup    SPI3W_Flag SPI3W Flag
 * \{
 * \ingroup     SPI3W_Exported_Constants
 */
#define SPI3W_FLAG_BUSY                       BIT4     //!< SPI3W is busy.
#define SPI3W_FLAG_INT_IND                    BIT5     //!< There is SPI3W interrupt.
#define SPI3W_FLAG_RESYNC_BUSY                BIT6     //!< SPI3W resync busy indicator.

#define IS_SPI3W_FLAG(FLAG) (((FLAG) == SPI3W_FLAG_BUSY) || \
                             ((FLAG) == SPI3W_FLAG_INT_IND) || \
                             ((FLAG) == SPI3W_FLAG_RESYNC_BUSY)) //!< Check if the input parameter is valid.

/** End of SPI3W_Flag
  * \}
  */

/**
 * \defgroup    SPI3W_Interrupt_Definition SPI3W Interrupt Definition
 * \{
 * \ingroup     SPI3W_Exported_Constants
 */
#define SPI3W_INT_BIT                          BIT0     //!< SPI3W Interrupt Definition.

#define IS_SPI3W_INT(INT) ((INT) == SPI3W_INT_BIT) //!< Check if the input parameter is valid.

/** End of SPI3W_Interrupt_Definition
  * \}
  */

#if (SPI3W_SUPPORT_RAP_FUNCTION ==1)
/**
 * \defgroup    SPI3W_Qactive_Control SPI3W Qactive Control
 * \{
 * \ingroup     SPI3W_Exported_Constants
 */
typedef enum
{
    SPI3W_QACTIVE_HW_CONTROL_AUTO = 0x0,
    SPI3W_QACTIVE_FW_CONTROL_HIGH = 0x1,
} SPI3WQactiveControl_TypeDef;

/** End of SPI3W_Qactive_Control
  * \}
  */

/**
 * \defgroup    SPI3W_Task SPI3W Task
 * \{
 * \ingroup     SPI3W_Exported_Constants
 */
typedef enum
{
    SPI3W_TASK_START = 0,
} SPI3WTask_TypeDef;

/** End of SPI3W_Task
  * \}
  */

#endif

/** End of SPI3W_Exported_Constants
  * \}
  */

/*============================================================================*
 *                         Types
 *============================================================================*/
/** \defgroup SPI3W_Exported_Types SPI3W Exported Types
  * \brief
  * \{
  */

/**
 * \brief   SPI3W initialize parameters.
 *
 * \ingroup SPI3W_Exported_Types
 */
typedef struct
{
    uint32_t SPI3W_SysClock;              /**< Specify system clock.
                                               This parameter must be 20MHz. */

    uint32_t SPI3W_Speed;                 /**< Specify SPI3W clock.
                                               This parameter can be a value of 20/256MHz ~ 2MHz. */

    SPI3WMode_TypeDef SPI3W_Mode;         /**< Specify SPI3W operation mode.
                                               This parameter can be a value of \ref SPI3W_Mode. */

    uint32_t SPI3W_ReadDelay;             /**< Specify the delay time from the end of address phase to the start of read data phase.
                                               This parameter can be a value of 0x0 to 0x1f.
                                               delay time = (SPI3W_ReadDelay+1)/(2*SPI3W_Speed). */

    SPI3WDelay_TypeDef SPI3W_OutputDelay; /**< Specify SPI output delay 1T or not.
                                               This parameter can be a value of \ref SPI3W_OE_Delay_Config. */

    SPI3WExtMode_TypeDef SPI3W_ExtMode;   /**< Specify extended timing window for SPI output enable = 0.
                                               This parameter can be a value of \ref SPI3W_End_Extend_Mode. */
} SPI3W_InitTypeDef;

/** End of SPI3W_Exported_Types
  * \}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** \defgroup SPI3W_Exported_Functions SPI3W Exported Functions
  * \brief
  * \{
  */

/**
 * \brief  Deinitialize the SPI3W peripheral registers to their default reset values.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_spi3wire_init(void)
 * {
 *     SPI3W_DeInit();
 * }
 * \endcode
 */
void SPI3W_DeInit(void);

/**
 * \brief   Initialize the SPI3W peripheral according to the specified parameters in SPI3W_InitStruct.
 *
 * \param[in] SPI3W_InitStruct  Pointer to a SPI3W_InitTypeDef structure which will be initialized.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_spi3wire_init(void)
 * {
 *     RCC_PeriphClockCmd(APBPeriph_SPI3W, APBPeriph_SPI3W_CLOCK, ENABLE);
 *     SPI3W_InitTypeDef SPI3W_InitStruct;
 *     SPI3W_StructInit(&SPI3W_InitStruct);
 *     SPI3W_InitStruct.SPI3W_Speed = 2000000;
 *     SPI3W_InitStruct.SPI3W_Mode  = SPI3W_3WIRE_MODE;
 *     SPI3W_InitStruct.SPI3W_ReadDelay = 0x1f;
 *     SPI3W_InitStruct.SPI3W_OutputDelay = SPI3W_OE_DELAY_1T;
 *     SPI3W_InitStruct.SPI3W_ExtMode = SPI3W_EXTEND_MODE;
 *     SPI3W_Init(&SPI3W_InitStruct);
 * }
 * \endcode
 */
void SPI3W_Init(SPI3W_InitTypeDef *SPI3W_InitStruct);

/**
 * \brief   Fill each SPI3W_InitStruct member with its default value.
 *
 * \note The default settings for the SPI3W_InitStruct member are shown in the following table:
 *       | SPI3W_InitStruct member      | Default value                        |
 *       |:----------------------------:|:------------------------------------:|
 *       | SPI3W_SysClock               | 20000000                             |
 *       | SPI3W_Speed                  | 1000000                              |
 *       | SPI3W_Mode                   | \ref SPI3W_2WIRE_MODE                |
 *       | SPI3W_ReadDelay              | 0                                    |
 *       | SPI3W_OutputDelay            | \ref SPI3W_OE_DELAY_1T               |
 *       | SPI3W_ExtMode                | \ref SPI3W_NORMAL_MODE               |
 *
 * \param[in] SPI3W_InitStruct  Pointer to a SPI3W_InitTypeDef structure which will be initialized.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_spi3wire_init(void)
 * {
 *     RCC_PeriphClockCmd(APBPeriph_SPI3W, APBPeriph_SPI3W_CLOCK, ENABLE);
 *     SPI3W_InitTypeDef SPI3W_InitStruct;
 *     SPI3W_StructInit(&SPI3W_InitStruct);
 *     SPI3W_InitStruct.SPI3W_SysClock    = 20000000;
 *     SPI3W_InitStruct.SPI3W_Speed       = 800000;
 *     SPI3W_InitStruct.SPI3W_Mode        = SPI3W_2WIRE_MODE;
 *     SPI3W_Init(SPI3W);
 * }
 * \endcode
 */
void SPI3W_StructInit(SPI3W_InitTypeDef *SPI3W_InitStruct);

/**
 * \brief   Enable or disable the specified SPI3W peripheral.
 *
 * \param[in] NewState  New state of the SPI3W peripheral.
 *                      This parameter can be one of the following values:
 *                      - ENABLE: Enable the SPI3W.
 *                      - DISABLE: Disable the SPI3W.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_spi3wire_init(void)
 * {
 *     RCC_PeriphClockCmd(APBPeriph_SPI3W, APBPeriph_SPI3W_CLOCK, ENABLE);
 *     SPI3W_InitTypeDef SPI3W_InitStruct;
 *     SPI3W_StructInit(&SPI3W_InitStruct);
 *     SPI3W_InitStruct.SPI3W_SysClock    = 20000000;
 *     SPI3W_InitStruct.SPI3W_Speed       = 800000;
 *     SPI3W_InitStruct.SPI3W_Mode        = SPI3W_2WIRE_MODE;
 *     SPI3W_Init(SPI3W);
 *     SPI3W_Cmd(ENABLE);
 * }
 * \endcode
 */
void SPI3W_Cmd(FunctionalState NewState);

/**
 * \brief   Enable or disable the specified SPI3W interrupts.
 *
 * \param[in] SPI3W_INT  Specify the SPI3W interrupts sources to be enabled or disabled.
 *                       This parameter can be only be the following value:
 *                       \arg SPI3W_INT_BIT: Enable SPI3W interrupt.
 * \param[in] newState   New state of the specified SPI3W interrupts.
 *                       This parameter can be one of the following values:
 *                       - ENABLE: Enable the interrupt of SPI3W.
 *                       - DISABLE: Disable the interrupt of SPI3W.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_spi3wire_init(void)
 * {
 *     RCC_PeriphClockCmd(APBPeriph_SPI3W, APBPeriph_SPI3W_CLOCK, ENABLE);
 *     SPI3W_InitTypeDef SPI3W_InitStruct;
 *     SPI3W_StructInit(&SPI3W_InitStruct);
 *     SPI3W_InitStruct.SPI3W_SysClock    = 20000000;
 *     SPI3W_InitStruct.SPI3W_Speed       = 800000;
 *     SPI3W_InitStruct.SPI3W_Mode        = SPI3W_2WIRE_MODE;
 *     SPI3W_Init(SPI3W);
 *     SPI3W_INTConfig(SPI3W_INT_BIT, ENABLE);
 * }
 * \endcode
 */
void SPI3W_INTConfig(uint32_t SPI3W_INT, FunctionalState newState);

/**
 * \brief   Configure resync signal time value.
 *
 * \param[in] value  Resync signal time value whose uint is 1/(2*SPI3W_Speed).
 *                   This parameter can be only be a value of 0x0 ~ 0xf.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void spi3wire_demo(void)
 * {
 *     SPI3W_SetResyncTime(10);
 * }
 * \endcode
 */
void SPI3W_SetResyncTime(uint32_t value);

/**
 * \brief   Enable or disable sending the resync signal. Must send when SPI3W is disable.
 *
 * \param[in] NewState  New state of the SPI3W peripheral.
 *                      This parameter can be only be the following value:
 *                      - ENABLE: Trigger resync signal.
 *                      - DISABLE: Disable resync signal.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void spi3wire_demo(void)
 * {
 *     SPI3W_ResyncSignalCmd(ENABLE);
 * }
 * \endcode
 */
void SPI3W_ResyncSignalCmd(FunctionalState NewState);

/**
 * \brief   Get total number of data byte in each SPI reading.
 *
 * \return  The total number of data byte in each SPI reading.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void spi3wire_demo(void)
 * {
 *    uint8_t data_len = 0;
 *    data_len = SPI3W_GetRxDataLen();
 * }
 * \endcode
 */
uint8_t SPI3W_GetRxDataLen(void);

/**
 * \brief   Clear read data number status.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void spi3wire_demo(void)
 * {
 *    SPI3W_ClearRxDataLen();
 * }
 * \endcode
 */
void SPI3W_ClearRxDataLen(void);

/**
 * \brief   Clear all read data registers.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void spi3wire_demo(void)
 * {
 *    SPI3W_ClearRxFIFO();
 * }
 * \endcode
 */
void SPI3W_ClearRxFIFO(void);

/**
 * \brief   Start to write data.
 *
 * \param[in] address  Write address.
 * \param[in] data     Write data.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void spi3wire_demo(void)
 * {
 *     uint8_t address = 0x10;
 *     uint8_t data = 0x10;
 *     SPI3W_StartWrite(address, data);
 * }
 * \endcode
 */
void SPI3W_StartWrite(uint8_t address, uint8_t data);

/**
 * \brief   Start reading data.
 *
 * \param[in] address  Read address.
 * \param[in] len      Number of data to read. This value can be 0x1 to 0xf.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void spi3wire_demo(void)
 * {
 *     uint8_t address = 0x10;
 *     uint8_t len = 0x1;
 *     SPI3W_StartRead(address, len);
 * }
 * \endcode
 */
void SPI3W_StartRead(uint8_t address, uint32_t len);

/**
 * \brief  Read data.
 *
 * \param[in] pBuf     Buffer to store read data.
 * \param[in] readNum  Read number.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void spi3wire_demo(void)
 * {
 *     uint8_t data_buf[10] = {0};
 *     uint8_t data_len = 10;
 *     SPI3W_ReadBuf(data_buf, data_len);
 * }
 * \endcode
 */
void SPI3W_ReadBuf(uint8_t *pBuf, uint8_t readNum);

/**
 * \brief   Get the specified SPI3W flag status.
 *
 * \param[in] SPI3W_FLAG  Specify flags to check.
 *                        This parameter can be one of the following values, Refer to \ref SPI3W_Flag.
 *                        \arg SPI3W_FLAG_BUSY: SPI3W is busy.
 *                        \arg SPI3W_FLAG_INT_IND: There is SPI3W interrupt.
 *                        \arg SPI3W_FLAG_RESYNC_BUSY: Resync busy or not.
 *
 * \return  The new state of the specified SPI3W flag.
 *          - SET: The specified SPI3W flag state is pending.
 *          - RESET: The specified SPI3W flag state is not pending.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void spi3wire_demo(void)
 * {
 *    FlagStatus flag_status = RESET;
 *    flag_status = SPI3W_GetFlagStatus(SPI3W_FLAG_BUSY);
 * }
 * \endcode
 */
FlagStatus SPI3W_GetFlagStatus(uint32_t SPI3W_FLAG);

/**
 * \brief   Clear the SPI3W interrupt pending bit.
 *
 * \param[in] SPI3W_INT  Specify the SPI3W interrupt sources.
 *                       This parameter can only be the following values:
 *                       - SPI3W_INT_BIT: Enable SPI3W interrupt source.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void spi3wire_demo(void)
 * {
 *    SPI3W_ClearINTPendingBit(SPI3W_INT_BIT);
 * }
 * \endcode
 */

void SPI3W_ClearINTPendingBit(uint32_t SPI3W_INT);


#if (SPI3W_SUPPORT_RAP_FUNCTION ==1)

void SPI3W_RAPQactiveCtrl(uint32_t Qactive, FunctionalState NewState);

void SPI3W_QuickBurstStart(void);

void SPI3W_QuickBurstPulseWidthSet(uint8_t pulse_width);

void SPI3W_RAPModeCmd(FunctionalState NewState);

void SPI3W_TaskTrigger(uint32_t Task);

#endif


/** End of SPI3W_Exported_Functions
  * \}
  */

/** End of SPI3W
  * \}
  */

#ifdef __cplusplus
}
#endif

#endif /* RTL_SPI3W_H */

