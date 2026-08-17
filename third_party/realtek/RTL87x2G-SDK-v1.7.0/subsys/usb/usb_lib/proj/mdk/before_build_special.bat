@echo off
echo build_before_special bat
bash.exe ..\..\..\..\tool\version_generation\git_generate_version_sdk_lib.sh bee4-sdk- ..\..\..\..\tool\version_generation\ ..\..\..\..\src\app\bee4_sdk_lib\inc_int\lib_version.h

:: modify for version management
git checkout -- sdk_lib_version.h