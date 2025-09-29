#!/bin/bash

objcopy  -O elf32-i386 -B i386 -I binary logo.data logo.o
