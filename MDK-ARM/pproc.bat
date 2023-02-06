del   ..\bin\*.bin, ..\bin\*.hex
copy  boot\*.hex  ..\bin\*.hex
srec_cat.exe ..\bin\boot.hex -Intel -crop 0x8000000 -offset -0x8000000 -o ..\bin\boot.bin -Binary


