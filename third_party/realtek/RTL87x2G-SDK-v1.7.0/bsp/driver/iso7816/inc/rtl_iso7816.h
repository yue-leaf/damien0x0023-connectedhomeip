/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef RTL_ISO7816_H
#define RTL_ISO7816_H

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "utils/rtl_utils.h"
#if defined (CONFIG_SOC_SERIES_RTL87X2G)
#include "iso7816/src/rtl87x2g/rtl_iso7816_def.h"
#endif

/** \defgroup ISO7816     ISO7816
  * \brief
  * \{
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/
/** \defgroup ISO7816_Exported_Constants ISO7816 Exported Constants
  * \brief
  * \{
  */

/**
 * \defgroup    ISO7816_FIFO_SIZE ISO7816 FIFO SIZE
 * \{
 * \ingroup     ISO7816_Exported_Constants
 */
#define ISO7816_TX_FIFO_SIZE             16    //!< ISO7816 TX FIFO SIZE is 16.
#define ISO7816_RX_FIFO_SIZE             32    //!< ISO7816 RX FIFO SIZE is 32.

/** End of ISO7816_FIFO_SIZE
  * \}
  */

/**
 * \defgroup    ISO7816_Clock_Cmd ISO7816 Clock Cmd
 * \{
 * \ingroup     ISO7816_Exported_Constants
 */
#define ISO7816_CLK_CONTACT_Reset        ISO7816_CLOCK_ENABLE_CLR    //!< ISO7816 clock disable.
#define ISO7816_CLK_CONTACT_Set          ISO7816_CLOCK_ENABLE_MSK    //!< ISO7816 clock enable.

/** End of ISO7816_Clock_Cmd
  * \}
  */

/**
 * \defgroup    ISO7816_Clock_Stop_Polarity ISO7816 Clock Stop Polarity
 * \{
 * \ingroup     ISO7816_Exported_Constants
 */
#define ISO7816_CLOCK_STOP_POLAR_LOW     ISO7816_CLOCK_STOP_POLAR_CLR    //!< The ISO7816 Clock Stop Polarity is set to low.
#define ISO7816_CLOCK_STOP_POLAR_HIGH    ISO7816_CLOCK_STOP_POLAR_MSK    //!< The ISO7816 Clock Stop Polarity is set to high.

/** End of ISO7816_Clock_Stop_Polarity
  * \}
  */

/**
 * \defgroup    ISO7816_Power_Cmd ISO7816 Power Cmd
 * \{
 * \ingroup     ISO7816_Exported_Constants
 */
#define ISO7816_VCC_CONTACT_Reset        ISO7816_VCC_ENABLE_CLR   //!< ISO7816 VCC disable.
#define ISO7816_VCC_CONTACT_Set          ISO7816_VCC_ENABLE_MSK   //!< ISO7816 VCC enable.

/** End of ISO7816_Power_Cmd
  * \}
  */

/**
 * \defgroup    ISO7816_Reset_Cmd ISO7816 Reset Cmd
 * \{
 * \ingroup     ISO7816_Exported_Constants
 */
#define ISO7816_RST_CONTACT_Reset        ISO7816_RESET_ENABLE_CLR   //!< ISO7816 interface unreset output
#define ISO7816_RST_CONTACT_Set          ISO7816_RESET_ENABLE_MSK   //!< ISO7816 interface reset output

/** End of ISO7816_Reset_Cmd
  * \}
  */

/**
 * \defgroup    ISO7816_General_Purpose_Counter_Cmd ISO7816 GP Counter Cmd
 * \{
 * \ingroup     ISO7816_Exported_Constants
 */
#define ISO7816_GP_CNT_DISABLE           ISO7816_GP_COUNTER_ENABLE_CLR   //!< Disable ISO7816 general purpose counter.
#define ISO7816_GP_CNT_ENABLE            ISO7816_GP_COUNTER_ENABLE_MSK   //!< Enable ISO7816 general purpose counter.

/** End of ISO7816_General_Purpose_Counter_Cmd
  * \}
  */

/**
 * \defgroup    ISO7816_Controller_Cmd ISO7816 Controller Cmd
 * \{
 * \ingroup     ISO7816_Exported_Constants
 */
