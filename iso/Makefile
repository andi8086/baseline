all: test.img

test.img:
	mkdir -p iso/boot/grub
	cp ../src/kernel/krnl32.elf iso/boot/
	cp grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -v -o disk.img iso


