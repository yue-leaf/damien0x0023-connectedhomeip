#include "../../../../../include/rtl87x2g/config/secure/mem_config.h"
#include "../../../../../include/rtl87x2g/config/secure/flash_map.h"

/* Entry Point */
ENTRY(Reset_Handler)

#if (BUILD_BANK == 0)
#define APP_ADDR                    BANK0_SECURE_APP_ADDR
#define APP_SIZE                    BANK0_SECURE_APP_SIZE
#else
#define APP_ADDR                    BANK1_SECURE_APP_ADDR
#define APP_SIZE                    BANK1_SECURE_APP_SIZE
#endif

MEMORY
{
S_APP (r)   : ORIGIN = APP_ADDR, LENGTH = APP_SIZE
S_RAM_GLOBAL (rwx) : ORIGIN = S_RAM_APP_ADDR, LENGTH = S_RAM_APP_SIZE 
S_HEAP(rwx): ORIGIN = S_HEAP_ADDR, LENGTH = S_HEAP_SIZE
APP_TRACE (r): ORIGIN = 0x18180000, LENGTH = 0x80000
}

/* _estack = 0x13E000; */
/* Define output sections */
SECTIONS
{
    . = APP_ADDR;
    app_s.bin  APP_ADDR :
    {
        KEEP(* (.app.flash.header))
    }AT > S_APP

    __app_flash_load_ad__ = APP_ADDR;

    FLASH_HEADER_EXT :
    {
       KEEP(* (.app.flash.header_ext))
    }AT > S_APP

#if (FEATURE_FLASH_SEC == 1)
    FLASH_GAP_REGION :
    {
        * (.app.flash.gap.text)
        . = ALIGN(0x1000);
        __flash_gap_region_load_ad__ = LOADADDR(FLASH_GAP_REGION);
        __flash_gap_region_exe_ad__ = ADDR(FLASH_GAP_REGION);
    }AT > S_APP
#endif

    /* Secure Gateway Veneers */
    .gnu.sgstubs : ALIGN(32)
    {
        . = ALIGN(32);
        _start_sg = .;
        KEEP(*(.gnu.sgstubs*))
    } AT > S_APP

    _end_sg = ADDR(.gnu.sgstubs) + SIZEOF(.gnu.sgstubs);

    FLASH_TEXT :
    {
        * (.app.flash.start)
#if (FEATURE_RAM_CODE == 0)
        *(.text*)
        . = ALIGN(4);
#endif
        *(.rodata*)
        . = ALIGN(4);
        * (.isr.text)
        * (.app.flash.text)
        * (.app.flash.rodata)
        *(.ARM.exidx*)
    }AT > S_APP

    ER_IRAM_S :
    {
       . = ALIGN(0x20);
        __enter_iram_s_ro_start__ = .;
#if (FEATURE_RAM_CODE == 1)
        *(.text*)
        . = ALIGN(4);
#endif
        * (.app.ram.text)
        __enter_iram_s_ro_end__ = .;
        __enter_iram_s_rw_start__ = .;
        *(.data*)
        * (.app.ram.data)
        __enter_iram_s_rw_end__ = .;
    }> S_RAM_GLOBAL  AT > S_APP
    
    ER_IRAM_S_BSS (NOLOAD):
    {
        . = ALIGN(4);
         __enter_iram_s_zi_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __enter_iram_s_zi_end__ = .;
    }> S_RAM_GLOBAL AT > S_APP
    __enter_iram_s_ro_load_ad__ = LOADADDR(ER_IRAM_S);
    __enter_iram_s_ro_exe_ad__ = ADDR(ER_IRAM_S);
    __enter_iram_s_ro_length__ = __enter_iram_s_ro_end__ - __enter_iram_s_ro_start__;
    __enter_iram_s_rw_length__ = __enter_iram_s_rw_end__ - __enter_iram_s_rw_start__;
    __enter_iram_s_zi_length__ = __enter_iram_s_zi_end__ - __enter_iram_s_zi_start__;
    __enter_iram_s_rw_load_ad__ = LOADADDR(ER_IRAM_S) + __enter_iram_s_ro_length__;

    .heap (NOLOAD):
    {
        . = ALIGN(4);
        PROVIDE ( end = . );
        _sheap = .;
        . = . + S_HEAP_SIZE;
        . = ALIGN(4);
        _eheap = .;
    }> S_HEAP

    App.trace 0x18180000 :
    {
        *(.TRACE*)
    }>APP_TRACE
}

