@echo off
echo after_build_special bat
1>NUL del  ..\..\lib\mdk\lwip.lib
1>NUL copy .\objects\lwip.lib    ..\..\lib\mdk\lwip.lib