all: krnl32.elf

CFLAGS = \
        -Wl,--no-dynamic-linker \
        -ffreestanding \
        -nostartfiles \
        -nodefaultlibs \
        -nostdlib \
        -m32 \
        -Wl,-melf_i386 \
        -Wl,-ekmain \
        -masm=intel \
        -fno-stack-protector \
        -O0 \
        -fno-pic \
        -fno-pie


objs_32 = \
        kmain.o \
        pe_mem.o \
        klib.o \
        kacpi.o \
        video/fb.o \
        video/vcon.o \
        fonts/tsvga8x16.o \
        assets/logo.o \
        btrb_compact.o \
        heapm32.o

#       fonts/ega8x16.o

%.o: %.c
	$(info CC $@)
	@gcc $(CFLAGS) -c -Wa,-aslh=${@:.o=.s} $< -o $@

krnl32.elf: $(objs_32)
	$(info LD $@)
	@gcc $(CFLAGS) -T krnl32.ld -lgcc $^ -o $@
