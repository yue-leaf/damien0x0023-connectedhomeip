..\..\prepend_header\prepend_header.exe -t userdata5 -p "SampleUserData5.bin" -m 1 -c sha256 -b 15
1>NUL ren SampleUserData5.tmp SampleImageUserData5.bin
1>NUL ren SampleUserData5_MP.bin SampleImageUserData5_MP.bin
..\..\md5\md5.exe "SampleImageUserData5_MP.bin"