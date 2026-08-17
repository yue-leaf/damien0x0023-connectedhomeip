/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "secure_app_section.h"
#include "patch_header_check.h"
#include "rtl876x.h"
#include "rom_uuid.h"
#include "trace.h"
#include "version.h"
#include "mem_config.h"
#include "secure_rom_cfg.h"
#include "flash_nor_device.h"
#include "tzm_config.h"
#include "rxi300_idau.h"

bool system_init(void);

#if (FEATURE_RAM_CODE == 1)
extern char Image$$RAM_TEXT$$RO$$Base[];
extern char Load$$RAM_TEXT$$RO$$Base[];
extern char Load$$RAM_TEXT$$RO$$Length[];
#else
extern char Image$$FLASH_START_TEXT$$Base[];
#endif
#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
extern char Load$$LR$$LOAD_FLASH$$Base[];
#elif defined (__GNUC__)
extern uint32_t *__app_flash_load_ad__;
#endif


APP_FLASH_HEADER const T_IMG_HEADER_FORMAT img_header =
{
    .auth =
    {
        .image_mac = {[0 ... 15] = 0xFF},
    },
    .ctrl_header =
    {
        .ic_type = IMG_IC_TYPE,
        .secure_version = 0,
#if (FEATURE_RAM_CODE == 1)
        .ctrl_flag.xip = 0,
#if (FEATURE_ENCRYPTION == 1)
        .ctrl_flag.enc = 1,
#else
        .ctrl_flag.enc = 0,
#endif
        .ctrl_flag.load_when_boot = 1,
        .ctrl_flag.enc_load = 0,
        .ctrl_flag.enc_key_select = ENC_KEY_OCEK,
#else
        .ctrl_flag.xip = 1,
        .ctrl_flag.enc = 0,
        .ctrl_flag.load_when_boot = 0,
        .ctrl_flag.enc_load = 0,
        .ctrl_flag.enc_key_select = 0,
#endif
        .ctrl_flag.not_obsolete = 1,
        .image_id = IMG_SECUREMCUAPP,
        .payload_len = 0x100,    //Will modify by build tool later
    },
    .uuid = DEFINE_symboltable_uuid,

#if (FEATURE_RAM_CODE == 1)
    /* to be modified based on different user scenario */
    .load_src = (uint32_t)Load$$RAM_TEXT$$RO$$Base,
    .load_dst = (uint32_t)Image$$RAM_TEXT$$RO$$Base,
    .load_len = (uint32_t)Load$$RAM_TEXT$$RO$$Length,
    .exe_base = (uint32_t)Image$$RAM_TEXT$$RO$$Base,
#else
    /* XIP test */
#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
    .load_src = 0,
    .load_dst = 0,
    .load_len = 0,  //0 indicates all XIP
    .exe_base = (uint32_t)Image$$FLASH_START_TEXT$$Base,
#elif defined (__GNUC__)
    .load_src = (uint32_t) system_init,
    .load_dst = (uint32_t) system_init,
    .load_len = 0,  //0 indicates all XIP
    .exe_base = (uint32_t) system_init,
#endif
#endif
#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
    .image_base = (uint32_t) &Load$$LR$$LOAD_FLASH$$Base,
#elif defined (__GNUC__)
    .image_base = (uint32_t) &__app_flash_load_ad__,
#endif
    .git_ver =
    {
        .ver_info.sub_version._version_major = VERSION_MAJOR,
        .ver_info.sub_version._version_minor = VERSION_MINOR,
        .ver_info.sub_version._version_revision = VERSION_REVISION,
        .ver_info.sub_version._version_reserve = VERSION_BUILDNUM % 32, //only 5 bit
        ._version_commitid = VERSION_GCID,
        ._customer_name = {CN_1, CN_2, CN_3, CN_4, CN_5, CN_6, CN_7, CN_8},
    },
};

const uint8_t enc_dummy_align[16] APP_ENCRYPION_DUMMY_ALIGN;


