ECHO off
rem

::log address map: https://wiki.realtek.com/pages/viewpage.action?spaceKey=Bee1&title=Bee3Pro+log+trace+address
:: merge rom trace
1>NUL del ROM.trace
..\MergeBin\MergeBin.exe ".\input\ROM.trace"::;;65536 ".\input\ROM_NS.trace"
1>NUL ren  out.bin ROM.trace

:: merge app trace
1>NUL del app.trace
..\MergeBin\MergeBin.exe ".\input\app_ns.trace"::;;524288 ".\input\app_s.trace"
1>NUL ren  out.bin app.trace

::patch need merge log.h



