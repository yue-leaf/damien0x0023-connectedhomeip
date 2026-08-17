..\..\prepend_header\prepend_header.exe -t userdata4 -p "SampleUserData4.bin" -m 1 -c sha256 -b 15
1>NUL ren SampleUserData4.tmp SampleImageUserData4.bin
1>NUL ren SampleUserData4_MP.bin SampleImageUserData4_MP.bin
..\..\md5\md5.exe "SampleImageUserData4_MP.bin"