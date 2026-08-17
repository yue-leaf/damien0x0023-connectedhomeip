ECHO off
rem

for /f "tokens=2,3" %%i in (..\..\..\..\common_header_file\flash_map.h) do (
    REM if %%i==BANK0_OTA_HDR_SIZE set /A BANK0_OTA_HDR_SIZE_CFG=%%j
    REM if %%i==BANK0_BT_STACK_PATCH_SIZE set /A BANK0_BT_STACK_PATCH_SIZE_CFG=%%j
    REM if %%i==BANK0_NS_PATCH_SIZE set /A BANK0_NS_PATCH_SIZE_CFG=%%j
    REM if %%i==BANK0_UPPERSTACK_SIZE set /A BANK0_UPPERSTACK_SIZE_CFG=%%j
    REM if %%i==BANK0_APP_SIZE set /A BANK0_APP_SIZE_CFG=%%j

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

1>NUL del OTAHeader_Bank0.bin
REM ..\MergeBin.exe "..\..\..\..\keil_proj\rom_secure\OTAHeader_Bank0.bin"::;;4096 "..\..\..\..\..\stack-patch-v2.0\sdk\stack_patch\bin\bt_stack_patch.bin"::;;28672 "..\..\..\..\keil_proj\patch_ns\bin\ns_patch.bin"::;;32768 "..\..\..\..\keil_proj\upperstack\bin\upperstack.bin"::;;217088 "..\..\..\..\sdk\board\evb\template\mdk\bin\app.bin"::;;327680
..\MergeBin.exe "..\..\..\..\keil_proj\rom_secure\OTAHeader_Bank0.bin"::;;%BANK0_OTA_HDR_SIZE_CFG% "..\..\..\..\..\stack-patch-v2.0\sdk\stack_patch\bin\bt_stack_patch.bin"::;;%BANK0_BT_STACK_PATCH_SIZE_CFG% "..\..\..\..\keil_proj\patch_ns\bin\ns_patch.bin"::;;%BANK0_NS_PATCH_SIZE_CFG% "..\..\..\..\keil_proj\upperstack\bin\upperstack.bin"::;;%BANK0_UPPERSTACK_SIZE_CFG% "..\..\..\..\sdk\board\evb\template\mdk\bin\app.bin"::;;%BANK0_APP_SIZE_CFG%
1>NUL ren  out.bin OTAHeader_Bank0.bin

pause


