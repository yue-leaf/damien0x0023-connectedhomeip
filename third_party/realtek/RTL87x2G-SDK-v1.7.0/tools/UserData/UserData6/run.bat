..\..\prepend_header\prepend_header.exe -t userdata6 -p "SampleUserData6.bin" -m 1 -c sha256 -b 15
1>NUL ren SampleUserData6.tmp SampleImageUserData6.bin
1>NUL ren SampleUserData6_MP.bin SampleImageUserData6_MP.bin
..\..\md5\md5.exe "SampleImageUserData6_MP.bin"