@echo off
:: generate sign key
..\signImage.exe -g ecdsa -r \rtk_ecdsa

..\..\srecord\srec_cat .\rtk_ecdsa_hash.bin -Binary -o ".\ecdsa_public_key_hash.c" -C-Array public_key_hash -INClude

::remove 512bytes MP Header
::bash.exe -c "dd if=OTAHeader_Bank0_MP.bin of=OTAHeader_Bank0.bin bs=1 skip=512"

::sign image
::..\..\prepend_header\prepend_header.exe -t rom_patch -b 15 -p ".\ns_patch.bin" -m 1 -r ".\rtk_rsa.pem"
::..\..\prepend_header\prepend_header.exe -t ota_header -b 15 -p ".\OTAHeader_Bank0.bin" -m 1 -r ".\rtk_ecdsa.pem"

::..\..\srecord\srec_cat ".\OTAHeader_Bank0.bin"   -binary -offset 0x04005000 -o ".\OTAHeader_Bank0.hex" -intel


