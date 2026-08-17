/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _TZM_CONFIG_H_
#define _TZM_CONFIG_H_



/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif
#include <stdbool.h>
#include "flash_map.h"

/*default flash map: single bank*/
#define FAKE_ROM_VENEER_TABLE_END_ADDR    (0x00016280 - 1)
#define SECURE_ROM_SIZE                   (116 * 1024)


#define ITCM0_ADDR            0x00000000
#define ITCM0_SIZE            0x00080000   // 512K

#define CODE_SIZE_S             (SECURE_ROM_SIZE)  // contains nsc region
#define CODE_SIZE_NS            (NONSECURE_ROM_SIZE)
#define CODE_START_S            ITCM0_ADDR
#define CODE_START_NS           (CODE_START_S + CODE_SIZE_S)

#define APP_RAM_START_NS        TCM_START_ADDR
#define APP_RAM_SIZE_NS         NS_RAM_APP_RESERVED_SIZE
#define APP_RAM_START_S         (TCM_START_ADDR + NS_RAM_APP_RESERVED_SIZE)
#define APP_RAM_SIZE_S          S_RAM_APP_RESERVED_SIZE

#define RESERVED_RAM_START_NS   (TCM_START_ADDR + TCM_TOTAL_SIZE - S_RAM_SYSTEM_RESERVED_SIZE - NS_RAM_SYSTEM_RESERVED_SIZE)
#define RESERVED_RAM_SIZE_NS    NS_RAM_SYSTEM_RESERVED_SIZE
#define RESERVED_RAM_START_S    (TCM_START_ADDR + TCM_TOTAL_SIZE - S_RAM_SYSTEM_RESERVED_SIZE)
#define RESERVED_RAM_SIZE_S     S_RAM_SYSTEM_RESERVED_SIZE

#define DATA_START_NS           DATA_SRAM_ADDR

#define AON_SECURE_REGION_START 0x40000000
#define AON_SECURE_REGION_END   0x400004a0   //non-secure PF_RTC aligned down 32 bytes

#define IDAU_PERIPH_START_NS    AON_SECURE_REGION_START     // since IDAU region aligns 4K, NS starts from 0x40000000
#define SAU_PERIPH_START_NS     AON_SECURE_REGION_END      // first 6K of AON is secure
#define PERIPH_END_NS           0x50000000

#define DATA0_FLASH_START_NS    FLASH_START_ADDR
#define CODE0_FLASH_START_S     BANK0_BOOT_PATCH_ADDR
#define CODE0_FLASH_START_NS    OTA_BANK0_ADDR
#define CODE1_FLASH_START_S     BANK0_SECURE_APP_ADDR
#define DATA1_FLASH_START_NS    OTA_TMP_ADDR

/*============================================================================*
  *                               SAU Region Map
  * reference wiki: https://wiki.realtek.com/pages/viewpage.action?pageId=223804232
  *============================================================================*/

/*!
* @brief TrustZone initialization(GRACE: Just for reference!! Need modify later)
 *
 * SAU Configuration
 * This function configures 5 regions:
 * depends on secure rom code size  - non-secure callable ROM code for veneer table
 * 0x0003_8000 - 0x0003_FFFF(32KB)  - non-secure ROM code              --
 * 0x0040_0000 - 0x0043_FFFF(256KB) - non-secure RAM code                |-- same SAU region
 * 0x0044_0000 - 0x0045_FFFF(128KB) - non-secure RAM for data storage  --
 * 0x0050_0000 - 0x0400_2FFF        - external ram and non-secure flash code
 * 0x0403_8000 - 0x3FFF_FFFF        - non-secure Flash code and other memory
 * 0x4000_1800 - 0x4FFF_FFFF        - non-secure address space for peripherals
 */

extern unsigned int Image$$NSC_VENEER_TABLE$$Base;
extern unsigned int Image$$NSC_VENEER_TABLE$$Length;

// uint32_t nsc_start_addr = (unsigned int)& Image$$VENEER_TABLE$$Base;
// uint32_t nsc_region_len = (unsigned int)& Image$$VENEER_TABLE$$Length;