#if (FEATURE_RAM_CODE == 0)
void ram_init(void) APP_FLASH_TEXT_SECTION;
#endif
void ram_init(void)
{
#if (FEATURE_RAM_CODE == 0)
#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
    extern unsigned int Image$$RAM_TEXT$$RO$$Base;
    extern unsigned int Load$$RAM_TEXT$$RO$$Base;
    extern unsigned int Image$$RAM_TEXT$$RO$$Length;

    unsigned int *ro_image_base = (unsigned int *)& Image$$RAM_TEXT$$RO$$Base;
    unsigned int *ro_load_base = (unsigned int *)& Load$$RAM_TEXT$$RO$$Base;
    unsigned int ro_image_length = (unsigned int)& Image$$RAM_TEXT$$RO$$Length;
#elif defined ( __GNUC__ )
    extern unsigned int *__enter_iram_s_ro_exe_ad__;
    extern unsigned int *__enter_iram_s_ro_load_ad__;
    extern unsigned int *__enter_iram_s_ro_length__;

    unsigned int *ro_image_base = (unsigned int *)& __enter_iram_s_ro_exe_ad__;
    unsigned int *ro_load_base = (unsigned int *)& __enter_iram_s_ro_load_ad__;
    unsigned int ro_image_length = (unsigned int)& __enter_iram_s_ro_length__;
#endif

    // cppcheck-suppress nullPointer; objectIndex
    memcpy(ro_image_base, ro_load_base, ro_image_length);

    DBG_DIRECT("Secure APP copy ro: src 0x%x, dest 0x%x, len %d",
               (uint32_t)ro_load_base, (uint32_t)ro_image_base, ro_image_length);

#endif

#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
    extern unsigned int Image$$RAM_TEXT$$RW$$Base;
    extern unsigned int Load$$RAM_TEXT$$RW$$Base;
    extern unsigned int Image$$RAM_TEXT$$RW$$Length;

    unsigned int *data_image_base = (unsigned int *)& Image$$RAM_TEXT$$RW$$Base;
    unsigned int *data_load_base = (unsigned int *)& Load$$RAM_TEXT$$RW$$Base;
    unsigned int data_image_length = (unsigned int)& Image$$RAM_TEXT$$RW$$Length;
#elif defined ( __GNUC__ )
    extern unsigned int *__enter_iram_s_rw_load_ad__;
    extern unsigned int *__enter_iram_s_rw_length__;
    extern unsigned int *__enter_iram_s_rw_start__;
    unsigned int *data_image_base = (unsigned int *)&__enter_iram_s_rw_start__;
    unsigned int *data_load_base = (unsigned int *)& __enter_iram_s_rw_load_ad__;
    unsigned int data_image_length = (unsigned int)& __enter_iram_s_rw_length__;
#endif

    // cppcheck-suppress nullPointer; objectIndex
    memcpy(data_image_base, data_load_base, data_image_length);

    DBG_DIRECT("Secure APP copy rw: src 0x%x, dest 0x%x, len %d",
               (uint32_t)data_load_base, (uint32_t)data_image_base, data_image_length);

#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
    extern unsigned int Image$$RAM_TEXT$$ZI$$Base;
    extern unsigned int Image$$RAM_TEXT$$ZI$$Length;

    unsigned int *bss_image_base = (unsigned int *)& Image$$RAM_TEXT$$ZI$$Base;
    unsigned int bss_image_length = (unsigned int)& Image$$RAM_TEXT$$ZI$$Length;
#elif defined ( __GNUC__ )
    extern unsigned int *__enter_iram_s_zi_start__;
    extern unsigned int *__enter_iram_s_zi_length__;

    unsigned int *bss_image_base = (unsigned int *)& __enter_iram_s_zi_start__;
    unsigned int bss_image_length = (unsigned int)& __enter_iram_s_zi_length__;
#endif

    // cppcheck-suppress nullPointer; objectIndex
    memset(bss_image_base, 0, bss_image_length);

    DBG_DIRECT("Secure APP clear zi: addr 0x%x, len %d", (uint32_t)bss_image_base, bss_image_length);

}

