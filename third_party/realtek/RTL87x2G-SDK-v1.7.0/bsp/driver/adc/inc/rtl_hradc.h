/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef RTL_HRADC_H
#define RTL_HRADC_H

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "utils/rtl_utils.h"
#if defined (CONFIG_SOC_SERIES_RTL87X2G)
#include "adc/src/rtl87x2g/rtl_hradc_def.h"
#endif

/*============================================================================*
 *                          Private Macros
 *============================================================================*/
#define HRADC_FIFO_LENGTH                         32
#define HRADC_FIFO_WIDTH                          24
#define MAX_HRADC_SCH_NUM                         8
#define IS_HRADC_PERIPH(PERIPH) ((PERIPH) == HRADC)

/** \defgroup HRADC   HRADC
  * \brief
  * \{
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/
/** \defgroup HRADC_Exported_Constants HRADC Exported Constants
  * \brief
  * \{
  */

/**
 * \defgroup    HRADC_Channel_Index HRADC Channel Index
 * \{
 * \ingroup     HRADC_Exported_Constants
 */
#define HRADC_Channel_Index_0         0      //!< HRADC channel 0.
#define HRADC_Channel_Index_1         1      //!< HRADC channel 1.
#define HRADC_Channel_Index_2         2      //!< HRADC channel 2.
#define HRADC_Channel_Index_3         3      //!< HRADC channel 3.

/** End of HRADC_Channel_Index
  * \}
  */

/**
 * \defgroup    HRADC_Schedule_Index HRADC Schedule Index
 * \{
 * \ingroup     HRADC_Exported_Constants
 */
#define HRADC_Schedule_Index_0        0      //!< HRADC schedule index 0.
#define HRADC_Schedule_Index_1        1      //!< HRADC schedule index 1.
#define HRADC_Schedule_Index_2        2      //!< HRADC schedule index 2.
#define HRADC_Schedule_Index_3        3      //!< HRADC schedule index 3.
#define HRADC_Schedule_Index_4        4      //!< HRADC schedule index 4.
#define HRADC_Schedule_Index_5        5      //!< HRADC schedule index 5.
#define HRADC_Schedule_Index_6        6      //!< HRADC schedule index 6.
#define HRADC_Schedule_Index_7        7      //!< HRADC schedule index 7.
#define IS_HRADC_SCH_INDEX(IDEX) ((IDEX) <= 7) //!< Specify the maximum HRADC schedule number.

/** End of HRADC_Schedule_Index
  * \}
  */


/**
 * \defgroup    HRADC_LDO_Mode HRADC LDO Mode
 * \{
 * \ingroup     HRADC_Exported_Constants
 */
typedef enum
{
    HRADC_28_LDO_MODE = 0x00, //!< HRADC_LDO_2.8V_MODE.
    HRADC_18_LDO_MODE = 0x01, //!< HRADC_LDO_1.8V_MODE.
    HRADC_Bypass_MODE = 0x02, //!< HRADC_Bypass_MODE.
} HRADCMode_TypeDef;

#define IS_HRADC_MODE(MODE) (((MODE) == HRADC_28_LDO_Mode)\
                             || ((MODE) == HRADC_18_LDO_Mode)\
                             || ((MODE) == HRADC_Bypass_Mode)) //!< Check if the input parameter is valid.

/** End of HRADC_LDO_Mode
  * \}
  */

/**
 * \defgroup    HRADC_Write_Data_To_FIFO HRADC Write Data To FIFO
 * \{
 * \ingroup     HRADC_Exported_Constants
 */
typedef enum
{
    HRADC_DATA_WRITE_TO_FIFO_DISABLE = 0x00,  //!< Disable HRADC data write to FIFO.
    HRADC_DATA_WRITE_TO_FIFO_ENABLE = 0x01,   //!< Enable HRADC data write to FIFO.
} HRADCDataWriteToFIFOCmd_TypeDef;

#define IS_HRADC_DATA_WRITE_TO_FIFO_TYPE(TYPE) (((TYPE) == HRADC_DATA_WRITE_TO_FIFO_DISABLE)\
                                                || ((TYPE) == HRADC_DATA_WRITE_TO_FIFO_ENABLE)) //!< Check if the input parameter is valid.

/** End of HRADC_Write_Data_To_FIFO
  * \}
  */

