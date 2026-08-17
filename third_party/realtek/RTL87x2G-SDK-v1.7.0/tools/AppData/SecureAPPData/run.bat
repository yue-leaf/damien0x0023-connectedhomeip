..\..\prepend_header\prepend_header.exe -t secure_app_data -p "SampleSecureAppData.bin" -m 1 -c sha256 -b 15
1>NUL ren SampleSecureAppData.tmp SampleImageSecureAppData.bin
1>NUL ren SampleSecureAppData_MP.bin SampleImageSecureAppData_MP.bin
..\..\md5\md5.exe "SampleImageSecureAppData_MP.bin"