__WEAK
void sau_region_init(void)
{
    boot_cfg.sau_region[1].RBAR = SAU_REGION1_BASE_ADDR;
    boot_cfg.sau_region[1].RLAR = SAU_REGION1_END_ADDR;
    boot_cfg.sau_region[1].NSC = 0;

    boot_cfg.sau_region[2].RBAR = SAU_REGION2_BASE_ADDR;
    boot_cfg.sau_region[2].RLAR = SAU_REGION2_END_ADDR;
    boot_cfg.sau_region[2].NSC = 0;

    boot_cfg.sau_region[3].RBAR = SAU_REGION3_BASE_ADDR;
    boot_cfg.sau_region[3].RLAR = SAU_REGION3_END_ADDR;
    boot_cfg.sau_region[3].NSC = 0;

    boot_cfg.sau_region[4].RBAR = SAU_REGION4_BASE_ADDR;
    boot_cfg.sau_region[4].RLAR = SAU_REGION4_END_ADDR;
    boot_cfg.sau_region[4].NSC = 0;

    boot_cfg.sau_region[5].RBAR = SAU_REGION5_BASE_ADDR;
    boot_cfg.sau_region[5].RLAR = SAU_REGION5_END_ADDR;
    boot_cfg.sau_region[5].NSC = 0;

    boot_cfg.sau_region[6].RBAR = SAU_REGION6_BASE_ADDR;
    boot_cfg.sau_region[6].RLAR = SAU_REGION6_END_ADDR;
    boot_cfg.sau_region[6].NSC = 0;
}

/* must be ram code because flash is not ready at this point */
APP_RAM_DATA_SECTION
void idau_region_init(void)
{
    RXI300->u_140.IDAU_CTRL = 0;

    /* Region 0: hw fixed at 0x4000_0000 ~ 0x4FFF_FFFF, non-secure peripherals */

    /* Region 1: non-secure ROM code, non-secure RAM code, non-secure RAM data */
    RXI300->u_108.IDAU_BA1 = IDAU_REGION1_BASE_ADDR;
    RXI300->u_10C.IDAU_LA1 = IDAU_REGION1_END_ADDR;
    RXI300->u_140.BITS_140.region1_enable = 1;

    /* Region 2: non-secure tcm ram */
    RXI300->u_110.IDAU_BA2 = IDAU_REGION2_BASE_ADDR;
    RXI300->u_114.IDAU_LA2 = IDAU_REGION2_END_ADDR;
    RXI300->u_140.BITS_140.region2_enable = 1;

    /* Region 3: non-secure data sram, buffer ram and partial non-secure flash data*/
    RXI300->u_118.IDAU_BA3 = IDAU_REGION3_BASE_ADDR;
    RXI300->u_11C.IDAU_LA3 = IDAU_REGION3_END_ADDR;
    RXI300->u_140.BITS_140.region3_enable = 1;

    /* Region 4: non-secure flash code */
    RXI300->u_120.IDAU_BA4 = IDAU_REGION4_BASE_ADDR;
    RXI300->u_124.IDAU_LA4 = IDAU_REGION4_END_ADDR;
    RXI300->u_140.BITS_140.region4_enable = 1;

    /* Region 5: non-secure flash data */
    RXI300->u_128.IDAU_BA5 = IDAU_REGION5_BASE_ADDR;
    RXI300->u_12C.IDAU_LA5 = IDAU_REGION5_END_ADDR;
    RXI300->u_140.BITS_140.region5_enable = 1;
}

void hal_setup_trustzone(void)
{
    DBG_DIRECT("TZ enable %d", boot_cfg.common.trustzone_enable);
    if (boot_cfg.common.trustzone_enable)
    {
        sau_region_init();

        idau_region_init();

        TrustZone_init();
        // If Cache is enabled before SAU init, it should clean D-cache after SAU init
        if (boot_cfg.common.dcache_enable)
        {
            SCB_CleanInvalidateDCache();
        }
    }
}