/**
 * \defgroup    HRADC_Operation_Mode HRADC Operation Mode
 * \{
 * \ingroup     HRADC_Exported_Constants
 */
typedef enum
{
    HRADC_ONE_SHOT_MODE = 0x00,    //!< HRADC continuous mode.
    HRADC_CONTINUOUS_MODE = 0x01,  //!< HRADC one shot mode.
} HRADCSampleMode_TypeDef;

#define IS_HRADC_SAMPLE_MODE(MODE) (((MODE) == HRADC_ONE_SHOT_MODE)\
                                    || ((MODE) == HRADC_ONE_SHOT_MODE))) //!< Check if the input parameter is valid.

/** End of HRADC_Operation_Mode
  * \}
  */

/**
 * \defgroup    HRADC_Gain_Control HRADC Gain Control
 * \{
 * \ingroup     HRADC_Exported_Constants
 */
typedef enum
{
    HRADC_GAIN_1X,  //!< Gain of 1X
    HRADC_GAIN_2X,  //!< Gain of 2X
    HRADC_GAIN_4X,  //!< Gain of 4X
    HRADC_GAIN_8X,  //!< Gain of 8X
    HRADC_GAIN_16X, //!< Gain of 16X
    HRADC_GAIN_32X, //!< Gain of 32X
    HRADC_GAIN_64X, //!< Gain of 64X
} HRADCGainCtrl_TypeDef;

#define IS_HRADC_GAIN_CTRL_TYPE(TYPE) (((TYPE) == HRADC_GAIN_1X)\
                                       || ((TYPE) == HRADC_GAIN_2X)\
                                       || ((TYPE) == HRADC_GAIN_4X)\
                                       || ((TYPE) == HRADC_GAIN_8X)\
                                       || ((TYPE) == HRADC_GAIN_16X)\
                                       || ((TYPE) == HRADC_GAIN_32X)\
                                       || ((TYPE) == HRADC_GAIN_64X)) //!< Check if the input parameter is valid.

/** End of HRADC_Gain_Control
  * \}
  */

/**
 * \defgroup    HRADC_Output_Data_Rate HRADC Output Data Rate
 * \{
 * \ingroup     HRADC_Exported_Constants
 */
typedef enum
{
    HRADC_2_5_SPS = 0x00,       //!< Output data rate is 2.5SPS.
    HRADC_5_SPS = 0x01,         //!< Output data rate is 5SPS.
    HRADC_10_SPS = 0x02,        //!< Output data rate is 10SPS.
    HRADC_20_SPS = 0x03,        //!< Output data rate is 20SPS.
    HRADC_40_SPS = 0x04,        //!< Output data rate is 40SPS.
    HRADC_80_SPS = 0x05,        //!< Output data rate is 80SPS.
    HRADC_160_SPS = 0x06,       //!< Output data rate is 160SPS.
    HRADC_320_SPS = 0x07,       //!< Output data rate is 320SPS.
    HRADC_640_SPS = 0x08,       //!< Output data rate is 640SPS.
    HRADC_1280_SPS  = 0x09,     //!< Output data rate is 1280SPS.
    HRADC_6_25_SPS = 0x10,      //!< Output data rate is 6.25SPS.
    HRADC_12_5_SPS = 0x11,      //!< Output data rate is 12.5SPS.
    HRADC_25_SPS = 0x12,        //!< Output data rate is 25SPS.
    HRADC_50_SPS = 0x13,        //!< Output data rate is 50SPS.
    HRADC_100_SPS = 0x14,       //!< Output data rate is 100SPS.
    HRADC_200_SPS = 0x15,       //!< Output data rate is 200SPS.
    HRADC_400_SPS = 0x16,       //!< Output data rate is 400SPS.
    HRADC_800_SPS = 0x17,       //!< Output data rate is 800SPS.
    HRADC_1600_SPS = 0x18,      //!< Output data rate is 1600SPS.
    HRADC_3200_SPS = 0x19,      //!< Output data rate is 3200SPS.
} HRADCOutputDataRate_TypeDef;

#define IS_HRADC_OUTPUT_DATA_RATE_TYPE(TYPE) ((TYPE) <= 0x19) //!< Check if the input parameter is valid.

/** End of HRADC_Output_Data_Rate
  * \}
  */

/**
 * \defgroup    HRADC_FIFO_Threshold HRADC FIFO Threshold
 * \{
 * \ingroup     HRADC_Exported_Constants
 */
