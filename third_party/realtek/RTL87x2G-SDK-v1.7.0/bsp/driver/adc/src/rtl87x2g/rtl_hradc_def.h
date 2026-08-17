/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef RTL_HRADC_DEF_H
#define RTL_HRADC_DEF_H

#include "utils/rtl_utils.h"
#include "address_map.h"
#include "aon_reg.h"

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/*============================================================================*
 *                          HRADC Defines
 *============================================================================*/

/*============================================================================*
 *                          HRADC Registers Memory Map
 *============================================================================*/
typedef struct                  /*!< HRADC Structure */
{
    __IO uint32_t REG0X_SDADC;                    /*!< 0x00 */
    __IO uint32_t REG0X_SDADC_DIG_CONFIG;         /*!< 0x04 */
    __IO uint32_t REG2X_SDADC_DIG_CONFIG;         /*!< 0x08 */
    __IO uint32_t REG4X_SDADC_DIG_CONFIG;         /*!< 0x0C */
    __O  uint32_t REG_SDADC_OUTPUT_DATA_FIFO;     /*!< 0x10 */
    __O  uint32_t REG_CH_IDX0_SDADC_OUTPUT_DATA;  /*!< 0x14 */
    __O  uint32_t REG_CH_IDX1_SDADC_OUTPUT_DATA;  /*!< 0x18 */
    __O  uint32_t REG_CH_IDX2_SDADC_OUTPUT_DATA;  /*!< 0x1C */
    __O  uint32_t REG_CH_IDX3_SDADC_OUTPUT_DATA;  /*!< 0x20 */
    __O  uint32_t REG_CH_IDX4_SDADC_OUTPUT_DATA;  /*!< 0x24 */
    __O  uint32_t REG_CH_IDX5_SDADC_OUTPUT_DATA;  /*!< 0x28 */
    __O  uint32_t REG_CH_IDX6_SDADC_OUTPUT_DATA;  /*!< 0x2C */
    __O  uint32_t REG_CH_IDX7_SDADC_OUTPUT_DATA;  /*!< 0x30 */
    __IO uint32_t REG_SDADC_POWERON_CONFIG;       /*!< 0x34 */
    __IO uint32_t REG_SDADC_FIFO_INFO;            /*!< 0x38 */
} HRADC_TypeDef;

/*============================================================================*
 *                          HRADC Declaration
 *============================================================================*/
#define HRADC                   ((HRADC_TypeDef *) HRADC_REG_BASE)
#define HRADC_LDO_REG1DC0       *((volatile uint32_t *)0x40001DC0UL)

/*============================================================================*
 *                          HRADC Private Types
 *============================================================================*/
typedef enum
{
    HRADC_BG_READY_DLY_2MS = 0x4,
} HRADCBgReadyDly_TypeDef;

#define IS_HRADC_BG_READY_DLY_TYPE(TYPE) ((TYPE) <= 0x7)


typedef enum
{
    HRADC_SS_DONE_DLY_4MS = 0x8,
} HRADCSSDoneDly_TypeDef;

#define IS_HRADC_SS_DONE_DLY_TYPE(TYPE) ((TYPE) <= 0x15)

typedef enum
{
    HRADC_LDO_READY_DLY_8MS = 0x10,
} HRADCLdoReadyDly_TypeDef;

#define IS_HRADC_LDO_READY_DLY_TYPE(TYPE) ((TYPE) <= 0x31)

typedef enum
{
    HRADC_CLK_READY_DLY_62_5UM = 0x0,
} HRADCClkReadyDly_TypeDef;

#define IS_HRADC_CLK_READY_DLY_TYPE(TYPE) ((TYPE) <= 0x3)

typedef enum
{
    HRADC_DMY_LOAD_EXT_DLY_500UM = 0x1,
} HRADCDmyLoadExtDly_TypeDef;

#define IS_HRADC_DMY_LOAD_EXT_DLY_TYPE(TYPE) ((TYPE) <= 0x3)

typedef enum
{
    HRADC_READY_DLY_125UM = 0x1,
} HRADC_READY_DLY_TypeDef;

#define IS_HRADC_READY_DLY_TYPE(TYPE) ((TYPE) <= 0x3)


/*============================================================================*
 *                          HRADC Registers and Field Descriptions
 *============================================================================*/
/**
  * @brief Analog to digital converter. (HRADC)
  */
