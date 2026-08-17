/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _OTP_H_
#define _OTP_H_

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    OS_TICK_10MS       = 0,
    OS_TICK_5MS        = 1,
    OS_TICK_2MS        = 2,
    OS_TICK_1MS        = 3,
} OS_TICK;

typedef union _HEAP_TYPE_MASK
{
    struct
    {
        uint8_t heap_data_on_mask : 1;
        uint8_t heap_buffer_on_mask : 1;
        uint8_t heap_ext_data_sram_mask : 1;
        uint8_t reserved : 6;
    };
    uint8_t heap_type_mask;
} HEAP_TYPE_MASK;

/* OTP start addr: 0x00138934 */
typedef struct otp_struct
{

    /***************** os_cfg *********************/
    /* start addr: 0x0013d79c, size:0x1f */
    uint8_t getStackHighWaterMark : 1;         /* 0 for release version, 1 for debug version */
    uint8_t checkForStackOverflow : 1;         /* 0 for release version, 1 for debug version */
    uint8_t dumpMemoryUsage : 1;               /* 0 for release version, 1 for debug version */
    uint8_t enableASSERT : 1;                   /* 0 for release version, 1 for debug version */
    uint8_t os_cfg_rsvd0 : 4;

    uint8_t  timerMaxNumber;                  /* default = 0x20 */
    uint8_t  timerQueueLength;                /* default = 0x20 */

    uint32_t appDataAddr;                     //0x0013d79f
    uint32_t appDataSize;                     //0x0013d7a3
    uint32_t heapDataONSize;                  //0x0013d7a7
    uint32_t heapBufferONSize;                //0x0013d7ab
    uint32_t globalExtDataSRAMSize;           //0x0013d7af
    HEAP_TYPE_MASK heap_mask;

    uint16_t idle_task_stack_size;             /* measured in bytes, default 256 * 4 bytes */
    uint16_t timer_task_stack_size;            /* measured in bytes, default 256 * 4 bytes */
    uint16_t lower_task_stack_size;            /* measured in bytes, default 768 * 4 bytes */
    /**************** end os_cfg *******************/

    uint8_t rsvd1[1353];

    /**************** platform config ****************/
    /* start addr: 0x0013dd04, size: 0x154 */
    uint32_t stack_en : 1;
    uint32_t cpu_sleep_en : 1;
    uint32_t systick_clk_src : 1;                  /* SYSTICK_EXTERNAL_CLOCK, SYSTICK_PROCESSOR_CLOCK */
    uint32_t printAllLogBeforeEnterLowpower : 1;   /* 0 for release version, 1 for debug version */
    uint32_t log_encode : 3;
    uint32_t log_ram_size : 3;
    uint32_t log_ram_type : 3;
    uint32_t platform_cfg_rsvd1 : 19;

    uint32_t os_tick_rate_HZ;
    uint32_t systick_ext_clk_freq; /* External systick timer clock frequency */

    uint32_t platform_cfg_rsvd2 : 30;
    uint32_t ftl_use_mapping_table : 1;
    uint32_t ftl_init_in_rom : 1;

    uint16_t ftl_app_start_addr;
    uint16_t flash_task_stack_depth : 2;
    uint16_t use_ftl : 1;
    uint16_t write_info_to_flash_when_hardfault: 1;/* write info to flash when hardfault happens */
    uint16_t platform_cfg_rsvd3 : 5;
    uint16_t ftl_logic_addr_map_bit_num : 3; /*4bit by step, default value is 3 means 12bit*/
    uint16_t platform_cfg_rsvd4 : 4;

    uint32_t reboot_record_address; /* start address of reboot record */

    uint32_t HardFault_Record_CFG; // default value: 0x7
    // [0]: 1, enable saving HardFault_Record
    // [1]: reserved//1, 1 bit slow mode
    // [2]: 1, dump HardFaultRecord in boot time
    // [3]: 1, Clean up HardFaultRecord after dumping HardFault_Record
    // [4]: 1, Clean up HardFaultRecord before saving HardFault_Record
    uint32_t HardFault_Record_BegAddr;      /* start address of hardfault record */
    uint32_t HardFault_Record_EndAddr;      /* end address of hardfault record */
    /**************** end platform config ****************/
}  __attribute__((packed)) T_OTP_CFG;
/* end otp, addr = 0x00138f80 */

#define OTP_STRUCT_BASE             0x0013d79c
#define OTP                         ((T_OTP_CFG *) OTP_STRUCT_BASE)

#ifdef __cplusplus
}
#endif

#endif  /* _OTP_H_ */
