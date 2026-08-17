ECHO off

set TEST_APP=ble_ota_ns
set linker_output_file_name=app_ns

:: Get build bank from config.h
for /f "tokens=2,3" %%i in (..\..\..\..\rom\config\mem_config.h) do (
    if %%i==BUILD_BANK set BUILD_BANK_CFG=%%j
)
echo BUILD_BANK_CFG=%BUILD_BANK_CFG%

for /f "tokens=2,3" %%i in (..\..\..\..\sdk\board\evb\%TEST_APP%\mem_config.h) do (
    if %%i==BUILD_BANK set APP_BUILD_BANK_CFG=%%j
)
echo APP_BUILD_BANK_CFG=%APP_BUILD_BANK_CFG%

if %BUILD_BANK_CFG% == %APP_BUILD_BANK_CFG% (
    echo check app buid bank pass
) else (
    echo check app buid bank fail !!!!
)

for /f "tokens=2,3" %%i in (..\..\..\..\common_header_file\flash_map.h) do (
    if %%i==OTA_BANK0_SIZE set OTA_BANK0_SIZE_CFG=%%j
    if %%i==OTA_BANK1_SIZE set OTA_BANK1_SIZE_CFG=%%j
)
echo OTA_BANK0_SIZE_CFG=%OTA_BANK0_SIZE_CFG%
echo OTA_BANK1_SIZE_CFG=%OTA_BANK1_SIZE_CFG%

if %OTA_BANK0_SIZE_CFG% == %OTA_BANK1_SIZE_CFG% (
    set /A dual_bnak=1
) else (
    set /A dual_bnak=0
)
echo dual_bank is %dual_bnak%

for /f "tokens=2,3" %%i in (..\..\..\..\common_header_file\flash_map.h) do (

    if %%i==BOOT_PATCH0_ADDR set BOOT_PATCH0_ADDR_CFG=%%j
    if %%i==BOOT_PATCH0_SIZE set BOOT_PATCH0_SIZE_CFG=%%j
    if %%i==BOOT_PATCH1_ADDR set BOOT_PATCH1_ADDR_CFG=%%j
    if %%i==BOOT_PATCH1_SIZE set BOOT_PATCH1_SIZE_CFG=%%j
    if %%i==SECURE_APP_CODE0_ADDR set SECURE_APP_CODE0_ADDR_CFG=%%j
    if %%i==SECURE_APP_CODE0_SIZE set SECURE_APP_CODE0_SIZE_CFG=%%j
    if %%i==SECURE_APP_CODE1_ADDR set SECURE_APP_CODE1_ADDR_CFG=%%j
    if %%i==SECURE_APP_CODE1_SIZE set SECURE_APP_CODE1_SIZE_CFG=%%j

    if %%i==OTA_TMP_ADDR set OTA_TMP_ADDR_CFG=%%j

    if %%i==BANK0_OTA_HDR_SIZE set BANK0_OTA_HDR_SIZE_CFG=%%j
    if %%i==BANK0_BT_STACK_PATCH_SIZE set BANK0_BT_STACK_PATCH_SIZE_CFG=%%j
    if %%i==BANK0_NS_PATCH_SIZE set BANK0_NS_PATCH_SIZE_CFG=%%j
    if %%i==BANK0_UPPERSTACK_SIZE set BANK0_UPPERSTACK_SIZE_CFG=%%j
    if %%i==BANK0_APP_SIZE set BANK0_APP_SIZE_CFG=%%j
)
echo BOOT_PATCH0_ADDR_CFG=%BOOT_PATCH0_ADDR_CFG%
echo BOOT_PATCH1_ADDR_CFG=%BOOT_PATCH1_ADDR_CFG%
echo BOOT_PATCH0_SIZE_CFG=%BOOT_PATCH0_SIZE_CFG%

echo SECURE_APP_CODE0_ADDR_CFG=%SECURE_APP_CODE0_ADDR_CFG%
echo SECURE_APP_CODE1_ADDR_CFG=%SECURE_APP_CODE1_ADDR_CFG%
echo SECURE_APP_CODE0_SIZE_CFG=%SECURE_APP_CODE0_SIZE_CFG%


echo BANK0_OTA_HDR_SIZE_CFG=%BANK0_OTA_HDR_SIZE_CFG%
echo BANK0_BT_STACK_PATCH_SIZE_CFG=%BANK0_BT_STACK_PATCH_SIZE_CFG%
echo BANK0_NS_PATCH_SIZE_CFG=%BANK0_NS_PATCH_SIZE_CFG%
echo BANK0_UPPERSTACK_SIZE_CFG=%BANK0_UPPERSTACK_SIZE_CFG%
echo BANK0_APP_SIZE_CFG=%BANK0_APP_SIZE_CFG%

1>NUL del OTA_TMP.bin
1>NUL del OTA_TMP.hex

if %dual_bnak% == 1 (
    ..\MergeBin.exe "..\..\..\..\keil_proj\boot_patch_s\bin\boot_patch.bin"::;; "..\..\..\..\keil_proj\secure_app\bin\app_s.bin"
) else (
    :: dfu update images in ota bank
    REM ..\MergeBin.exe "..\..\..\..\..\stack-patch-v2.0\sdk\stack_patch\bin\bt_stack_patch.bin"::;; "..\..\..\..\keil_proj\patch_ns\bin\ns_patch.bin"::;; "..\..\..\..\keil_proj\upperstack\bin\upperstack.bin"::;; "..\..\..\..\sdk\board\evb\%TEST_APP%\mdk\bin\%linker_output_file_name%.bin"::;;
    :: dfu update boot patch and images in ota bank
    ..\MergeBin.exe "..\..\..\..\keil_proj\boot_patch_s\bin\boot_patch.bin"::;; "..\..\..\..\..\stack-patch-v2.0\sdk\stack_patch\bin\bt_stack_patch.bin"::;; "..\..\..\..\keil_proj\patch_ns\bin\ns_patch.bin"::;; "..\..\..\..\keil_proj\upperstack\bin\upperstack.bin"::;; "..\..\..\..\sdk\board\evb\%TEST_APP%\mdk\bin\%linker_output_file_name%.bin"::;;
)
1>NUL ren  out.bin OTA_TMP.bin


echo OTA Temp download addr: %OTA_TMP_ADDR_CFG%
..\srec_cat ".\OTA_TMP.bin"   -binary -offset %OTA_TMP_ADDR_CFG% -o ".\OTA_TMP.hex" -intel

1>NUL del    ..\..\..\..\keil_proj\rom_secure\OTA_TMP.hex
1>NUL del    ..\..\..\..\keil_proj\rom_secure\OTA_TMP.bin
1>NUL copy   .\OTA_TMP.hex     ..\..\..\..\keil_proj\rom_secure\
1>NUL copy   .\OTA_TMP.bin     ..\..\..\..\keil_proj\rom_secure\

pause