#define IS_HRADC_FIFO_THRESHOLD(THD) ((THD) <= 0x1F) //!< Check if the input parameter is valid.

/** End of HRADC_FIFO_Threshold
  * \}
  */

/**
 * \defgroup    HRADC_Burst_Size HRADC Burst Size
 * \{
 * \ingroup     HRADC_Exported_Constants
 */
#define IS_HRADC_BURST_SIZE_CONFIG(CONFIG) ((CONFIG) <= 0x1F) //!< Check if the input parameter is valid.
#define IS_HRADC_BITMAP(IDEX) ((IDEX) <= 0xFFFF)              //!< Check if the input parameter is valid.

/** End of HRADC_Burst_Size
  * \}
  */

/**
 * \defgroup    HRADC_Interrupts_Definition HRADC Interrupts Definition
 * \{
 * \ingroup     HRADC_Exported_Constants
 */
#define HRADC_INT_FIFO_THR                          ((uint32_t)(1 << 0)) //!< HRADC FIFO threshold interrupt. 
#define HRADC_INT_FIFO_OVERFLOW                     ((uint32_t)(1 << 1)) //!< HRADC FIFO overflow interrupt. 
#define HRADC_INT_SCAN_ONEROUND_DOWN                ((uint32_t)(1 << 2)) //!< HRADC scan one round down interrupt.
#define HRADC_INT_DATA_READY                        ((uint32_t)(1 << 3)) //!< HRADC data ready interrupt.
#define HRADC_INT_BOOT_READY                        ((uint32_t)(1 << 4)) //!< HRADC boot ready interrupt.

#define IS_HRADC_INT(INT) (((INT) == HRADC_INT_FIFO_THR) || ((INT) == HRADC_INT_FIFO_OVERFLOW)\
                           ||((INT) == HRADC_INT_SCAN_ONEROUND_DOWN) || ((INT) == HRADC_INT_DATA_READY)\
                           ||((INT) == HRADC_INT_BOOT_READY)) //!< Check if the input parameter is valid.

/** End of HRADC_Interrupts_Definition
  * \}
  */

/**
 * \defgroup    HRADC_Schedule_Table HRADC Channel and Mode
 * \{
 * \ingroup     HRADC_Exported_Constants
 */
#define HRADC_SINGLE_ENDED_MODE(index)              ((uint16_t)((0x00 << 2) | (index))) //!< Check if the input parameter is valid.
#define HRADC_DIFFERENTIAL_MODE(index)              ((uint16_t)((0x01 << 2) | (index))) //!< Check if the input parameter is valid.

/** End of HRADC_Schedule_Table
  * \}
  */

/** End of HRADC_Exported_Constants
  * \}
  */

/*============================================================================*
 *                         Types
 *============================================================================*/
/** \defgroup HRADC_Exported_Types HRADC Exported Types
  * \brief
  * \{
  */

/**
 * \brief       HRADC init structure definition.
 *
 * \ingroup     HRADC_Exported_Types
 */
typedef struct
{
    HRADCGainCtrl_TypeDef HRADC_Gain_Crtol;                /**< Specify the PGA gain control.
                                                                This parameter can be a value of \ref HRADC_Gain_Control. */

    HRADCDataWriteToFIFOCmd_TypeDef
    HRADC_DataWriteToFifo; /**< Enable or disable writing HRADC sampling data to FIFO in one shot mode.
                                                                This parameter can be a value of \ref HRADC_Write_Data_To_FIFO. */

    HRADCSampleMode_TypeDef
    HRADC_SampleMode;              /**< Enable or disable writing HRADC scan mode.
                                                                This parameter can be a value of \ref HRADC_Operation_Mode.*/

    HRADCOutputDataRate_TypeDef HRADC_OutPutDataRate;      /**< Specify the HRADC output data rate.
                                                                This parameter can be a value of \ref HRADC_Output_Data_Rate.*/

    uint32_t HRADC_FifoThdLevel;                           /**< Specify the HRADC FIFO threshold to trigger \ref HRADC_INT_FIFO_THD interrupt.
                                                                This parameter can be a value of 0 to 31. */

    uint32_t HRADC_BurstSize;                              /**< Specify the HRADC FIFO burst size to trigger GDMA.
                                                                This parameter can be a value of 0 to 31. */

    uint16_t HRADC_SchIndex[8];                            /**< Specify HRADC mode and channel for schedule table.
                                                                This parameter can be a value of \ref HRADC_Schedule_Table. */

    uint8_t HRADC_Bitmap;                                  /**< Specify the schedule table channel map.
                                                                This parameter can be a value of 8-bit map. */

    uint16_t HRADC_DataDiscard;                            /**< Specify the discard the first sdadc_data_discard data.
                                                                This parameter can be a value of 0 to 0x1f. */

    uint16_t HRADC_SwitchInterval;                         /**< Specify the channel switch interval config.
                                                                This parameter can be a value of 0 to 0xf.
                                                                Actual interval = 1/(8MHz)*(2^HRADC_SwitchInterval+2). */

    HRADCMode_TypeDef HRADC_Mode;                          /**< Specify the SDADC mode.
                                                                This parameter can be a value of \ref HRADC_LDO_Mode.*/
} HRADC_InitTypeDef;

