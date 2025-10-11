all: bootdisk

boot.bin: boot.asm
	nasm -f bin $< -o $@


bload.sys: bload.asm
	nasm -f bin $< -o $@

bootdisk: boot.bin bload.sys
	dd if=/dev/zero of=floppy.bin bs=1024 count=1440
	mformat -i floppy.bin
	dd if=boot.bin conv=notrunc of=floppy.bin bs=512 count=1
	mcopy -i floppy.bin bload.sys ::/
