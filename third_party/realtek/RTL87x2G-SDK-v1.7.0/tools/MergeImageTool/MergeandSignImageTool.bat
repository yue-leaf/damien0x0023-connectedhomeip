ECHO off

REM set TEST_APP=template
REM set TEST_APP=ble_peripheral
REM set linker_output_file_name=app

REM set TEST_APP=ble_peripheral_ns
set TEST_APP=template_ns
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
    if %%i==BANK0_OTA_HDR_ADDR set BANK0_OTA_HDR_ADDR_CFG=%%j
    if %%i==BANK0_OTA_HDR_SIZE set BANK0_OTA_HDR_SIZE_CFG=%%j
    if %%i==BANK0_BT_STACK_PATCH_SIZE set BANK0_BT_STACK_PATCH_SIZE_CFG=%%j
    if %%i==BANK0_NS_PATCH_SIZE set BANK0_NS_PATCH_SIZE_CFG=%%j
    if %%i==BANK0_UPPERSTACK_SIZE set BANK0_UPPERSTACK_SIZE_CFG=%%j
    if %%i==BANK0_APP_SIZE set BANK0_APP_SIZE_CFG=%%j
)
echo BANK0_OTA_HDR_SIZE_CFG=%BANK0_OTA_HDR_SIZE_CFG%
echo BANK0_BT_STACK_PATCH_SIZE_CFG=%BANK0_BT_STACK_PATCH_SIZE_CFG%
echo BANK0_NS_PATCH_SIZE_CFG=%BANK0_NS_PATCH_SIZE_CFG%
echo BANK0_UPPERSTACK_SIZE_CFG=%BANK0_UPPERSTACK_SIZE_CFG%
echo BANK0_APP_SIZE_CFG=%BANK0_APP_SIZE_CFG%

if %dual_bnak% == 1 (
    for /f "tokens=2,3" %%i in (..\..\..\..\common_header_file\flash_map.h) do (
        if %%i==BANK1_OTA_HDR_ADDR set BANK1_OTA_HDR_ADDR_CFG=%%j
        if %%i==BANK1_OTA_HDR_SIZE set BANK1_OTA_HDR_SIZE_CFG=%%j
        if %%i==BANK1_BT_STACK_PATCH_SIZE set BANK1_BT_STACK_PATCH_SIZE_CFG=%%j
        if %%i==BANK1_NS_PATCH_SIZE set BANK1_NS_PATCH_SIZE_CFG=%%j
        if %%i==BANK1_UPPERSTACK_SIZE set BANK1_UPPERSTACK_SIZE_CFG=%%j
        if %%i==BANK1_APP_SIZE set BANK1_APP_SIZE_CFG=%%j
    )

    echo BANK1_OTA_HDR_SIZE_CFG=%BANK1_OTA_HDR_SIZE_CFG%
    echo BANK1_BT_STACK_PATCH_SIZE_CFG=%BANK1_BT_STACK_PATCH_SIZE_CFG%
    echo BANK1_NS_PATCH_SIZE_CFG=%BANK1_NS_PATCH_SIZE_CFG%
    echo BANK1_UPPERSTACK_SIZE_CFG=%BANK1_UPPERSTACK_SIZE_CFG%
    echo BANK1_APP_SIZE_CFG=%BANK1_APP_SIZE_CFG%
)

1>NUL del OTAHeader_Bank0_unsign.bin
1>NUL del OTAHeader_Bank0.bin
1>NUL del OTAHeader_Bank0.hex
1>NUL del OTAHeader_Bank0_MP.bin

1>NUL del OTAHeader_Bank1_unsign.bin
1>NUL del OTAHeader_Bank1.bin
1>NUL del OTAHeader_Bank1.hex
1>NUL del OTAHeader_Bank1_MP.bin