#define ISO7816_CR_DISABLE               ISO7816_CONTROLLER_ENBALE_CLR   //!< Disable ISO7816 controller.
#define ISO7816_CR_ENABLE                ISO7816_CONTROLLER_ENBALE_MSK   //!< Enable ISO7816 controller.

/** End of ISO7816_Controller_Cmd
  * \}
  */

/**
 * \defgroup    ISO7816_IO_Mode ISO7816 IO Mode
 * \{
 * \ingroup     ISO7816_Exported_Constants
 */
#define ISO7816_IO_MODE_RX                ISO7816_IO_MODE_CLR   //!< Enable reception mode
#define ISO7816_IO_MODE_TX                ISO7816_IO_MODE_MSK   //!< Enable transmission mode 

/** End of ISO7816_IO_Mode
  * \}
  */

/**
 * \defgroup    ISO7816_Coding_Convention_Cmd ISO7816 Coding Convention Cmd
 * \{
 * \ingroup     ISO7816_Exported_Constants
 */
#define ISO7816_CODING_CONVENTION_DIRECT  ISO7816_CODING_CONVENTION_CLR   //!< ISO7816 direct coding convention.
#define ISO7816_CODING_CONVENTION_INVERSE ISO7816_CODING_CONVENTION_MSK   //!< ISO7816 inverse coding convention.

/** End of ISO7816_Coding_Convention_Cmd
  * \}
  */

/**
 * \defgroup    ISO7816_IO_STATE_SAMPLE ISO7816 IO State Sample
 * \{
 * \ingroup     ISO7816_Exported_Constants
 */
#define ISO7816_IO_STATE_SAMPLE_DISABLE   ISO7816_IO_STATE_SAMPLE_CLR    //!< Disable ISO7816 IO state sample.
#define ISO7816_IO_STATE_SAMPLE_ENABLE    ISO7816_IO_STATE_SAMPLE_MSK    //!< Enable ISO7816 IO state sample.

/** End of ISO7816_IO_STATE_SAMPLE
  * \}
  */

/**
 * \defgroup    ISO7816_Interrupts_Definition  ISO7816 Interrupts Definition
 * \{
 * \ingroup     ISO7816_Exported_Constants
 */
#define ISO7816_INT_RESET_TIMING_VIOLATION           ((uint16_t)(1 << 0))     //!< ISO7816 reset timing violation interrupt
#define ISO7816_INT_TX_NAK_THD                       ((uint16_t)(1 << 1))     //!< ISO7816 TNAKTH exceeded interrupt
#define ISO7816_INT_CHAR_WAIT_TIMEOUT                ((uint16_t)(1 << 2))     //!< ISO7816 character reception timeout interrupt
#define ISO7816_INT_GP_COUNTER                       ((uint16_t)(1 << 3))     //!< ISO7816 general purpose counter hit interrupt
#define ISO7816_INT_TX_FIFO_EMPTY                    ((uint16_t)(1 << 4))     //!< ISO7816 TX FIFO empty interrupt
#define ISO7816_INT_TX_FIFO_NOT_FULL                 ((uint16_t)(1 << 5))     //!< ISO7816 TX FIFO not full interrupt
#define ISO7816_INT_TX_FIFO_OVERFLOW                 ((uint16_t)(1 << 6))     //!< ISO7816 TX FIFO overflow interrupt
#define ISO7816_INT_RX_FIFO_NOT_EMPTY                ((uint16_t)(1 << 7))     //!< ISO7816 RX FIFO not empty interrupt
#define ISO7816_INT_RX_FIFO_ALMOST_FULL              ((uint16_t)(1 << 8))     //!< ISO7816 RX FIFO almost full interrupt
#define ISO7816_INT_RX_FIFO_FULL                     ((uint16_t)(1 << 9))     //!< ISO7816 RX FIFO full interrupt
#define ISO7816_INT_RX_FIFO_OVERFLOW                 ((uint16_t)(1 << 10))    //!< ISO7816 RX FIFO overflow interrupt
#define ISO7816_INT_RX_FIFO_UNDERFLOW                ((uint16_t)(1 << 11))    //!< ISO7816 RX FIFO underflow interrupt
#define ISO7816_INT_TX_DONE                          ((uint16_t)(1 << 12))    //!< ISO7816 TX completed interrupt

