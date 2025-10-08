#!/bin/bash

qemu-system-i386 -s -S -smp cpus=4 -M q35 -hda disk.img