if %dual_bnak% == 1 (
    if %BUILD_BANK_CFG% == 1 (
        ..\MergeBin.exe "..\..\..\..\sdk\board\fpga\flash_download\hex\OTA_Header_support_bank_switch\default_flash_map_witch_secure_app\OTAHeader_Bank1.bin"::;;%BANK1_OTA_HDR_SIZE_CFG% "..\..\..\..\..\stack-patch-v2.0\sdk\stack_patch\bin\bt_stack_patch.bin"::;;%BANK1_BT_STACK_PATCH_SIZE_CFG% "..\..\..\..\keil_proj\patch_ns\bin\ns_patch.bin"::;;%BANK1_NS_PATCH_SIZE_CFG% "..\..\..\..\keil_proj\upperstack\bin\upperstack.bin"::;;%BANK1_UPPERSTACK_SIZE_CFG% "..\..\..\..\sdk\board\evb\%TEST_APP%\mdk\bin\%linker_output_file_name%.bin"::;;%BANK1_APP_SIZE_CFG%
        1>NUL ren  out.bin OTAHeader_Bank1.bin
        1>NUL copy  OTAHeader_Bank1.bin OTAHeader_Bank1_unsign.bin

        call:ModifyOTAHdr1PayloadlenAndSignImg
        goto:EXIT

    ) else (
        :: select bank1 if bank0 and bank1 all valid
        ..\MergeBin.exe "..\..\..\..\sdk\board\fpga\flash_download\hex\OTA_Header_support_bank_switch\default_flash_map_witch_secure_app\OTAHeader_Bank0.bin"::;;%BANK0_OTA_HDR_SIZE_CFG% "..\..\..\..\..\stack-patch-v2.0\sdk\stack_patch\bin\bt_stack_patch.bin"::;;%BANK0_BT_STACK_PATCH_SIZE_CFG% "..\..\..\..\keil_proj\patch_ns\bin\ns_patch.bin"::;;%BANK0_NS_PATCH_SIZE_CFG% "..\..\..\..\keil_proj\upperstack\bin\upperstack.bin"::;;%BANK0_UPPERSTACK_SIZE_CFG% "..\..\..\..\sdk\board\evb\%TEST_APP%\mdk\bin\%linker_output_file_name%.bin"::;;%BANK0_APP_SIZE_CFG%
        :: select bank1 if bank0 and bank1 all valid
        REM ..\MergeBin.exe "..\..\..\..\sdk\board\fpga\flash_download\hex\OTA_Header_support_bank_switch\default_flash_map_witch_secure_app\OTAHeader_Bank0_ver2.bin"::;;%BANK0_OTA_HDR_SIZE_CFG% "..\..\..\..\..\stack-patch-v2.0\sdk\stack_patch\bin\bt_stack_patch.bin"::;;%BANK0_BT_STACK_PATCH_SIZE_CFG% "..\..\..\..\keil_proj\patch_ns\bin\ns_patch.bin"::;;%BANK0_NS_PATCH_SIZE_CFG% "..\..\..\..\keil_proj\upperstack\bin\upperstack.bin"::;;%BANK0_UPPERSTACK_SIZE_CFG% "..\..\..\..\sdk\board\evb\%TEST_APP%\mdk\bin\%linker_output_file_name%.bin"::;;%BANK0_APP_SIZE_CFG%
        1>NUL ren  out.bin OTAHeader_Bank0.bin
        1>NUL copy  OTAHeader_Bank0.bin OTAHeader_Bank0_unsign.bin
        call:ModifyOTAHdr0PayloadlenAndSignImg
        goto:EXIT
    )
) else (
    REM ..\MergeBin.exe "..\..\..\..\keil_proj\rom_secure\OTAHeader_Bank0.bin"::;;4096 "..\..\..\..\..\stack-patch-v2.0\sdk\stack_patch\bin\bt_stack_patch.bin"::;;28672 "..\..\..\..\keil_proj\patch_ns\bin\ns_patch.bin"::;;32768 "..\..\..\..\keil_proj\upperstack\bin\upperstack.bin"::;;217088 "..\..\..\..\sdk\board\evb\template\mdk\bin\app.bin"::;;327680
    ..\MergeBin.exe "..\..\..\..\sdk\board\fpga\flash_download\hex\OTA_Header_not_support_bank_switch\default_flash_map_with_secure_app\OTAHeader_Bank0.bin"::;;%BANK0_OTA_HDR_SIZE_CFG% "..\..\..\..\..\stack-patch-v2.0\sdk\stack_patch\bin\bt_stack_patch.bin"::;;%BANK0_BT_STACK_PATCH_SIZE_CFG% "..\..\..\..\keil_proj\patch_ns\bin\ns_patch.bin"::;;%BANK0_NS_PATCH_SIZE_CFG% "..\..\..\..\keil_proj\upperstack\bin\upperstack.bin"::;;%BANK0_UPPERSTACK_SIZE_CFG% "..\..\..\..\sdk\board\evb\%TEST_APP%\mdk\bin\%linker_output_file_name%.bin"::;;%BANK0_APP_SIZE_CFG%
    1>NUL ren  out.bin OTAHeader_Bank0.bin
    1>NUL copy  OTAHeader_Bank0.bin OTAHeader_Bank0_unsign.bin

    call:ModifyOTAHdr0PayloadlenAndSignImg
    goto:EXIT
)