#define IS_ISO7816_INT(INT) (((INT) == ISO7816_INT_RESET_TIMING_VIOLATION) || ((INT) == ISO7816_INT_TX_NAK_THD)\
                             || ((INT) == ISO7816_INT_CHAR_WAIT_TIMEOUT) || ((INT) == ISO7816_INT_GP_COUNTER)\
                             || ((INT) == ISO7816_INT_TX_FIFO_EMPTY) || ((INT) == ISO7816_INT_TX_FIFO_NOT_FULL)\
                             || ((INT) == ISO7816_INT_TX_FIFO_OVERFLOW) || ((INT) == ISO7816_INT_RX_FIFO_NOT_EMPTY)\
                             || ((INT) == ISO7816_INT_RX_FIFO_ALMOST_FULL) || ((INT) == ISO7816_INT_RX_FIFO_FULL)\
                             || ((INT) == ISO7816_INT_RX_FIFO_OVERFLOW) || ((INT) == ISO7816_INT_RX_FIFO_UNDERFLOW)\
                             || ((INT) == ISO7816_INT_TX_DONE)) //!< Check if the input parameter is valid.

/** End of ISO7816_Interrupts_Definition
  * \}
  */

/**
 * \defgroup    ISO7816_Flags_Definition ISO7816 Flags Definition
 * \{
 * \ingroup     ISO7816_Exported_Constants
 */
#define ISO7816_FLAG_TX_FIFO_EMPTY                   ((uint16_t)(1 << 0))    //!< ISO7816 TX FIFO empty flag
#define ISO7816_FLAG_TX_FIFO_NOT_FULL                ((uint16_t)(1 << 1))    //!< ISO7816 TX FIFO not full flag
#define ISO7816_FLAG_TX_IDLE                         ((uint16_t)(1 << 2))    //!< ISO7816 TX idle state
#define ISO7816_FLAG_RX_FIFO_NOT_EMPTY               ((uint16_t)(1 << 4))    //!< ISO7816 RX FIFO not empty flag
#define ISO7816_FLAG_RX_FIFO_ALMOST_FULL             ((uint16_t)(1 << 5))    //!< ISO7816 RX FIFO almost full flag
#define ISO7816_FLAG_RX_FIFO_FULL                    ((uint16_t)(1 << 6))    //!< ISO7816 RX FIFO full flag
#define ISO7816_FLAG_RX_IO_HIGH_TIMING_VIOLATION     ((uint16_t)(1 << 8))    //!< ISO7816 Ta or Td timing violation flag
#define ISO7816_FLAG_RX_ATR_TIMING_VIOLATION         ((uint16_t)(1 << 9))    //!< ISO7816 Tc or Tf timing violation flag
#define ISO7816_FLAG_TX_NAK_CNT_THD                  ((uint16_t)(1 << 10)    //!< ISO7816 TX NAK count hits threshold flag
#define ISO7816_FLAG_RX_NAK_CNT_THD                  ((uint16_t)(1 << 11)    //!< ISO7816 RX NAK count hits threshold flag
#define ISO7816_FLAG_TX_FIFO_OVERFLOW                ((uint16_t)(1 << 12)    //!< ISO7816 TX FIFO overflow flag
#define ISO7816_FLAG_RX_FIFO_OVERFLOW                ((uint16_t)(1 << 13)    //!< ISO7816 RX FIFO overflow flag
#define ISO7816_FLAG_RX_FIFO_UNDERFLOW               ((uint16_t)(1 << 14)    //!< ISO7816 RX FIFO underflow flag
#define ISO7816_FLAG_RX_PARITY_ERR                   ((uint16_t)(1 << 15)    //!< ISO7816 RX parity error flag

