cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(template LANGUAGES C VERSION 1.0.0)

configure_file(app_version.h.in ${CMAKE_BINARY_DIR}/template/include/app_version.h)
target_include_directories(app PRIVATE ${CMAKE_BINARY_DIR}/template/include src proj/rtl87x2g)

target_sources(app PRIVATE src/main_ns.c src/app_task.c)

if (CONFIG_CODE_DATA_RELOCATION AND CONFIG_SOC_SERIES_RTL87X2G)
zephyr_code_relocate(LIBRARY kernel LOCATION ITCM_TEXT)
zephyr_code_relocate(FILES ${ZEPHYR_BASE}/kernel/xip.c LOCATION FLASH_TEXT NOCOPY)
zephyr_code_relocate(FILES ${ZEPHYR_BASE}/drivers/timer/cortex_m_systick.c LOCATION ITCM_TEXT)
zephyr_code_relocate(FILES ${ZEPHYR_BASE}/arch/${ARCH}/core/aarch32/swap.c LOCATION ITCM_TEXT)
zephyr_code_relocate(FILES ${ZEPHYR_BASE}/arch/${ARCH}/core/aarch32/swap_helper.S LOCATION ITCM_TEXT)
zephyr_code_relocate(FILES ${ZEPHYR_BASE}/arch/${ARCH}/core/aarch32/isr_wrapper.S LOCATION ITCM_TEXT)
zephyr_code_relocate(FILES ${ZEPHYR_BASE}/arch/${ARCH}/core/aarch32/cortex_m/exc_exit.S LOCATION ITCM_TEXT)
endif()