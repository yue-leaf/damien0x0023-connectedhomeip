SET PATH=C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\Bin;C:\S2C\PlayerPro\bin;C:\Users\echo_gao\AppData\Roaming\ActiveState\bin;C:\ProgramData\Oracle\Java\javapath;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Windows\System32\OpenSSH\;C:\Program Files\TortoiseGit\bin;C:\Program Files\Cppcheck;C:\Strawberry\c\bin;C:\Strawberry\perl\site\bin;C:\Strawberry\perl\bin;C:\Strawberry\perl;C:\Program Files (x86)\Pulse Secure\VC142.CRT\X64\;C:\Program Files (x86)\Pulse Secure\VC142.CRT\X86\;D:\GCC\GCC\gcc-arm-none-eabi-10-2020-q4-major-win32\gcc-arm-none-eabi-10-2020-q4-major\bin;D:\GCC\GCC\MinGW\bin;D:\GCC\GCC\MinGW\include;D:\GCC\GCC\MinGW\lib;C:\Program Files\CMake\bin;C:\Program Files\Git\cmd;C:\Program Files\Git;C:\Program Files\Git\bin;C:\Program Files\Git\usr;C:\Program Files (x86)\Common Files\Pulse Secure\TNC Client Plugin\;C:\Program Files\dotnet\;;%SystemRoot%\system32;%SystemRoot%;%SystemRoot%\System32\Wbem;%SYSTEMROOT%\System32\WindowsPowerShell\v1.0\;%SYSTEMROOT%\System32\OpenSSH\;C:\Users\echo_gao\AppData\Local\Programs\Python\Python314\Scripts\;C:\Users\echo_gao\AppData\Local\Programs\Python\Python314\;C:\Users\echo_gao\AppData\Local\Programs\Python\Launcher\;C:\Users\echo_gao\AppData\Local\Programs\Python\Python39\Scripts\;C:\Users\echo_gao\AppData\Local\Programs\Python\Python39\;C:\Users\echo_gao\AppData\Local\Microsoft\WindowsApps;C:\Program Files\Git\usr\bin;C:\Users\echo_gao\AppData\Local\Programs\Microsoft VS Code\bin;C:\ProgramData\chocoportable\bin;C:\ProgramData\chocoportable;C:\ProgramData\chocolatey\lib\Wget\tools;C:\Program Files\7-Zip;C:\ProgramData\chocolatey\lib\ninja\tools;C:\ProgramData\chocolatey\lib\dtc-msys2\tools\usr\bin;C:\ProgramData\chocolatey\lib\gperf\tools;C:\Program Files (x86)\SEGGER\JLink_V766g;C:\ProgramData\chocolatey\lib\make\tools\install\bin;C:\Users\echo_gao\.dotnet\tools;C:\Program Files\AStyle;C:\Program Files\Cppcheck\bin;C:\Program Files\AStyle\bin
SET CPU_TYPE=ARMv81MML_DSP_DP_MVE_FP
SET CPU_VENDOR=ARM
SET UV2_TARGET=secure_only_app
SET CPU_CLOCK=0x00B71B00
"C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\Bin\ArmClang.exe" @".\objects\startup_rtl.__i"
"C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\Bin\ArmClang.exe" @".\objects\system_rtl.__i"
"C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\Bin\ArmClang.exe" @".\objects\rtl_gpio.__i"
"C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\Bin\ArmClang.exe" @".\objects\rtl_nvic.__i"
"C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\Bin\ArmClang.exe" @".\objects\rtl_pinmux.__i"
"C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\Bin\ArmClang.exe" @".\objects\rtl_rcc.__i"
"C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\Bin\ArmClang.exe" @".\objects\io_dlps.__i"
"C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\Bin\ArmClang.exe" @".\objects\main_ns.__i"
"C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\Bin\ArmClang.exe" @".\objects\app_task.__i"
"C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\Bin\ArmClang.exe" @".\objects\app_usb.__i"
"C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\Bin\ArmClang.exe" @".\objects\usb_dev.__i"
"C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\Bin\ArmClang.exe" @".\objects\usb_hid.__i"
"C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\Bin\ArmLink" --Via ".\Objects\app.lnp"
"C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\Bin\fromelf.exe" ".\Objects\app.axf" --i32combined --output ".\Objects\app.hex"
after_build_script.bat C:\Keil_v5\ARM\ArmCompilerforEmbedded6.22\include D:\Bee4HoneyComb\workspace\beeSDK\samples\usb\usb_hid_mouse\proj\mdk\Objects\app.axf app D:\Bee4HoneyComb\workspace\beeSDK\samples\usb\usb_hid_mouse\proj\mdk\ D:\Bee4HoneyComb\workspace\beeSDK\samples\usb\usb_hid_mouse\proj\mdk\Objects\