#define IS_ISO7816_FLAG(FLAG) (((FLAG) == ISO7816_FLAG_TX_FIFO_EMPTY) || ((FLAG) == ISO7816_FLAG_TX_FIFO_NOT_FULL)\
                               || ((FLAG) == ISO7816_FLAG_TX_IDLE) || ((FLAG) == ISO7816_FLAG_RX_FIFO_NOT_EMPTY)\
                               || ((FLAG) == ISO7816_FLAG_RX_FIFO_ALMOST_FULL) || ((FLAG) == ISO7816_FLAG_RX_FIFO_FULL)\
                               || ((FLAG) == ISO7816_FLAG_RX_AD_TIMING_VIOLATION) || ((FLAG) == ISO7816_FLAG_RX_CF_TIMING_VIOLATION)\
                               || ((FLAG) == ISO7816_FLAG_TX_NAK_CNT_THD) || ((FLAG) == ISO7816_FLAG_RX_NAK_CNT_THD)\
                               || ((FLAG) == ISO7816_FLAG_TX_FIFO_OVERFLOW) || ((FLAG) == ISO7816_FLAG_RX_FIFO_OVERFLOW)\
                               || ((FLAG) == ISO7816_FLAG_RX_FIFO_UNDERFLOW) || ((FLAG) == ISO7816_FLAG_RX_PARITY_ERR)) //!< Check if the input parameter is valid.

/** End of ISO7816_Flags_Definition
  * \}
  */

/**
 * \defgroup    ISO7816_Error_Status ISO7816 Error Status
 * \{
 * \ingroup     ISO7816_Exported_Constants
 */
#define ISO7816_STATUS_RX_IO_HIGH_TIMING_VIOLATION     ((uint16_t)(1 << 0))   //!< ISO7816 Ta or Td timing violation status
#define ISO7816_STATUS_RX_ATR_TIMING_VIOLATION         ((uint16_t)(1 << 1))   //!< ISO7816 Tc or Tf timing violation status
#define ISO7816_STATUS_TX_NAK_CNT_THD                  ((uint16_t)(1 << 2)    //!< ISO7816 TX NAK count hits threshold status
#define ISO7816_STATUS_RX_NAK_CNT_THD                  ((uint16_t)(1 << 3)    //!< ISO7816 RX NAK count hits threshold status
#define ISO7816_STATUS_TX_FIFO_OVERFLOW                ((uint16_t)(1 << 4)    //!< ISO7816 TX FIFO overflow status
#define ISO7816_STATUS_RX_FIFO_OVERFLOW                ((uint16_t)(1 << 5)    //!< ISO7816 RX FIFO overflow status
#define ISO7816_STATUS_RX_FIFO_UNDERFLOW               ((uint16_t)(1 << 6)    //!< ISO7816 RX FIFO underflow status
#define ISO7816_STATUS_RX_PARITY_ERR                   ((uint16_t)(1 << 7)    //!< ISO7816 RX parity error status

#define IS_ISO7816_STATUS(STATUS) (((STATUS) == ISO7816_STATUS_RX_IO_HIGH_TIMING_VIOLATION) || ((STATUS) == ISO7816_STATUS_RX_ATR_TIMING_VIOLATION)\
                                   || ((STATUS) == ISO7816_STATUS_TX_NAK_CNT_THD) || ((STATUS) == ISO7816_STATUS_RX_NAK_CNT_THD)\
                                   || ((STATUS) == ISO7816_STATUS_TX_FIFO_OVERFLOW) || ((STATUS) == ISO7816_STATUS_RX_FIFO_OVERFLOW)\
                                   || ((STATUS) == ISO7816_STATUS_RX_FIFO_UNDERFLOW) || ((STATUS) == ISO7816_STATUS_RX_PARITY_ERR)) //!< Check if the input parameter is valid.

/** End of ISO7816_Error_Status
  * \}
  */

/**
 * \defgroup    ISO7816_CMD ISO7816 CMD
 * \{
 * \ingroup     ISO7816_Exported_Constants
 */
#define ISO7816_DISABLE                              ((uint8_t)(0 << 0))    //!< Disable the ISO7816.
#define ISO7816_ENABLE                               ((uint8_t)(1 << 0))    //!< Enable the ISO7816.

#define IS_ISO7816_CMD(CMD) (((CMD) == ISO7816_DISABLE) || ((CMD) == ISO7816_ENABLE)) //!< Check if the input parameter is valid.

/** End of ISO7816_CMD
  * \}
  */

/** End of ISO7816_Exported_Constants
  * \}
  */

/*============================================================================*
 *                         Types
 *============================================================================*/