/** Region 0: non-secure callable*/
#define SAU_REGION0_BASE_ADDR           ((unsigned int)& Image$$VENEER_TABLE$$Base)
#define SAU_REGION0_END_ADDR            (((unsigned int)& Image$$VENEER_TABLE$$Base) + ((unsigned int)& Image$$VENEER_TABLE$$Length) - 1)
/** Region 1: non-secure ROM code, non-secure RAM code, non-secure RAM data */
#define SAU_REGION1_BASE_ADDR           CODE_START_NS
#define SAU_REGION1_END_ADDR            (APP_RAM_START_S - 1)
/** Region 2: non-secure DTCM*/
#define SAU_REGION2_BASE_ADDR           RESERVED_RAM_START_NS
#define SAU_REGION2_END_ADDR            (RESERVED_RAM_START_S - 1)
/** Region 3: non-secure DTCM, data sram, buffer ram and partial non-secure flash code*/
#define SAU_REGION3_BASE_ADDR           DATA_START_NS
#define SAU_REGION3_END_ADDR            (CODE0_FLASH_START_S - 1)
/** Region 4: non-secure flash code and flash data*/
#define SAU_REGION4_BASE_ADDR           CODE0_FLASH_START_NS
#define SAU_REGION4_END_ADDR            (CODE1_FLASH_START_S - 1)
/** Region 5: non-secure flash code*/
#define SAU_REGION5_BASE_ADDR           DATA1_FLASH_START_NS
#define SAU_REGION5_END_ADDR            (AON_SECURE_REGION_START - 1)   // 0x40000000 - 1
/** Region 6: non-secure peripherals*/
#define SAU_REGION6_BASE_ADDR           SAU_PERIPH_START_NS   //0x400004a0
#define SAU_REGION6_END_ADDR            (PERIPH_END_NS - 1)
/** Region 7: non-secure callable*/
#if defined(__ARMCC_VERSION)
#define SAU_REGION7_BASE_ADDR           ((unsigned int)& Image$$NSC_VENEER_TABLE$$Base)
#define SAU_REGION7_END_ADDR            (((unsigned int)& Image$$NSC_VENEER_TABLE$$Base) + ((unsigned int)& Image$$NSC_VENEER_TABLE$$Length) - 1)
#elif defined(__GNUC__)
extern uint32_t *_start_sg;
extern uint32_t *_end_sg;
#define SAU_REGION7_BASE_ADDR           ((uint32_t) &_start_sg)
#define SAU_REGION7_END_ADDR            ((uint32_t) &_end_sg - 1)
#else
#endif

/*!
 * IDAU Configuration
 * This function configures 3 regions as non-secure region:
 * 0x0003_8000 - 0x0003_FFFF(32KB)  - non-secure ROM code              --
 * 0x0040_0000 - 0x0043_FFFF(256KB) - non-secure RAM code                |-- same IDAU region
 * 0x0044_0000 - 0x0045_FFFF(128KB) - non-secure RAM for data storage  --
 * 0x0050_0000 - 0x0400_2FFF        - external ram and non-secure flash code
 * 0x0403_8000 - 0x4FFF_FFFF        - non-secure Flash code, other memory and non-secure address space for peripherals
 *
 * non secure callable is secure in IDAU
 */

/** Region 0: hw fixed at 0x4000_0000 ~ 0x4FFF_FFFF, non-secure peripherals */

/** Region 1: non-secure ROM code, non-secure RAM code, non-secure RAM data */
#define  IDAU_REGION1_BASE_ADDR          SAU_REGION1_BASE_ADDR
#define  IDAU_REGION1_END_ADDR           SAU_REGION1_END_ADDR
/** Region 2: non-secure TCM*/
#define  IDAU_REGION2_BASE_ADDR          SAU_REGION2_BASE_ADDR
#define  IDAU_REGION2_END_ADDR           SAU_REGION2_END_ADDR
/** Region 3: non-secure data sram, buffer ram and partial non-secure flash code*/
#define  IDAU_REGION3_BASE_ADDR          SAU_REGION3_BASE_ADDR
#define  IDAU_REGION3_END_ADDR           SAU_REGION3_END_ADDR
/** Region 4: non-secure flash code and flash data */
#define  IDAU_REGION4_BASE_ADDR          SAU_REGION4_BASE_ADDR
#define  IDAU_REGION4_END_ADDR           SAU_REGION4_END_ADDR
/** Region 5: non-secure flash code */
#define  IDAU_REGION5_BASE_ADDR          SAU_REGION5_BASE_ADDR
#define  IDAU_REGION5_END_ADDR           SAU_REGION5_END_ADDR

/* Initialize TrustZone */
extern bool (*TrustZone_init)(void);
extern void (*RXI300_IDAU_setup)(void);
extern uint32_t secure_app_num;
extern void init_default_sau_cfg_settings(void);

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _TZM_CONFIG_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