/** End of HRADC_Exported_Types
  * \}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** \defgroup HRADC_Exported_Functions HRADC Exported Functions
  * \brief
  * \{
  */

/**
 * \brief   Deinitialize the HRADC peripheral registers to their
 *          default reset values(turn off HRADC clock).
 * \details
 * \param[in] HRADCx  Specify HRADC peripheral, can only be HRADC.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_HRADC_init(void)
 * {
 *    //Turn off the clock.
 *    HRADC_DeInit(HRADC);
 * }
 * \endcode
 */
void HRADC_DeInit(void);

/**
 * \brief Initialize the HRADC peripheral according to the specified
 *     parameters in the HRADC_InitStruct
 * \param[in]  HRADCx            Selected HRADC peripheral.
 * \param[in]  HRADC_InitStruct  Pointer to a HRADC_InitTypeDef structure which will be initialized.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_HRADC_init(void)
 * {
 *    //Turn on the clock.
 *    RCC_PeriphClockCmd(APBPeriph_HRADC, APBPeriph_HRADC_CLOCK, ENABLE);
 *    HRADC_InitTypeDef HRADC_InitStruct;
 *    HRADC_StructInit(&HRADC_InitStruct);
 *    HRADC_InitStruct.HRADC_SchIndex[0] = EXT_SINGLE_ENDED(0);
 *    HRADC_InitStruct.HRADC_SchIndex[1] = EXT_SINGLE_ENDED(1);
 *    HRADC_InitStruct.HRADC_Bitmap = 0x03;
 *    //Add other initialization parameters that need to be configured here.
 *    HRADC_Init(HRADC, &HRADC_InitStruct);
 * }
 * \endcode
 */
void HRADC_Init(HRADC_TypeDef *HRADCx, HRADC_InitTypeDef *HRADC_InitStruct);

/**
 * \brief   Fill each HRADC_InitStruct member with its default value.
 * \param[in] HRADC_InitStruct  Pointer to a HRADC_InitTypeDef structure which will be initialized.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_HRADC_init(void)
 * {
 *    //Turn on the clock.
 *    RCC_PeriphClockCmd(APBPeriph_HRADC, APBPeriph_HRADC_CLOCK, ENABLE);
 *    HRADC_InitTypeDef HRADC_InitStruct;
 *    HRADC_StructInit(&HRADC_InitStruct);
 *    HRADC_InitStruct.HRADC_SchIndex[0] = EXT_SINGLE_ENDED(0);
 *    HRADC_InitStruct.HRADC_SchIndex[1] = EXT_SINGLE_ENDED(1);
 *    HRADC_InitStruct.HRADC_Bitmap = 0x03;
 *    //Add other initialization parameters that need to be configured here.
 *    HRADC_Init(HRADC, &HRADC_InitStruct);
 * }
 * \endcode
 * \callgraph
 *
 */
void HRADC_StructInit(HRADC_InitTypeDef *HRADC_InitStruct);

