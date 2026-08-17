@echo off
echo after_build_special bat
1>NUL del  ..\..\..\lib\rtl87x2g\mdk\rtl87x2g_io.lib
1>NUL copy .\objects\rtl87x2g_io.lib    ..\..\..\lib\rtl87x2g\mdk\