:ModifyOTAHdr0PayloadlenAndSignImg
:: sign merged image bin
..\prepend_header.exe -t ota_header -b 15 -p ".\OTAHeader_Bank0.bin" -m 1 -r "..\rtk_ecdsa.pem"
REM ..\md5.exe ".\OTAHeader_Bank0.bin"
REM ..\md5.exe ".\OTAHeader_Bank0_MP.bin"
bash.exe -c "dd if=OTAHeader_Bank0_MP.bin of=OTAHeader_Bank0.bin bs=1 skip=512"

echo OTA Header0 download addr: %BANK0_OTA_HDR_ADDR_CFG%
..\srec_cat ".\OTAHeader_Bank0.bin"   -binary -offset %BANK0_OTA_HDR_ADDR_CFG% -o ".\OTAHeader_Bank0.hex" -intel

1>NUL del    ..\..\..\..\keil_proj\rom_secure\OTAHeader_Bank0.hex
1>NUL del    ..\..\..\..\keil_proj\rom_secure\OTAHeader_Bank0.bin
1>NUL copy   .\OTAHeader_Bank0.hex     ..\..\..\..\keil_proj\rom_secure\
1>NUL copy   .\OTAHeader_Bank0.bin     ..\..\..\..\keil_proj\rom_secure\
goto:eof

:ModifyOTAHdr1PayloadlenAndSignImg
:: sign merged image bin
..\prepend_header.exe -t ota_header -b 15 -p ".\OTAHeader_Bank1.bin" -m 1 -r "..\rtk_ecdsa.pem"
REM ..\md5.exe ".\OTAHeader_Bank1.bin"
REM ..\md5.exe ".\OTAHeader_Bank1_MP.bin"
bash.exe -c "dd if=OTAHeader_Bank1_MP.bin of=OTAHeader_Bank1.bin bs=1 skip=512"

echo OTA Header1 download addr: %BANK1_OTA_HDR_ADDR_CFG%
..\srec_cat ".\OTAHeader_Bank1.bin"   -binary -offset %BANK1_OTA_HDR_ADDR_CFG% -o ".\OTAHeader_Bank1.hex" -intel

1>NUL del    ..\..\..\..\keil_proj\rom_secure\OTAHeader_Bank1.hex
1>NUL del    ..\..\..\..\keil_proj\rom_secure\OTAHeader_Bank1.bin
1>NUL copy   .\OTAHeader_Bank1.hex     ..\..\..\..\keil_proj\rom_secure\
1>NUL copy   .\OTAHeader_Bank1.bin     ..\..\..\..\keil_proj\rom_secure\
goto:eof

:EXIT
pause


