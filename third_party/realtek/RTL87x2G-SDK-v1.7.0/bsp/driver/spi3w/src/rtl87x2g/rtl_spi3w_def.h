/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef RTL_SPI3W_DEF_H
#define RTL_SPI3W_DEF_H

#include "utils/rtl_utils.h"
#include "address_map.h"

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/*============================================================================*
 *                         SPI3W Registers Memory Map
 *============================================================================*/
typedef struct
{
    __IO uint32_t SPI3W_RSVD0[12]; ;                     /*!< 0x00 -- 0x2C for Q-decode */
    __IO uint32_t SPI3W_CONFIG;                          /*!< 0X30 */
    __IO uint32_t SPI3W_WDATA;                           /*!< 0X34 */
    __IO uint32_t SPI3W_CLR;                             /*!< 0X38 */
    __I  uint32_t SPI3W_STS;                             /*!< 0X3C */
    __IO uint32_t SPI3W_RDATA0;                          /*!< 0X40 */
    __IO uint32_t SPI3W_RDATA4;                          /*!< 0X44 */
    __IO uint32_t SPI3W_RDATA8;                          /*!< 0X48 */
    __IO uint32_t SPI3W_RDATA12;                         /*!< 0X4C */
} SPI3W_TypeDef;

/*============================================================================*
 *                         SPI3W Declaration
 *============================================================================*/
/** \defgroup SPI3W    SPI3W
  * \brief
  * \{
  */

/** \defgroup SPI3W_Exported_Constants SPI3W Exported Constants
  * \brief
  * \{
  */

/** \defgroup SPI3W_Declaration SPI3W Declaration
  * \{
  * \ingroup  SPI3W_Exported_Constants
  */

#define SPI3W           ((SPI3W_TypeDef *) SPI3W_REG_BASE)  //!< SPI3W base address.

/** End of SPI3W_Declaration
  * \}
  */

/** End of SPI3W_Exported_Constants
  * \}
  */

/** End of SPI3W
  * \}
  */


/*============================================================================*
 *                         SPI3W Private Types
 *============================================================================*/
typedef struct
{
    uint32_t spi3w_reg[2];
} SPI3WStoreReg_Typedef;

/*============================================================================*
 *                         SPI3W Registers and Field Descriptions
 *============================================================================*/
/* 0x30
   0       R/W    spi3w_int_en            1'h0
   1       R/W    spi3w_int_mask          1'h0
   2       R      reserved14              1'h0
   7:3     R/W    spi3w_dly_cycle         5'h7
   15:8    R/W    spi3w_div_num           8'h13
   19:16   R/W    spi3w_burst_read_num    4'h7
   20      R/W    spi3w_burst_read_en     1'h0
   24:21   R/W    spi3w_resync_timer      4'h0
   27:25   R      reserved8               3'h0
   28      R/W    spi3w_oe_dly_en         1'h0
   29      R/W    spi3w_end_ext_en        1'h0
   30      R/W    spi3w_cs_en             1'h1
   31      R/W    spi3w_en                1'h0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        uint32_t spi3w_int_en: 1;
        uint32_t spi3w_int_mask: 1;
        const uint32_t reserved_1: 1;
        uint32_t spi3w_dly_cycle: 5;
        uint32_t spi3w_div_num: 8;
        uint32_t spi3w_burst_read_num: 4;
        uint32_t spi3w_burst_read_en: 1;
        uint32_t spi3w_resync_timer: 4;
        const uint32_t reserved_0: 3;
        uint32_t spi3w_oe_dly_en: 1;
        uint32_t spi3w_end_ext_en: 1;
        uint32_t spi3w_cs_en: 1;
        uint32_t spi3w_en: 1;
    } b;
} SPI3W_CONFIG_TypeDef;



/* 0x34
   7:0     R/W    spi3w_data_data       8'h0
   14:8    R/W    spi3w_data_addr       7'h0
   15      R/W    spi3w_data_mode       1'h0
   31:16   R      reserved18            16'h0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        uint32_t spi3w_data_data: 8;
        uint32_t spi3w_data_addr: 7;
        uint32_t spi3w_data_mode: 1;
        const uint32_t reserved_0: 16;
    } b;
} SPI3W_WDATA_TypeDef;



/* 0x38
   0       W1C    spi3w_int_clr         1'h0
   1       W1C    spi3w_rd_num_clr      1'h0
   2       W1C    spi3w_rd_data_clr     1'h0
   29:3    R      reserved25            27'h0
   30      R/W    spi3w_resync_en       1'h0
   31      R      reserved23            1'h0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        uint32_t spi3w_int_clr: 1;
        uint32_t spi3w_rd_num_clr: 1;
        uint32_t spi3w_rd_data_clr: 1;
        const uint32_t reserved_1: 27;
        uint32_t spi3w_resync_en: 1;
        const uint32_t reserved_0: 1;
    } b;
} SPI3W_CLR_TypeDef;



/* 0x3C
   3:0     R      spi3w_rdata_num           4'h0
   4       R      spi3w_busy                1'h0
   5       R      spi3w_int_status          1'h0
   6       R      spi3w_resync_busy         1'h0
   31:7    R      reserved30                25'h0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t spi3w_rdata_num: 4;
        const uint32_t spi3w_busy: 1;
        const uint32_t spi3w_int_status: 1;
        const uint32_t spi3w_resync_busy: 1;
        const uint32_t reserved_0: 25;
    } b;
} SPI3W_STS_TypeDef;



/* 0x40
   7:0     R      spi3w_rdata_0                 8'h0
   15:8    R      spi3w_rdata_1                 8'h0
   23:16   R      spi3w_rdata_2                 8'h0
   31:24   R      spi3w_rdata_3                 8'h0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t spi3w_rdata_0: 8;
        const uint32_t spi3w_rdata_1: 8;
        const uint32_t spi3w_rdata_2: 8;
        const uint32_t spi3w_rdata_3: 8;
    } b;
} SPI3W_RDATA0_TypeDef;



/* 0x44
   7:0     R      spi3w_rdata_4                 8'h0
   15:8    R      spi3w_rdata_5                 8'h0
   23:16   R      spi3w_rdata_6                 8'h0
   31:24   R      spi3w_rdata_7                 8'h0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t spi3w_rdata_4: 8;
        const uint32_t spi3w_rdata_5: 8;
        const uint32_t spi3w_rdata_6: 8;
        const uint32_t spi3w_rdata_7: 8;
    } b;
} SPI3W_RDATA4_TypeDef;



/* 0x48
   7:0     R      spi3w_rdata_8                 8'h0
   15:8    R      spi3w_rdata_9                 8'h0
   23:16   R      spi3w_rdata_10                8'h0
   31:24   R      spi3w_rdata_11                8'h0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t spi3w_rdata_8: 8;
        const uint32_t spi3w_rdata_9: 8;
        const uint32_t spi3w_rdata_10: 8;
        const uint32_t spi3w_rdata_11: 8;
    } b;
} SPI3W_RDATA8_TypeDef;



/* 0x4C
   7:0     R      spi3w_rdata_12                8'h0
   15:8    R      spi3w_rdata_13                8'h0
   23:16   R      spi3w_rdata_14                8'h0
   31:24   R      reserved51                    8'h0
*/
typedef union
{
    uint32_t d32;
    uint8_t d8[4];
    struct
    {
        const uint32_t spi3w_rdata_12: 8;
        const uint32_t spi3w_rdata_13: 8;
        const uint32_t spi3w_rdata_14: 8;
        const uint32_t reserved_0: 8;
    } b;
} SPI3W_RDATA12_TypeDef;


#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* RTL_SPI3W_DEF_H */