/**
 * \brief   Enable or disable the HRADC peripheral.
 * \param[in]  HRADCx     Specify HRADC peripheral.
 * \param[in]  HRADCMode  HRADC operation mode selection.
 *                        This parameter can be one of the following values:
 *                        \arg HRADC_ONE_SHOT_MODE: One shot mode.
 *                        \arg HRADC_CONTINUOUS_MODE: Continuous sampling mode.
 * \param[in]  NewState   New state of the HRADC peripheral.
 *                        - ENABLE: Enable the specified HRADC peripheral.
 *                        - DISABLE: Disable the specified HRADC peripheral.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void board_HRADC_init(void)
 * {
 *     Pad_Config(P2_0, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
 *                PAD_OUT_LOW);
 *
 *     Pad_Config(P2_1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
 *                PAD_OUT_LOW);
 * }
 *
 * void driver_HRADC_init(void)
 * {
 *    //open clock
 *    RCC_PeriphClockCmd(APBPeriph_HRADC, APBPeriph_HRADC_CLOCK, ENABLE);
 *
 *    HRADC_InitTypeDef HRADC_InitStruct;
 *    HRADC_StructInit(&HRADC_InitStruct);
 *    HRADC_InitStruct.HRADC_SchIndex[0] = EXT_SINGLE_ENDED(0);
 *    HRADC_InitStruct.HRADC_SchIndex[1] = EXT_SINGLE_ENDED(1);
 *    HRADC_InitStruct.HRADC_Bitmap = 0x03;
 *    //Add other initialization parameters here.
 *    HRADC_Init(HRADC, &HRADC_InitStruct);
 *
 *    HRADC_INTConfig(HRADC, HRADC_INT_ONE_SHOT_DONE, ENABLE);
 * }
 *
 * void HRADC_demo(void)
 * {
 *    board_HRADC_init();
 *    driver_HRADC_init();
 *    HRADC_Cmd( ENABLE);
 * }
 * \endcode
 */
void HRADC_Cmd(FunctionalState NewState);

/**
 * \brief   Enable or disable the specified HRADC interrupts.
 * \param[in]  HRADCx    Specify HRADC peripheral.
 * \param[in]  HRADC_IT  Specify the HRADC interrupts sources to be enabled or disabled.
 *                       This parameter can be any combination of the following values:
 *                       \arg HRADC_INT_FIFO_THR :HRADC FIFO size exceeds the threshold interrupt.
 *                       \arg HRADC_INT_FIFO_OVERFLOW :HRADC FIFO overflow interrupt.
 *                       \arg HRADC_INT_SCAN_ONEROUND_DOWN :HRADC one round done interrupt.
 *                       \arg HRADC_INT_DATA_READY :HRADC data ready interrupt.
 *                       \arg HRADC_INT_BOOT_READY :HRADC boot ready interrupt
 * \param[in]  newState  New state of the specified HRADC interrupt.
 *                       This parameter can be one of the following values:
 *                       - ENABLE: Enable the specified HRADC interrupts.
 *                       - DISABLE: Disable the specified HRADC interrupts.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_HRADC_init(void)
 * {
 *    //open clock
 *    RCC_PeriphClockCmd(APBPeriph_HRADC, APBPeriph_HRADC_CLOCK, ENABLE);
 *
 *    HRADC_InitTypeDef HRADC_InitStruct;
 *    HRADC_StructInit(&HRADC_InitStruct);
 *    HRADC_InitStruct.HRADC_SchIndex[0] = EXT_SINGLE_ENDED(0);
 *    HRADC_InitStruct.HRADC_SchIndex[1] = EXT_SINGLE_ENDED(1);
 *    HRADC_InitStruct.HRADC_Bitmap = 0x03;
 *    //Add other initialization parameters here.
 *    HRADC_Init(HRADC, &HRADC_InitStruct);
 *
 *    HRADC_INTConfig(HRADC, HRADC_INT_FIFO_RD_ERR, ENABLE);
 *    HRADC_INTConfig(HRADC, HRADC_INT_ONE_SHOT_DONE, ENABLE);
 * }
 * \endcode
 *
 */
void HRADC_INTConfig(HRADC_TypeDef *HRADCx, uint32_t HRADC_INT,
                     FunctionalState NewState);
