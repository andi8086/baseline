#!/bin/bash

objcopy -O elf32-i386 -B i386 -I binary TSVGA_ET4000_8x16.bin tsvga8x16.o
