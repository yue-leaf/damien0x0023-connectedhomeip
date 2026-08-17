..\..\prepend_header\prepend_header.exe -t userdata8 -p "SampleUserData8.bin" -m 1 -c sha256 -b 15
1>NUL ren SampleUserData8.tmp SampleImageUserData8.bin
1>NUL ren SampleUserData8_MP.bin SampleImageUserData8_MP.bin
..\..\md5\md5.exe "SampleImageUserData8_MP.bin"