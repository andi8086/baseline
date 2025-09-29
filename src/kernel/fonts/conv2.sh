#!/bin/bash
# grub-mkfont -o pho437_8x16.psfu pho437_8x16.ttf
objcopy -O elf32-i386 -B i386 -I binary pho437_8x16.psfu pho437_8x16.o
