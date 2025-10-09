#!/bin/bash

qemu-system-i386 -s -S -smp cpus=4 -hda disk.img
