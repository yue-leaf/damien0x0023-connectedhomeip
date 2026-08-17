..\..\prepend_header\prepend_header.exe -t userdata7 -p "SampleUserData7.bin" -m 1 -c sha256 -b 15
1>NUL ren SampleUserData7.tmp SampleImageUserData7.bin
1>NUL ren SampleUserData7_MP.bin SampleImageUserData7_MP.bin
..\..\md5\md5.exe "SampleImageUserData7_MP.bin"