/** \defgroup ISO7816_Exported_Types ISO7816 Exported Types
  * \brief
  * \{
  */

/**
 * \brief       ISO7816 initialize parameters
 *
 * \ingroup     ISO7816_Exported_Types
 */
typedef struct
{
    //baudrate calibration

    uint32_t ISO7816_IOMode;             /*!< Specify IO mode.
                                              This parameter can be a value of \ref ISO7816_IO_Mode. */

    uint32_t ISO7816_CodingConvention;   /*!< Specify the coding convention.
                                              This parameter can be a value of \ref ISO7816_Coding_Convention_Cmd. */

    uint16_t ISO7816_IntegerF;           /*!< Specify the clock rate conversion integer.*/

    uint16_t ISO7816_IntegerD;           /*!< Specify the baud rate adjustment integer.Number of ISO7816_ioclk periods to make up a etu time (equal to F/D -1 )*/

    uint32_t ISO7816_IOStateSample;      /*!< Sample I/O line in middle of start bit to confirm if a falling edge does lead to a character or not.
                                              This parameter can be a value of \ref ISO7816_IO_STATE_SAMPLE. */

} ISO7816_InitTypeDef;

/** End of ISO7816_Exported_Types
  * \}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** \defgroup ISO7816_Exported_Functions ISO7816 Exported Functions
  * \brief
  * \{
  */

/**
 * \brief   Deinitialize the ISO7816 peripheral registers to their default reset values(turn off ISO7816 clock).
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_smartcard_init(void)
 * {
 *    ISO7816_DeInit();
 * }
 * \endcode
 */
void ISO7816_DeInit(void);

/**
 * \brief       Initialize the ISO7816 peripheral according to the specified parameters in ISO7816_InitStruct.
 * \param[in]   ISO7816_InitStruct  Pointer to a ISO7816_InitTypeDef structure which will be initialized.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_smartcard_init(void)
 * {
 *     ISO7816_DeInit();
 *
 *     RCC_PeriphClockCmd(APBPeriph_ISO7816, APBPeriph_ISO7816_CLOCK, ENABLE);
 *
 *     ISO7816_InitTypeDef ISO7816_InitStruct;
 *     ISO7816_StructInit(&ISO7816_InitStruct);
 *
 *     ISO7816_InitStruct.ISO7816_IntegerF              = 372;
 *     ISO7816_InitStruct.ISO7816_IntegerD              = 1;
 *     ISO7816_InitStruct.ISO7816_IOMode                = ISO7816_IO_MODE_TX;
 *     ISO7816_InitStruct.ISO7816_CodingConvention      = ISO7816_CODING_CONVENTION_DIRECT;
 *     ISO7816_InitStruct.ISO7816_IOStateSample         = ISO7816_IO_STATE_SAMPLE_ENABLE;
 *     //Add other initialization parameters that need to be configured here.
 *     ISO7816_Init(ISO7816, &ISO7816_InitStruct);
 * }
 * \endcode
 */
void ISO7816_Init(ISO7816_InitTypeDef *ISO7816_InitStruct);

/**
 * \brief   Fill each ISO7816_InitStruct member with its default value.
 *
 * \note The default settings for the ISO7816_InitStruct member are shown in the following table:
 *       | ISO7816_InitStruct member    | Default value                        |
 *       |:----------------------------:|:------------------------------------:|
 *       | ISO7816_IntegerF             | 372                                  |
 *       | ISO7816_IntegerD             | 1                                    |
 *       | ISO7816_IOMode               | \ref ISO7816_IO_MODE_TX              |
 *       | ISO7816_CodingConvention     | \ref ISO7816_CODING_CONVENTION_DIRECT|
 *       | ISO7816_IOStateSample        | \ref ISO7816_IO_STATE_SAMPLE_ENABLE  |
 *
 * \param[in] ISO7816_InitStruct  Pointer to an ISO7816_InitTypeDef structure which will be initialized.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_smartcard_init(void)
 * {
 *     ISO7816_DeInit();
 *
 *     RCC_PeriphClockCmd(APBPeriph_ISO7816, APBPeriph_ISO7816_CLOCK, ENABLE);
 *
 *     ISO7816_InitStruct.ISO7816_IntegerF              = 372;
 *     ISO7816_InitStruct.ISO7816_IntegerD              = 1;
 *     ISO7816_InitStruct.ISO7816_IOMode                = ISO7816_IO_MODE_TX;
 *     ISO7816_InitStruct.ISO7816_CodingConvention      = ISO7816_CODING_CONVENTION_DIRECT;
 *     ISO7816_InitStruct.ISO7816_IOStateSample         = ISO7816_IO_STATE_SAMPLE_ENABLE;
 *     //Add other initialization parameters that need to be configured here.
 *     ISO7816_Init(ISO7816, &ISO7816_InitStruct);
 * }
 * \endcode
 */
