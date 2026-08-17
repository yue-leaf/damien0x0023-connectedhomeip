@echo off
echo after_build_special bat
1>NUL del  ..\..\lib\mdk\sd.lib
1>NUL copy .\objects\sd.lib    ..\..\lib\mdk\sd.lib

goto notesEnd  
:: modify for version management
for /f "tokens=3 delims= " %%a in (..\sd_lib_version.h)do (
    setlocal enabledelayedexpansion
        set take=%%a
        goto save
)
:save 
    set /a version= %take% + 1
    echo %version%


for /f "eol=* tokens=*" %%i in (..\sd_lib_version.h) do ( 
    set a=%%i 
    set "a=!a:%take%=%version%!" 
    echo !a! >>version1.h
)
copy version1.h ..\sd_lib_version.h
del version1.h
:notesEnd
echo need add sdk lib version management in sdk branch