/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _BLS_DEFINE_H_
#define _BLS_DEFINE_H_

/** @defgroup BLS Blood Pressure Service
  * @brief Blood Pressure Service
  * @{
  */

/** @defgroup BLS_DEFINE  BLS Define
  * @brief Blood Pressure Service define file
  * @{
  */

/*============================================================================*
 *                         Macros
 *============================================================================*/
/** @defgroup BLS_Common_Exported_Macros BLS Common Exported Macros
  * @{
  */

/** @defgroup BLS_Char_Sup BLS Characteristic Support Configuration
  * @{
  */
#define BLS_INTERMEDIATE_CUFF_PRESSURE_SUPPORT    1 /**< Whether to support Intermediate Cuff Pressure characteristic.
                                                        - 0: Do not support Intermediate Cuff Pressure characteristic.
                                                        - 1: Support Intermediate Cuff Pressure characteristic. */
/** @} End of BLS_Char_Sup */

/** @defgroup BLS_UUID BLS Service and Characteristic UUID
  * @{
  */
#define GATT_UUID_BLOOD_PRESSURE                        0x1810  /**< Blood Pressure Service. */
#define GATT_UUID_CHAR_BLP_MEASUREMENT                  0x2A35  /**< Blood Pressure Measurement characteristic UUID. */
#define GATT_UUID_CHAR_INTERMEDIATE_CUFF_PRESSURE       0x2A36  /**< Intermediate Cuff Pressure characteristic UUID. */
#define GATT_UUID_CHAR_BLP_FEATURE                      0x2A49  /**< Blood Pressure Feature characteristic UUID. */
/** @} End of BLS_UUID */


/** @defgroup BLS_Related_Def BLS Service Related Definitions
  * @{
  */
#define BLS_MEASUREMENT_VALUE_MAX_LEN  25  /**< Maximum value length for Blood Pressure Measurement characteristic. */

#define BLS_MEASUREMENT_COMPOUND_VALUE_NAN  0x07ff  /**< Blood Pressure Measurement Compound Value field is unavailable. */
#define BLS_MEASUREMENT_USER_ID_UNKNOW      0xFF  /**< Blood Pressure Measurement User ID field is unknown. */
/** @} End of BLS_Related_Def */

/** @defgroup Blood_Pressure_Units_Flag Blood Pressure Units Flag
  * @{
  */
#define BLS_MEASUREMENT_VALUE_UNITS_MMHG      0  /**< Blood pressure for Systolic, Diastolic and MAP in units of mmHg. */
#define BLS_MEASUREMENT_VALUE_UNITS_KPA       1  /**< Blood pressure for Systolic, Diastolic and MAP in units of kPa. */
/** @} End of Blood_Pressure_Units_Flag */

/** @defgroup BLS_Measurement_Flags Blood Pressure Measurement characteristic Flags Bits
  * @brief Define which data fields are present in the Blood Pressure Measurement characteristic value.
  * @{
  */
#define BLS_FLAG_MEASUREMENT_UINT_BIT                       0x01  /**< Blood Pressure Units Flag. */
#define BLS_FLAG_MEASUREMENT_TIMESTAMP_PRESENT_BIT          0x02  /**< Time Stamp Flag. */
#define BLS_FLAG_MEASUREMENT_PULSE_RATE_BIT                 0x04  /**< Pulse Rate Flag. */
#define BLS_FLAG_MEASUREMENT_USER_ID_BIT                    0x08  /**< User ID Flag. */
#define BLS_FLAG_MEASUREMENT_STATUS_BIT                     0x10  /**< Measurement Status Flag. */
/** @} End of BLS_Measurement_Flags */

/** @defgroup Blood_Pressure_Feature_Bit Blood Pressure Feature Characteristic Bits
  * @brief The Blood Pressure Feature characteristic is used to represent the supported features of a blood pressure sensor.
  * @{
  */
#define BLS_FEATURE_BODY_MOVEMENT_DETECTION_SUPPORT_BIT           0x01  /**< Body Movement Detection Support. */
#define BLS_FEATURE_CUFF_FIT_DETECTION_SUPPORT_BIT                0x02  /**< Cuff Fit Detection Support. */
#define BLS_FEATURE_IRREGULAR_PULSE_DETECTION_SUPPORT_BIT         0x04  /**< Irregular Pulse Detection Support. */
#define BLS_FEATURE_PULSE_RATE_RANGE_DETECTION_SUPPORT_BIT        0x08  /**< Pulse Rate Range Detection Support. */
#define BLS_FEATURE_MEASUREMENT_POSITION_DETECTION_SUPPORT_BIT    0x10  /**< Measurement Position Detection Support. */
#define BLS_FEATURE_MULTIPLE_BOND_SUPPORT_BIT                     0x20  /**< Multiple Bond Support. */
/** @} End of Blood_Pressure_Feature_Bit */

/** @} End of BLS_Common_Exported_Macros */

/*============================================================================*
 *                         Types
 *============================================================================*/
/** @defgroup BLS_Common_Exported_Types BLS Common Exported Types
  * @brief
  * @{
  */
/** @details
   4 bit (MSB) exponent, 12 bit mantissa.
*/
typedef uint8_t     SFLOAT[2];

/**
*  @brief Blood Pressure Measurement Compound Value field.
*/
typedef struct
{
    SFLOAT bp_meas_systolic_value;
    SFLOAT bp_meas_diastolic_value;
    SFLOAT bp_meas_map_value;
} T_BLOOD_PRESSUREE_MEAS_COMPOUND_VALUE;

/**
*  @brief Time Stamp field.
*/
typedef struct
{
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
} T_BLS_TIME_STAMP;

/**
*  @brief Measurement Status field.
*/
typedef struct
{
    uint16_t bp_meas_body_movement_detection_flag: 1;
    uint16_t bp_meas_cuff_fit_detection_flag: 1;
    uint16_t bp_meas_irregular_pulse_detection_flag: 1;
    uint16_t bp_meas_pulse_rate_range_detection_flag: 2;
    uint16_t bp_meas_measurement_position_detection_flag: 1;
    uint16_t rfu: 10;
} T_BLOOD_PRESSUREE_MEAS_STATUS;

/**
*  @brief Blood Pressure Measurement characteristic value.
*/
typedef struct
{
    uint8_t bp_meas_flag;
    T_BLOOD_PRESSUREE_MEAS_COMPOUND_VALUE bp_meas_compound_value;
    T_BLS_TIME_STAMP time_stamp;
    SFLOAT bp_meas_pulse_rate;
    uint8_t bp_meas_user_id;
    T_BLOOD_PRESSUREE_MEAS_STATUS bp_meas_status;
} T_BLOOD_PRESSURE_MEASURMENT;

/** @} End of BLS_Common_Exported_Types */

/** @} End of BLS_DEFINE */

/** @} End of BLS */

#endif