void ISO7816_StructInit(ISO7816_InitTypeDef *ISO7816_InitStruct);

/**
 * \brief   Enable or disable the specified ISO7816 interrupts.
 *
 * \param[in] ISO7816_INT  Specify the ISO7816 interrupt sources.
 *                         This parameter can refer to \ref ISO7816_Interrupts_Definition.
 * \param[in] NewState     New state of the specified ISO7816 interrupt source.
 *                         This parameter can be one of the following values:
 *                         - ENABLE: Enable the ISO7816 interrupt.
 *                         - DISABLE: Disable the ISO7816 interrupt.
 *
 * <b>Example usage</b>
 * \code{.c}
 *

 * void smartcard_demo(void)
 * {
 *     ISO7816_INTConfig(ISO7816_INT_TX_DONE, ENABLE);
 * }
 * \endcode
 */
void ISO7816_INTConfig(uint32_t ISO7816_INT, FunctionalState NewState);

/**
 * \brief   Send data.
 *
 * \param[in] inBuf  Buffer of data to be sent.
 * \param[in] len    Length of data to be sent.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     uint8_t data[] = "ISO7816 demo";
 *     ISO7816_SendData(data, sizeof(data));
 * }
 * \endcode
 */
void ISO7816_SendData(const uint8_t *inBuf, uint16_t len);

/**
 * \brief   Receive data from RX FIFO.
 *
 * \param[out] outBuf  Buffer to store data which read from RX FIFO.
 * \param[in]  len     Length of data to read.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     uint8_t data[32] = {10};
 *     ISO7816_ReceiveData(data, 10);
 * }
 * \endcode
 */
void ISO7816_ReceiveData(uint8_t *outBuf, uint16_t len);

/**
 * \brief   Clod reset the smartcard.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_CardColdReset();
 * }
 * \endcode
 */
void ISO7816_CardColdReset(void);

/**
 * \brief   Warm reset the smartcard.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_CardWarmReset();
 * }
 * \endcode
 */
void ISO7816_CardWarmReset(void);

/**
 * \brief   Stop the clock output to the smartcard.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_CardClockStop();
 * }
 * \endcode
 */
void ISO7816_CardClockStop(void);

/**
 * \brief   Restart the clock output to the smartcard.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_CardClockRestart();
 * }
 * \endcode
 */
void ISO7816_CardClockRestart(void);

/**
 * \brief   Deactive a physically connected smartcard.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_CardDeactive();
 * }
 * \endcode
 */
void ISO7816_CardDeactive(void);

/**
 * \brief This function aims to provide delay based on cycle count Which should be count/Freq (s),
 *        given unit of Freq is Hz. Yet the actual delay function takes input parameters with unit us.
 *        We should have called simc_delay_us(count * 10^6 / Freq). However, there would be possible
 *        integer overflow of (count * 10^6). Thus we call iso7816_delay_us((count * 10^3) / (Freq / 10^3)) instead.
 *
 * \param[in] cycle  ISO7816 cycle count.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_DelayCycle(2000);
 * }
 * \endcode
 */
void ISO7816_DelayCycle(uint32_t cycle);