/* 0x000
    0       R/W    REG_PGA_VCMI_EN                     1'b0
    1       R/W    REG_PGA_BYPASS_EN                   1'b0
    2       R/W    REG_PGA_INSHORT_EN                  1'b0
    3       R/W    REG_PGA_RSTB                        1'b0
    4       R/W    REG_PGA_POW                         1'b0
    6:5     R/W    REG_CHOP_DIV<1:0>                   2'b00
    7       R/W    REG_CHOP_EN                         1'b0
    8       R/W    REG_DTSDM_RSTB                      1'b0
    9       R/W    REG_DTSDM_POW                       1'b0
    10      R/W    REG_AD_CK_POW                       1'b0
    11      R/W    REG_VREFP_EXT_SEL                   1'b0
    13:12   R/W    REG_PGA_IB_SEL<1:0>                 2'b10
    15:14   R/W    REG_DTSDM_IB_SEL<1:0>               2'b10
    24:16   R/W    REG_PGA_DUMMY<9:0>                  9'b110101010
    25      R/W    REG_ISO_SDADC                       1'b1
    28:26   R/W    REG_PGA_GSEL<2:0>                   3'b000
    30:29   R/W    REG_PGA_CH_NUM<1:0>                 2'b00
    31      R/W    REG_PGA_DIFF_MODE                   1'b1
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        uint32_t REG_PGA_VCMI_EN: 1;
        uint32_t REG_PGA_BYPASS_EN: 1;
        uint32_t REG_PGA_INSHORT_EN: 1;
        uint32_t REG_PGA_RSTB: 1;
        uint32_t REG_PGA_POW: 1;
        uint32_t REG_CHOP_DIV_1_0: 2;
        uint32_t REG_CHOP_EN: 1;
        uint32_t REG_DTSDM_RSTB: 1;
        uint32_t REG_DTSDM_POW: 1;
        uint32_t REG_AD_CK_POW: 1;
        uint32_t REG_VREFP_EXT_SEL: 1;
        uint32_t REG_PGA_IB_SEL_1_0: 2;
        uint32_t REG_DTSDM_IB_SEL_1_0: 2;
        uint32_t REG_PGA_DUMMY_9_0: 9;
        uint32_t REG_ISO_SDADC: 1;
        uint32_t REG_PGA_GSEL_2_0: 3;
        uint32_t REG_PGA_CH_NUM_1_0: 2;
        uint32_t REG_PGA_DIFF_MODE: 1;
    } b;
} HRADC_REG0X_SDADC_TypeDef;
/* 0x0004
    0       R/W    sdadc_fifo_thr_int_en               1'b1
    1       R/W    sdadc_fifo_overflow_int_en          1'b1
    2       R/W    sdadc_scan_oneround_done_int_en     1'b1
    3       R/W    sdadc_data_ready_int_en             1'b1
    4       R/W    sdadc_boot_ready_int_en             1'b1
    5       R/W    sdadc_fifo_thr_int_mask             1'b0
    6       R/W    sdadc_fifo_overflow_int_mask        1'b0
    7       R/W    sdadc_scan_oneround_done_int_mask   1'b0
    8       R/W    sdadc_data_ready_int_mask           1'b0
    9       R/W    sdadc_boot_ready_int_mask           1'b0
    10      R/W1C  sdadc_fifo_thr_int                  1'b0
    11      R/W1C  sdadc_fifo_overflow_int             1'b0
    12      R/W1C  sdadc_scan_oneround_done_int        1'b0
    13      R/W1C  sdadc_data_ready_int                1'b0
    14      R/W1C  sdadc_boot_ready_int                1'b0
    15      R/W    sdadc_fifo_clr                      1'b0
    16      R/W    sdadc_fifo_en                       1'b0
    21:17   R/W    sdadc_fifo_thr                      5'b0
    26:22   R/W    sdadc_data_discard                  5'b0
    31:27   R/W    sdadc_dma_req_burst_size            5'b0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        uint32_t sdadc_fifo_thr_int_en: 1;
        uint32_t sdadc_fifo_overflow_int_en: 1;
        uint32_t sdadc_scan_oneround_done_int_en: 1;
        uint32_t sdadc_data_ready_int_en: 1;
        uint32_t sdadc_boot_ready_int_en: 1;
        uint32_t sdadc_fifo_thr_int_mask: 1;
        uint32_t sdadc_fifo_overflow_int_mask: 1;
        uint32_t sdadc_scan_oneround_done_int_mask: 1;
        uint32_t sdadc_data_ready_int_mask: 1;
        uint32_t sdadc_boot_ready_int_mask: 1;
        uint32_t sdadc_fifo_thr_int: 1;
        uint32_t sdadc_fifo_overflow_int: 1;
        uint32_t sdadc_scan_oneround_done_int: 1;
        uint32_t sdadc_data_ready_int: 1;
        uint32_t sdadc_boot_ready_int: 1;
        uint32_t sdadc_fifo_clr: 1;
        uint32_t sdadc_fifo_en: 1;
        uint32_t sdadc_fifo_thr: 5;
        uint32_t sdadc_data_discard: 5;
        uint32_t sdadc_dma_req_burst_size: 5;
    } b;
} HRADC_REG0X_SDADC_DIG_CONFIG_TypeDef;
/* 0x0008
    3:0     R/W    sdadc_ch_switch_intv                4'b0
    7:4     R/W    sdadc_dr_config                     4'b0
    9:8     R/W    sdadc_channel_settling_time_config  2'd1
    11:10   R/W    sdadc_channel_reset_time_config     2'b0
    12      R/W    sdadc_filter_in_phase_swap          1'b0
    13      R/W    sdadc_clk_edge_swap                 1'b0
    14      R/W    sdadc_test_mode_en                  1'b0
    18:15   R/W    RSVD                                4'b0
    26:19   R/W    sdadc_ch_idx_en                     8'b0
    27      R/W    sdadc_adc_clk_sel                   1'b0
    28      R/W    sdadc_dig_clk_sel                   1'b0
    29      R/W    sdadc_scan_en                       1'b0
    30      R/W    sdadc_conv_mode                     1'b0
    31      R/W    sdadc_sample_start_en               1'b0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        uint32_t sdadc_ch_switch_intv: 4;
        uint32_t sdadc_dr_config: 4;
        uint32_t sdadc_channel_settling_time_config: 2;
        uint32_t sdadc_channel_reset_time_config: 2;
        uint32_t sdadc_filter_in_phase_swap: 1;
        uint32_t sdadc_clk_edge_swap: 1;
        uint32_t sdadc_test_mode_en: 1;
        uint32_t RESERVED_0: 4;
        uint32_t sdadc_ch_idx_en: 8;
        uint32_t sdadc_adc_clk_sel: 1;
        uint32_t sdadc_dig_clk_sel: 1;
        uint32_t sdadc_scan_en: 1;
        uint32_t sdadc_conv_mode: 1;
        uint32_t sdadc_sample_start_en: 1;
    } b;
} HRADC_REG2X_SDADC_DIG_CONFIG_TypeDef;
/* 0x000C
    1:0     R/W    sdadc_ch_num_idx0                   2'b0
    2       R/W    sdadc_mode_idx0                     1'b0
    4:3     R/W    sdadc_ch_num_idx1                   2'b0
    5       R/W    sdadc_mode_idx1                     1'b0
    7:6     R/W    sdadc_ch_num_idx2                   2'b0
    8       R/W    sdadc_mode_idx2                     1'b0
    10:9    R/W    sdadc_ch_num_idx3                   2'b0
    11      R/W    sdadc_mode_idx3                     1'b0
    13:12   R/W    sdadc_ch_num_idx4                   2'b0
    14      R/W    sdadc_mode_idx4                     1'b0
    16:15   R/W    sdadc_ch_num_idx5                   2'b0
    17      R/W    sdadc_mode_idx5                     1'b0
    19:18   R/W    sdadc_ch_num_idx6                   2'b0
    20      R/W    sdadc_mode_idx6                     1'b0
    22:21   R/W    sdadc_ch_num_idx7                   2'b0
    23      R/W    sdadc_mode_idx7                     1'b0
    24      R/W    sdadc_mux_sel_hw                    1'b1
    27:25   R/W    RSVD                                3'b0
    29:28   R/W    sdadc_filter_stage2_sel_order       2'd2
    31:30   R/W    sdadc_full_scale_compensation_gain  2'd1
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        uint32_t sdadc_ch_num_idx0: 2;
        uint32_t sdadc_mode_idx0: 1;
        uint32_t sdadc_ch_num_idx1: 2;
        uint32_t sdadc_mode_idx1: 1;
        uint32_t sdadc_ch_num_idx2: 2;
        uint32_t sdadc_mode_idx2: 1;
        uint32_t sdadc_ch_num_idx3: 2;
        uint32_t sdadc_mode_idx3: 1;
        uint32_t sdadc_ch_num_idx4: 2;
        uint32_t sdadc_mode_idx4: 1;
        uint32_t sdadc_ch_num_idx5: 2;
        uint32_t sdadc_mode_idx5: 1;
        uint32_t sdadc_ch_num_idx6: 2;
        uint32_t sdadc_mode_idx6: 1;
        uint32_t sdadc_ch_num_idx7: 2;
        uint32_t sdadc_mode_idx7: 1;
        uint32_t sdadc_mux_sel_hw: 1;
        uint32_t RESERVED_0: 3;
        uint32_t sdadc_filter_stage2_sel_order: 2;
        uint32_t sdadc_full_scale_compensation_gain: 2;
    } b;
} HRADC_REG4X_SDADC_DIG_CONFIG_TypeDef;
/* 0x0010
    31:0    R      sdadc_fifo_rdata                    32'b0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t sdadc_fifo_rdata: 32;
    } b;
} HRADC_REG_SDADC_OUTPUT_DATA_FIFO_TypeDef;
/* 0x0014
    31:0    R      sdadc_ch_idx0_output_data           32'b0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t sdadc_ch_idx0_output_data: 32;
    } b;
} HRADC_REG_CH_IDX0_SDADC_OUTPUT_DATA_TypeDef;
/* 0x0018
    31:0    R      sdadc_ch_idx1_output_data           32'b0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t sdadc_ch_idx1_output_data: 32;
    } b;
} HRADC_REG_CH_IDX1_SDADC_OUTPUT_DATA_TypeDef;
/* 0x001C
    31:0    R      sdadc_ch_idx2_output_data           32'b0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t sdadc_ch_idx2_output_data: 32;
    } b;
} HRADC_REG_CH_IDX2_SDADC_OUTPUT_DATA_TypeDef;
/* 0x0020
    31:0    R      sdadc_ch_idx3_output_data           32'b0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t sdadc_ch_idx3_output_data: 32;
    } b;
} HRADC_REG_CH_IDX3_SDADC_OUTPUT_DATA_TypeDef;
/* 0x0024
    31:0    R      sdadc_ch_idx4_output_data           32'b0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t sdadc_ch_idx4_output_data: 32;
    } b;
} HRADC_REG_CH_IDX4_SDADC_OUTPUT_DATA_TypeDef;
/* 0x0028
    31:0    R      sdadc_ch_idx5_output_data           32'b0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t sdadc_ch_idx5_output_data: 32;
    } b;
} HRADC_REG_CH_IDX5_SDADC_OUTPUT_DATA_TypeDef;
/* 0x002C
    31:0    R      sdadc_ch_idx6_output_data           32'b0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t sdadc_ch_idx6_output_data: 32;
    } b;
} HRADC_REG_CH_IDX6_SDADC_OUTPUT_DATA_TypeDef;
/* 0x0030
    31:0    R      sdadc_ch_idx7_output_data           32'b0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t sdadc_ch_idx7_output_data: 32;
    } b;
} HRADC_REG_CH_IDX7_SDADC_OUTPUT_DATA_TypeDef;
/* 0x0034
    2:0     R/W    sdadc_bg_ready_dly                  3'b100
    6:3     R/W    sdadc_ss_done_dly                   4'b1000
    11:7    R/W    sdadc_ldo_ready_dly                 5'b10000
    13:12   R/W    sdadc_adc_clk_ready_dly             2'b00
    15:14   R/W    sdadc_dmy_load_extend_dly           2'b01
    17:16   R/W    sdadc_adc_ready_dly                 2'b01
    18      R/W    sdadc_en                            1'b0
    19      R/W    sdadc_auto_boot_en                  1'b1
    20      R/W    sdadc_clk_en                        1'b0
    31:21   R/W    RSVD                                11'b0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        uint32_t sdadc_bg_ready_dly: 3;
        uint32_t sdadc_ss_done_dly: 4;
        uint32_t sdadc_ldo_ready_dly: 5;
        uint32_t sdadc_adc_clk_ready_dly: 2;
        uint32_t sdadc_dmy_load_extend_dly: 2;
        uint32_t sdadc_adc_ready_dly: 2;
        uint32_t sdadc_en: 1;
        uint32_t sdadc_auto_boot_en: 1;
        uint32_t sdadc_clk_en: 1;
        uint32_t RESERVED_0: 11;
    } b;
} HRADC_REG_SDADC_POWERON_CONFIG_TypeDef;
/* 0x0038
    5:0     R      sdadc_fifo_cnt                      6'b0
    31:6    R      RSVD                                26'b0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t sdadc_fifo_cnt: 6;
        const uint32_t RESERVED_0: 26;
    } b;
} HRADC_REG_SDADC_FIFO_INFO_TypeDef;


#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* RTL_HRADC_DEF_H */
