all: krnl32.elf

CFLAGS = \
        -Wl,--no-dynamic-linker \
        -ffreestanding \
        -mno-red-zone \
        -nostartfiles \
        -nodefaultlibs \
        -nostdlib \
        -m32 \
        -Wl,-melf_i386 \
        -Wl,-e_start \
        -masm=intel \
        -fno-stack-protector \
        -O0 \
        -fno-pic \
        -no-pie \
        -Iuacpi/include \
        -mno-sse \
        -mno-mmx \
        -g


objs_uACPI = \
        uacpi/source/uacpi.o \
        uacpi/source/default_handlers.o \
        uacpi/source/event.o \
        uacpi/source/interpreter.o \
        uacpi/source/io.o \
        uacpi/source/mutex.o \
        uacpi/source/namespace.o \
        uacpi/source/notify.o \
        uacpi/source/opcodes.o \
        uacpi/source/opregion.o \
        uacpi/source/osi.o \
        uacpi/source/registers.o \
        uacpi/source/resources.o \
        uacpi/source/shareable.o \
        uacpi/source/sleep.o \
        uacpi/source/stdlib.o \
        uacpi/source/tables.o \
        uacpi/source/types.o \
        uacpi/source/uacpi.o \
        uacpi/source/utilities.o


objs_32 = \
        kmain.o \
        pe_mem.o \
        klib.o \
        kacpi.o \
        video/gc.o \
        video/fb.o \
        video/vcon.o \
        kdi.o \
        fonts/tsvga8x16.o \
        assets/logo.o \
        btrb_compact.o \
        heapm32.o \
        ksmp_apic.o \
        smp_trampoline.o \
        pc_pit.o \
        kio.o \
        kspinlock.o \
        kmutex.o \
        pic8259.o \
        isr.o \
        kint.o \
        ioapic.o \
        lapic.o \
        pci.o \
        kdev.o \
        kprintf.o \
        drivers/device.o \
        drivers/driver.o \
        $(objs_uACPI) \
        uacpi_osi.o

#       fonts/ega8x16.o



kint.o: kint.c
	$(info CC $@)
	@gcc $(CFLAGS) -mgeneral-regs-only -c $< -o $@

%.o: %.S
	$(info AS $@)
	@gcc $(CFLAGS) -c $< -o $@

%.o: %.c
	$(info CC $@)
	@gcc $(CFLAGS) -c -Wa,-aslh=${@:.o=.s} $< -o $@

krnl32.elf: $(objs_32)
	$(info LD $@)
	@gcc $(CFLAGS) -T krnl32.ld $^ -lgcc -o $@