/**
 * \brief       Check whether the specified SmartCard flag is set.
 *
 * \param[in]   ISO7816_FLAG  Specify ISO7816 flag to check.
 *                            This parameter can be one of the following values:
 *                            \arg ISO7816_FLAG_RX_DATA_AVA: Rx data is avaliable.
 *                            \arg ISO7816_FLAG_RX_OVERRUN: Rx FIFO overrun.
 *                            \arg ISO7816_FLAG_RX_PARITY_ERR: Rx parity error.
 *                            \arg ISO7816_FLAG_RX_FRAME_ERR: Rx frame error.
 *                            \arg ISO7816_FLAG_RX_BREAK_ERR: Rx break error.
 *                            \arg ISO7816_FLAG_TX_FIFO_EMPTY: Tx Holding Register or Tx FIFO empty
 *                            \arg ISO7816_FLAG_TX_EMPTY: Tx FIFO and Tx shift register are both empty.
 *                            \arg ISO7816_FLAG_RX_FIFO_ERR: Rx FIFO error.
 *                            \arg ISO7816_FLAG_RX_BREAK: Rx break.
 *                            \arg ISO7816_FLAG_RX_IDLE: Rx idle.
 *                            \arg ISO7816_FLAG_TX_DONE: Tx waveform done & TX_FIFO_EMPTY = 1.
 *                            \arg ISO7816_FLAG_TX_THD: TX_FIFO_LEVEL<=txfifo_trigger_level.
 *
 * \return      New status of ISO7816 flag.
 *              - SET: The specified ISO7816 flag bit is set.
 *              - RESET: The specified flag is not set.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 *

 * void smartcard_demo(void)
 * {
 *     while (ISO7816_GetFlagStatus(ISO7816x, ISO7816_FLAG_TX_FIFO_EMPTY) == 0);
 *     ISO7816_SendData(data, sizeof(data));
 * }
 * \endcode
 */
FlagStatus ISO7816_GetFlagStatus(uint32_t ISO7816_FLAG);

/**
 * \brief   Active and cold reset a physically connected card.
 *
 * <b>Example usage</b>
 * \code{.c}
 * void smartcard_demo(void)
 * {
 *     ISO7816_CardActivtion();
 * }
 * \endcode
 */
void ISO7816_CardActivtion(void);

/**
 * \brief     Get the specified ISO7816 interrupt status.
 *
 * \param[in] ISO7816_INT  Specify the interrupt source to check.
 *                         This parameter can be one of the following values.
 *                         Refer to \ref ISO7816_Interrupts_Definition.
 *                         \arg ISO7816_INT_RESET_TIMING_VIOLATION: Reset timing violation.
 *                         \arg ISO7816_INT_TX_NAK_THD: TNAKTH exceeded.
 *                         \arg ISO7816_INT_RX_WAIT_TIMEOUT: Character wait timeout.
 *                         \arg ISO7816_INT_GP_COUNTER: General purpose counter hit.
 *                         \arg ISO7816_INT_TX_FIFO_EMPTY: TX FIFO empty.
 *                         \arg ISO7816_INT_TX_FIFO_NOT_FULL: TX FIFO not full, triggered when TX FIFO changes from full to non-full.
 *                         \arg ISO7816_INT_TX_FIFO_OVERFLOW: TX FIFO overflow.
 *                         \arg ISO7816_INT_RX_FIFO_NOT_EMPTY: RX FIFO not empty, triggered when RX FIFO changes from empty to non-empty.
 *                         \arg ISO7816_INT_RX_FIFO_ALMOST_FULL: RX FIFO almost full
 *                         \arg ISO7816_INT_RX_FIFO_FULL: RX FIFO full.
 *                         \arg ISO7816_INT_RX_FIFO_OVERFLOW: RX FIFO overflow.
 *                         \arg ISO7816_INT_RX_FIFO_UNDERFLOW: RX FIFO underflow.
 *                         \arg ISO7816_INT_TX_DONE: Transmission completed, triggered when TX FIFO is empty and the last bit has been sent out.
 *
 * \return    The new state of ISO7816 interrupts.
 *            - SET: The specified interrupt status is set.
 *            - RESET: The specified interrupt status is not set.
 */
uint16_t ISO7816_GetINTStatus(void);

/**
 * \brief    Get the error status of ISO7816.
 *
 * \return   The error status of ISO7816, refer to \ref ISO7816_Error_Status.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * uint8_t errorStatus = ISO7816_GetErrStatus();
 * if (errorStatus != 0) {
 * {
 *      //add code here
 * }
 * \endcode
 */
uint8_t ISO7816_GetErrStatus(void);

