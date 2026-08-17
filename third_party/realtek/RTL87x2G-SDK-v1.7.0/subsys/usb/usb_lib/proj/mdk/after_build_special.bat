@echo off
echo after_build_special bat
1>NUL del  ..\..\lib\rtl87x2g\mdk\usb.lib
1>NUL copy .\objects\usb.lib    ..\..\lib\rtl87x2g\mdk\

