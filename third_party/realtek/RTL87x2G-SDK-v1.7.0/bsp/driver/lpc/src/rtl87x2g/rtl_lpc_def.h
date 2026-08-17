/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef RTL_LPC_DEF_H
#define RTL_LPC_DEF_H

#include "utils/rtl_utils.h"
#include "address_map.h"
#include "platform_reg.h"
#include "aon_reg.h"

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/*============================================================================*
 *                          LPC Defines
 *============================================================================*/
/** \defgroup LPC         LPC
  * \brief
  * \{
  */

/** \defgroup LPC_Exported_Constants LPC Exported Constants
  * \brief
  * \{
  */

/**
 * \defgroup LPC_Defines LPC Defines
 * \{
 * \ingroup  LPC_Exported_Constants
 */
#define LPC_SUPPORT_POWER_OFF                          (1) //!< The definition supports power off sequence.
#define LPC_SUPPORT_POWER_ON                           (1) //!< The definition supports power on sequence.
#define LPC_SUPPORT_NOMAL_FLOW                         (0) //!< The function is not supported.
#define LPC_SUPPORT_TRIGER_MODE                        (0) //!< The function is not supported.
#define LPC_SUPPORT_INT_LPCOMP_AON                     (0) //!< The function is not supported.
#define LPC_SUPPORT_DEBOUNCE_MODE                      (1) //!< The definition supports debounce function. 

/** End of LPC_Defines
  * \}
  */

/** End of LPC_Exported_Constants
  * \}
  */

/** End of LPC
  * \}
  */

/*============================================================================*
 *                         LPC Registers Memory Map
 *============================================================================*/
typedef struct
{
    __IO uint32_t LPC_CR;        /*!< 0x00 */
    __IO uint32_t LPC_SR;        /*!< 0x04 */
} LPC_TypeDef;

/*============================================================================*
 *                         LPC Declaration
 *============================================================================*/
/** \defgroup LPC         LPC
  * \brief
  * \{
  */

/** \defgroup LPC_Exported_Constants LPC Exported Constants
  * \brief
  * \{
  */

/** \defgroup LPC_Declaration LPC Declaration
  * \{
  * \ingroup  LPC_Exported_Constants
  */

#define LPC0               ((LPC_TypeDef *) LPC_REG_BASE)   //!< LPC0 base address.

/** End of LPC_Declaration
  * \}
  */

/** \defgroup LPC_Threshold LPC threshold
  * \{
  * \ingroup  LPC_Exported_Constants
  */
typedef enum
{
    LPC_80_mV = 0x0000,
    LPC_160_mV = 0x0001,
    LPC_240_mV = 0x0002,
    LPC_320_mV = 0x0003,
    LPC_400_mV = 0x0004,
    LPC_480_mV = 0x0005,
    LPC_560_mV = 0x0006,
    LPC_640_mV = 0x0007,
    LPC_680_mV = 0x0008,
    LPC_720_mV = 0x0009,
    LPC_760_mV = 0x000a,
    LPC_800_mV = 0x000b,
    LPC_840_mV = 0x000c,
    LPC_880_mV = 0x000d,
    LPC_920_mV = 0x000e,
    LPC_960_mV = 0x000f,
    LPC_1000_mV = 0x0010,
    LPC_1040_mV = 0x0011,
    LPC_1080_mV = 0x0012,
    LPC_1120_mV = 0x0013,
    LPC_1160_mV = 0x0014,
    LPC_1200_mV = 0x0015,
    LPC_1240_mV = 0x0016,
    LPC_1280_mV = 0x0017,
    LPC_1320_mV = 0x0018,
    LPC_1360_mV = 0x0019,
    LPC_1400_mV = 0x001a,
    LPC_1440_mV = 0x001b,
    LPC_1480_mV = 0x001c,
    LPC_1520_mV = 0x001d,
    LPC_1560_mV = 0x001e,
    LPC_1600_mV = 0x001f,
    LPC_1640_mV = 0x0020,
    LPC_1680_mV = 0x0021,
    LPC_1720_mV = 0x0022,
    LPC_1760_mV = 0x0023,
    LPC_1800_mV = 0x0024,
    LPC_1840_mV = 0x0025,
    LPC_1880_mV = 0x0026,
    LPC_1920_mV = 0x0027,
    LPC_1960_mV = 0x0028,
    LPC_2000_mV = 0x0029,
    LPC_2040_mV = 0x002a,
    LPC_2080_mV = 0x002b,
    LPC_2120_mV = 0x002c,
    LPC_2160_mV = 0x002d,
    LPC_2200_mV = 0x002e,
    LPC_2240_mV = 0x002f,
    LPC_2280_mV = 0x0030,
    LPC_2320_mV = 0x0031,
    LPC_2360_mV = 0x0032,
    LPC_2400_mV = 0x0033,
    LPC_2440_mV = 0x0034,
    LPC_2480_mV = 0x0035,
    LPC_2520_mV = 0x0036,
    LPC_2560_mV = 0x0037,
    LPC_2640_mV = 0x0038,
    LPC_2720_mV = 0x0039,
    LPC_2800_mV = 0x003a,
    LPC_2880_mV = 0x003b,
    LPC_2960_mV = 0x003c,
    LPC_3040_mV = 0x003d,
    LPC_3120_mV = 0x003e,
    LPC_3200_mV = 0x003f,
} LPCThreshold_TypeDef;