/**
 * \brief       Clear the error status of ISO7816.
 *
 * \param[in]   status  The error status of ISO7816, refer to \ref ISO7816_Error_Status.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_ClearErrStatus(ISO7816_STATUS_RX_PARITY_ERR);
 * }
 * \endcode
  */
void ISO7816_ClearErrStatus(uint8_t status);

/**
 * \brief       Send one byte of data.
 *
 * \param[in]   data  Byte data to send.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     uint8_t data = 0x55;
 *     ISO7816_SendByte(data);
 * }
 * \endcode
 */
void ISO7816_SendByte(uint8_t data);

/**
 * \brief   Read a byte of data from SmartCard RX FIFO.
 *
 * \return  The byte data has been read.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     uint8_t data = ISO7816_ReceiveByte();
 *
 * }
 * \endcode
 */
uint8_t ISO7816_ReceiveByte(void);

/**
 * \brief   Clear Tx FIFO of SmartCard peripheral.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_ClearTxFIFO();
 * }
 * \endcode
 */
void ISO7816_ClearTxFIFO(void);

/**
 * \brief   Clear Rx FIFO of SmartCard peripheral.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_ClearRxFIFO();
 * }
 * \endcode
 */
void ISO7816_ClearRxFIFO(void);

/**
 * \brief Restart the general purpose counter.
 *
 * \param[in] value  0~0xFFFF.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_GPCounterRestart(400);
 * }
 * \endcode
 */
void ISO7816_GPCounterRestart(uint16_t value);

/**
 * \brief Set the general purpose counter.
 *
 * \param[in] value  0~0xFFFF.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_GPCounterSet(400);
 * }
 * \endcode
 */
void ISO7816_GPCounterSet(uint16_t value);

/**
 * \brief Enable the general purpose counter.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_GPCounterEnable();
 * }
 * \endcode
 */
void ISO7816_GPCounterEnable(void);

/**
 * \brief Disable the general purpose counter.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_GPCounterDisable();
 * }
 * \endcode
 */
void ISO7816_GPCounterDisable(void);

/**
 * \brief  Reset ISO7816 interface output.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_SetRST();
 * }
 * \endcode
  */
void ISO7816_SetRST(void);

/**
 * \brief  Enable or disable error signaling and character repetition features.
 *
 * \param[in] protocol  0 or 1.
 *                      - 0: Enable error signal and character repetition.
 *                      - 1: Disable error signal and character repetition.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_ProtocolTypeConfig(0);
 * }
 * \endcode
  */
void ISO7816_ProtocolTypeConfig(uint8_t protocol);

/**
 * \brief     Enable or disable ISO7816 clock stop polar.
 *
 * \param[in] polar  0 or 1.
 *                   - 0: Enable ISO7816 clock stop polar.
 *                   - 1: Disable ISO7816 clock stop polar.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_ClockPolarConfig(0);
 * }
 * \endcode
  */
void ISO7816_ClockPolarConfig(uint8_t polar);

/**
 * \brief  Specify the character encoding.
 *
 * \param[in] convention  0 or 1.
 *                        - 0: direct specification
 *                        - 1: inverse convention
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_ConventionConfig(0);
 * }
 * \endcode
  */
void ISO7816_ConventionConfig(uint8_t convention);

/**
 * \brief  Set the number of SIMC_IOCLK periods,(equal to F/D -1 ).
 *
 * \param[in] speed  0~0xFFF.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_SetSpeed(100);
 * }
 * \endcode
  */
void ISO7816_SetSpeed(uint16_t speed);

/**
 * \brief  Set extra guard time.
 *
 * \param[in] time  0~0xFF.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_SetGT(20);
 * }
 * \endcode
  */
void ISO7816_SetGT(uint8_t time);

/**
 * \brief  Set the character-to-character timeout.
 *         If the next character is not received within this time period,
 *         an error interrupt will be triggered.
 *
 * \param[in] time  0~0xFFFE.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void smartcard_demo(void)
 * {
 *     ISO7816_SetWT(0xF);
 * }
 * \endcode
  */
void ISO7816_SetWT(uint16_t time);

/** End of ISO7816_Exported_Functions
  * \}
  */

/** End of ISO7816
  * \}
  */

#ifdef __cplusplus
}
#endif

#endif /* RTL_ISO7816_H */



