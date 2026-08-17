..\..\prepend_header\prepend_header.exe -t userdata3 -p "SampleUserData3.bin" -m 1 -c sha256 -b 15
1>NUL ren SampleUserData3.tmp SampleImageUserData3.bin
1>NUL ren SampleUserData3_MP.bin SampleImageUserData3_MP.bin
..\..\md5\md5.exe "SampleImageUserData3_MP.bin"