void update_ram_layout(void)
{
    boot_cfg.secure_heap_addr = S_HEAP_ADDR;
    boot_cfg.secure_heap_size = S_HEAP_SIZE;
}

#if (FEATURE_RAM_CODE == 1)
bool system_init(void) APP_RAM_START_SECTION;
#else
bool system_init(void) APP_FLASH_START_SECTION;
#endif
bool system_init(void)
{
    DBG_DIRECT(">>> Secure APP Entry");

    DBG_DIRECT("Secure APP version: %s, GCID: 0x%x, active bank%d",
               VERSION_BUILD_STR, VERSION_GCID, secure_app_num);

    update_ram_layout();

    ram_init();

    /*patch for dlps restore*/
    RXI300_IDAU_setup = idau_region_init;

    hal_setup_trustzone();

    extern void secure_app_main(void);
    secure_app_main();

    return true;
}

extern void *pvPortMalloc(size_t xWantedSize);

extern void vPortFree(void *pv);

#define FreeRTOS_portBYTE_ALIGNMENT    8

typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK *pxNextFreeBlock;   /*<< The next free block in the list. */
    size_t xBlockSize : 28;                 /*<< The size of the free block. */
    size_t xRamType : 3;
    size_t xAllocateBit : 1;
} BlockLink_t;

static void *freertos_realloc(void *ptr, size_t size)
{
    void *new_ptr;
    BlockLink_t *pxLink;
    size_t old_size, new_size = 0;

    if (ptr == NULL)
    {
        return pvPortMalloc(size);
    }

    if (size == 0)
    {
        vPortFree(ptr);
        ptr = NULL;
        return NULL;
    }

    pxLink = (BlockLink_t *)((uint8_t *)ptr - sizeof(BlockLink_t));
    old_size = pxLink->xBlockSize;

    if ((old_size - sizeof(BlockLink_t)) ==
        (size + (FreeRTOS_portBYTE_ALIGNMENT - (size & (FreeRTOS_portBYTE_ALIGNMENT - 1)))))
    {
        return ptr;
    }

    __disable_irq();
    vPortFree(ptr);
    new_ptr = pvPortMalloc(size);
    if (ptr != new_ptr)
    {
        pxLink = (BlockLink_t *)((uint8_t *)new_ptr - sizeof(BlockLink_t));
        new_size = pxLink->xBlockSize;
        memcpy(new_ptr, ptr, (old_size < new_size) ? (old_size - sizeof(BlockLink_t)) : (new_size - sizeof(
                    BlockLink_t)));
    }
    __enable_irq();

    return new_ptr;
}

#if defined(__ARMCC_VERSION)

__WEAK void *malloc(size_t size)
{
    return pvPortMalloc(size);
}

__WEAK void free(void *ptr)
{
    vPortFree(ptr);
}

__WEAK void *calloc(size_t n, size_t s)
{
    size_t total = n * s;
    void *ptr = pvPortMalloc(total);
    if (ptr)
    {
        memset(ptr, 0, total);
    }
    return ptr;
}

__WEAK void *realloc(void *p, size_t s)
{
    return freertos_realloc(p, s);
}

#else

/*
 * The linker will redirect 'malloc' to '__wrap_malloc'
 * and 'free' to '__wrap_free'.
 */

__attribute__((weak)) void *__wrap__malloc_r(struct _reent *r, size_t size)
{
    return pvPortMalloc(size);
}

__attribute__((weak)) void __wrap__free_r(struct _reent *r, void *ptr)
{
    vPortFree(ptr);
}

__attribute__((weak)) void *__wrap__calloc_r(struct _reent *r, size_t nmemb, size_t size)
{
    size_t total = nmemb * size;
    void *ptr = pvPortMalloc(total);
    if (ptr)
    {
        memset(ptr, 0, total);
    }
    return ptr;
}

__attribute__((weak)) void *__wrap__realloc_r(struct _reent *r, void *ptr, size_t size)
{
    return freertos_realloc(ptr, size);
}

#endif