/**
 * \brief   Mask the specified HRADC interrupts.
 * \param[in]  HRADCx    Specify HRADC peripheral.
 * \param[in]  HRADC_IT  Specify the HRADC interrupts sources to be enabled or disabled.
 *                       This parameter can be any combination of the following values:
 *                       \arg HRADC_INT_FIFO_THR :HRADC FIFO size exceeds the threshold interrupt.
 *                       \arg HRADC_INT_FIFO_OVERFLOW :HRADC FIFO overflow interrupt.
 *                       \arg HRADC_INT_SCAN_ONEROUND_DOWN :HRADC one round done interrupt.
 *                       \arg HRADC_INT_DATA_READY :HRADC data ready interrupt.
 *                       \arg HRADC_INT_BOOT_READY :HRADC boot ready interrupt
 * \param[in]  newState  New state of the specified HRADC interrupt.
 *                       This parameter can be one of the following values:
 *                       - ENABLE: Mask the specified HRADC interrupts.
 *                       - DISABLE: Unmask the specified HRADC interrupts.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void driver_HRADC_init(void)
 * {
 *    //open clock
 *    RCC_PeriphClockCmd(APBPeriph_HRADC, APBPeriph_HRADC_CLOCK, ENABLE);
 *
 *    HRADC_InitTypeDef HRADC_InitStruct;
 *    HRADC_StructInit(&HRADC_InitStruct);
 *    HRADC_InitStruct.HRADC_SchIndex[0] = EXT_SINGLE_ENDED(0);
 *    HRADC_InitStruct.HRADC_SchIndex[1] = EXT_SINGLE_ENDED(1);
 *    HRADC_InitStruct.HRADC_Bitmap = 0x03;
 *    //Add other initialization parameters here.
 *    HRADC_Init(HRADC, &HRADC_InitStruct);
 *
 *    HRADC_MaskINTConfig(HRADC, HRADC_INT_FIFO_RD_ERR, ENABLE);
 *    HRADC_MaskINTConfig(HRADC, HRADC_INT_ONE_SHOT_DONE, ENABLE);
 * }
 * \endcode
 *
 */
void HRADC_MaskINTConfig(HRADC_TypeDef *HRADCx, uint32_t HRADC_INT,
                         FunctionalState NewState);
/**
 * \brief      Read HRADC data according to specific channel.
 *
 * \param[in]  HRADCx  Specify HRADC peripheral.
 * \param[in]  index   Can be 0 to 15.
 *
 * \return     The 12-bit converted HRADC raw data.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void board_HRADC_init(void)
 * {
 *     Pad_Config(P2_0, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
 *                PAD_OUT_LOW);
 *
 *     Pad_Config(P2_1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
 *                PAD_OUT_LOW);
 * }
 *
 * void driver_HRADC_init(void)
 * {
 *    //open clock
 *    RCC_PeriphClockCmd(APBPeriph_HRADC, APBPeriph_HRADC_CLOCK, ENABLE);
 *
 *    HRADC_InitTypeDef HRADC_InitStruct;
 *    HRADC_StructInit(&HRADC_InitStruct);
 *    HRADC_InitStruct.HRADC_SchIndex[0] = EXT_SINGLE_ENDED(0);
 *    HRADC_InitStruct.HRADC_SchIndex[1] = EXT_SINGLE_ENDED(1);
 *    HRADC_InitStruct.HRADC_Bitmap = 0x03;
 *    //Add other initialization parameters here.
 *    HRADC_Init(HRADC, &HRADC_InitStruct);
 *
 *    HRADC_INTConfig(HRADC, HRADC_INT_ONE_SHOT_DONE, ENABLE);
 * }
 *
 * void HRADC_demo(void)
 * {
 *    board_HRADC_init();
 *    driver_HRADC_init();
 *    HRADC_Cmd(ENABLE);
 *    while(HRADC_GetINTStatus(HRADC, HRADC_INT_ONE_SHOT_DONE) == RESET);
 *    uint16_t raw_data_0 = HRADC_ReadRawData(HRADC, 0);
 *    uint16_t raw_data_1 = HRADC_ReadRawData(HRADC, 1);
 * }
 * \endcode
 */
uint32_t HRADC_ReadRawData(HRADC_TypeDef *HRADCx, uint8_t Index);