/** End of LPC_Threshold
  * \}
  */

/**
 * \defgroup   LPC_Interrupts_Definition   LPC Interrupts Definition
 * \{
 * \ingroup     LPC_Exported_Constants
 */
#define LPC_INT_LPCOMP                 (BIT17)  //!< Voltage detection interrupt.
#define LPC_INT_LPCOMP_AON             (BIT18)  //!< Wakeup voltage detection interrupt.

#define IS_LPC_CONFIG_INT(INT) ((INT) == LPC_INT_LPCOMP ) //!< Check if the input parameter is valid.
#define IS_LPC_STATUS_INT(INT) ((INT) == LPC_INT_LPCOMP ) //!< Check if the input parameter is valid.
#define IS_LPC_CLEAR_INT(INT) ((INT) ==  LPC_INT_LPCOMP ) //!< Check if the input parameter is valid.

/** End of LPC_Interrupts_Definition
  * \}
  */

/** End of LPC_Exported_Constants
  * \}
  */

/** End of LPC
  * \}
  */
/*============================================================================*
 *                         LPC Registers and Field Descriptions
 *============================================================================*/
/* 0x00
    7:0     R/W    lpcomp_deb_cnt             8'b0
    10:8    R/W    lpcomp_deb_div             3'b0
    11      R/W    lpcomp_deb_en              1'b0
    12      R/W    lpcomp_hys_en              1'b0
    13      R/W    lpcomp_single_output_en    1'b0
    14      W1C    rsvd                       1'b0
    15      R/W    rsvd                       1'b0
    16      R/W    lpcomp_ie                  1'b0
    17      R/W    lpcomp_src_int_en          1'b0
    18      R/W    lpcomp_src_aon_int_en      1'b0
    19      R/W    lpcomp_deb_sel             1'b0
    30:20   R/W    lpc_cr0_dummy              11'b0
    31      R/W    lpc_cr1_dummy              1'b0
*/
// *INDENT-OFF*
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        uint32_t lpcomp0_deb_cnt: 8;
        uint32_t lpcomp0_deb_div: 3;
        uint32_t lpcomp0_deb_en: 1;
        uint32_t reserved_2: 1;
        uint32_t lpcomp0_single_output_en: 1;
        uint32_t reserved_1: 1;
        uint32_t reserved_0: 1;
        uint32_t lpcomp0_ie: 1;
        uint32_t lpcomp0_src_int_en: 1;
        uint32_t lpcomp0_src_aon_int_en: 1;
        uint32_t lpcomp0_deb_sel: 1;
        uint32_t lpc0_cr0_dummy: 11;
        uint32_t lpc0_cr1_dummy: 1;
    } b;
} LPC_CR0_TypeDef;

/* 0x04
    0       R      lpcomp0_flag                1'b0
    1       R      lpcomp0_out_aon_flag        1'b0
    31:2    R/W    lpc0_sr_Dummy               30'b0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t lpcomp_flag: 1;
        const uint32_t lpcomp_out_aon_flag: 1;
        uint32_t lpc_sr_Dummy: 30;
    } b;
} LPC_SR_TypeDef;

// *INDENT-ON*
#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* RTL_LPC_DEF_H */
