all: test.img

test.img:
	mkdir -p iso/boot/grub
	cp ../src/kernel/krnl32.elf iso/boot/
	printf "menuentry \"myos\" {\n\tmultiboot2 /boot/krnl32.elf\n}" > iso/boot/grub/grub.cfg
	grub-mkrescue -o disk.img iso