/**
 * \brief  Read one byte data from HRADC FIFO.
 *
 * \param[in]  HRADCx  Selected HRADC peripheral.
 *
 * \return HRADC FIFO data.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void board_HRADC_init(void)
 * {
 *     Pad_Config(P2_0, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
 *                PAD_OUT_LOW);
 * }
 *
 * void driver_HRADC_init(void)
 * {
 *    //open clock
 *    RCC_PeriphClockCmd(APBPeriph_HRADC, APBPeriph_HRADC_CLOCK, ENABLE);
 *
 *    HRADC_InitTypeDef HRADC_InitStruct;
 *    HRADC_StructInit(&HRADC_InitStruct);
 *    HRADC_InitStruct.HRADC_SchIndex[0] = EXT_SINGLE_ENDED(0);
 *    HRADC_InitStruct.HRADC_Bitmap = 0x01;
 *    HRADC_InitStruct.HRADC_DataWriteToFifo = HRADC_DATA_WRITE_TO_FIFO_DISABLE;
 *    HRADC_Init(HRADC, &HRADC_InitStruct);
 *
 *    HRADC_INTConfig(HRADC, HRADC_INT_ONE_SHOT_DONE, ENABLE);
 * }
 *
 * void HRADC_demo(void)
 * {
 *    board_HRADC_init();
 *    driver_HRADC_init();
 *    HRADC_Cmd(ENABLE);
 *    while(HRADC_GetINTStatus(HRADC, HRADC_INT_ONE_SHOT_DONE) == RESET);
 *    uint16_t raw_data = 0;
 *    raw_data = HRADC_ReadFIFO(HRADC);
 * }
 * \endcode
 */
uint32_t HRADC_ReadFIFO(HRADC_TypeDef *HRADCx);

/**
 * \brief   Get data from HRADC FIFO.
 *
 * \param[in]  HRADCx  Specify HRADC peripheral.
 * \param[out] outBuf  Buffer to save data read from HRADC FIFO.
 * \param[in]  num     Number of data to be read.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void HRADC_demo(void)
 * {
 *    //HRADC already start
 *    uint16_t raw_data[32] = {0};
 *    uint8_t data_len = HRADC_GetFIFODataLen(HRADC);
 *    HRADC_ReadFIFOData(HRADC,raw_data,data_len);
 * }
 * \endcode
 *
 */
void HRADC_ReadFIFOData(HRADC_TypeDef *HRADCx, uint16_t *OutBuf, uint16_t Num);

/**
 * \brief   Get HRADC FIFO data number.
 *
 * \param[in] HRADCx  Selected HRADC peripheral.
 *
 * \return  Current data number in HRADC FIFO.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void HRADC_demo(void)
 * {
 *    //HRADC already start
 *    uint16_t raw_data[32] = {0};
 *    uint8_t data_len = HRADC_GetFIFODataLen(HRADC);
 *    HRADC_ReadFIFOData(HRADC,raw_data,data_len);
 * }
 * \endcode
 *
 */
uint8_t HRADC_GetFIFODataLen(HRADC_TypeDef *HRADCx);

/**
 * \brief   Config HRADC schedule table.
 *
 * \param[in]  HRADCx     Specify HRADC peripheral.
 * \param[in]  HRADCMode  HRADC operation mode.
 *                        This parameter can be one of the following values:
 *                        \arg EXT_SINGLE_ENDED(index)
 *                        \arg EXT_DIFFERENTIAL(index)
 *                        \arg INTERNAL_VBAT_MODE
 * \param[in]  Index      Schedule table index.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void HRADC_demo(void)
 * {
 *   HRADC_SchIndexConfig(HRADC,INTERNAL_VBAT_MODE,0);
 * }
 * \endcode
 *
 */
void HRADC_SchIndexConfig(HRADC_TypeDef *HRADCx, uint8_t HRADCMode, uint16_t Index);

/**
 * \brief   Config HRADC schedule table.
 *
 * \param[in]  HRADCx      Specify HRADC peripheral.
 * \param[in]  BitMap      HRADC bit map.
 * \param[in]  NewState    New state of the HRADC peripheral.
 *                         This parameter can be one of the following values:
 *                         - ENABLE: Enable the HRADC schedule table.
 *                         - DISABLE: Disable the HRADC schedule table.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void HRADC_demo(void)
 * {
 *   uint16_t bit_map = 0x03;
 *   HRADC_BitMapConfig(HRADC,bit_map,ENABLE);
 * }
 * \endcode
 */
void HRADC_BitMapConfig(HRADC_TypeDef *HRADCx, uint16_t BitMap,
                        FunctionalState NewState);
/**
 * \brief   Power on HRADC manually.
 *
 * \param[in]  HRADCx    Specify HRADC peripheral.
 * \param[in]  NewState  New state of the HRADC power on.
 *                       This parameter can be one of the following values:
 *                       - ENABLE: Enable the manual power on function.
 *                       - DISABLE: Disable the manual power on function.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void HRADC_demo(void)
 * {
 *   HRADC_ManualPowerOnCmd(HRADC,ENABLE);
 * }
 * \endcode
 */
void HRADC_ManualPowerOnCmd(HRADC_TypeDef *HRADCx, FunctionalState NewState);

/**
 * \brief  Get the specified HRADC interrupt flag status.
 *
 * \param[in]  HRADCx          Selected HRADC peripheral.
 * \param[in]  HRADC_INT_FLAG  Specify the interrupt flag to check.
 *                             This parameter can be one of the following values:
 *                             \arg HRADC_INT_ONE_SHOT_DONE: HRADC once convert end interrupt.
 *                             \arg HRADC_INT_FIFO_OVERFLOW: HRADC FIFO overflow interrupt.
 *                             \arg HRADC_INT_FIFO_THD: FIFO data size is larger than threshod interrupt.
 *                             \arg HRADC_INT_FIFO_RD_ERR: HRADC FIFO underflow interrupt.
 *                             \arg HRADC_INT_FIFO_RD_REQ: HRADC FIFO size>=burst size interrupt.
 *
 * \return The new state of HRADC_INT (SET or RESET).
 *         - SET: The HRADC interrupt status is set..
 *         - RESET: The HRADC interrupt status is reset.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void HRADC_demo(void)
 * {
 *     ITStatus int_status = RESET;
 *     int_status = HRADC_GetINTStatus(HRADC,HRADC_INT_FIFO_OVERFLOW);
 * }
 * \endcode
 */
ITStatus HRADC_GetINTStatus(HRADC_TypeDef *HRADCx, uint32_t HRADC_INT);

/**
 * \brief  Clear the HRADC interrupt pending bit.
 *
 * \param[in] HRADCx     Specify HRADC peripheral.
 * \param[in] HRADC_INT  Specify the interrupt pending bit to clear.
 *                       This parameter can be any combination of the following values:
 *                       - HRADC_INT_ONE_SHOT_DONE: HRADC once convert end interrupt.
 *                       - HRADC_INT_FIFO_OVERFLOW: HRADC FIFO overflow interrupt.
 *                       - HRADC_INT_FIFO_THD: FIFO larger than threshold interrupt.
 *                       - HRADC_INT_FIFO_RD_ERR: HRADC FIFO underflow interrupt.
 *                       - HRADC_INT_FIFO_RD_REQ: HRADC FIFO size>=burst size interrupt.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void HRADC_demo(void)
 * {
 *   HRADC_ClearINTPendingBit(HRADC,HRADC_INT_FIFO_OVERFLOW);
 * }
 * \endcode
 */
void HRADC_ClearINTPendingBit(HRADC_TypeDef *HRADCx, uint32_t HRADC_INT);

/**
 * \brief   Clear HRADC FIFO.
 *
 * \param[in] HRADCx  Specify HRADC peripheral.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void HRADC_demo(void)
 * {
 *   HRADC_ClearFIFO(HRADC);
 * }
 * \endcode
 */
void HRADC_ClearFIFO(HRADC_TypeDef *HRADCx);

/**
 * \brief   Enbale or disable stop fifo from writing data.
 *
 * \param[in] HRADCx    Specify HRADC peripheral.
 * \param[in] NewState  New state of the ADC fifo write.
 *                      This parameter can be one of the following values:
 *                      - ENABLE: Enable stop fifo from writing data.
 *                      - DISABLE: Disable stop fifo from writing data.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void HRADC_demo(void)
 * {
 *   HRADC_WriteFIFOCmd(ENABLE);
 * }
 * \endcode
 */
void HRADC_WriteFIFOCmd(HRADC_TypeDef *HRADCx, FunctionalState NewState);

/**
 * \brief   Get all HRADC interrupt flag status.
 *
 * \param[in] HRADCx  Specify HRADC peripheral.
 *
 * \return  All HRADC interrupt status.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void HRADC_demo(void)
 * {
 *   uint8_t all_flag_status = 0;
 *   all_flag_status = HRADC_GetAllFlagStatus(HRADC);
 * }
 * \endcode
 *
 */
uint8_t HRADC_GetAllFlagStatus(HRADC_TypeDef *HRADCx);

/** End of HRADC_Exported_Functions
  * \}
  */

/** End of HRADC
  * \}
  */

#ifdef __cplusplus
}
#endif

#endif /* RTL_HRADC_H */

