all: bootdisk

boot.bin: boot.asm
	nasm -f bin $< -o $@

# bload.sys: bload.asm
# 	nasm -f bin $< -o $@


# WCC Flags:
# -0:  8086 code
# -d0: no debug information
# -s: remove stack overflow checks
# -wx: maximum warning level
# -zl: remove default lib info
# -zu: SS != DGROUP
# -ms: Memory model = small
# -zq: quiet

%.o: %.c
	wcc -0 -d0 -s -wx -zl -ms -zq -zp1 $<


space := $(empty) $(empty)
comma := ,

BLOAD_OBJc := bload.o,int86.o,vesa.o,conio.o,mem.o
BLOAD_OBJs = $(subst $(comma),$(space),$(BLOAD_OBJc))


bload.sys: $(BLOAD_OBJs)
	$(info $^)
	wlink FILE $(BLOAD_OBJc) FORMAT RAW BIN NAME $@ OPTION NODEFAULTLIBS, START=_cstart_

bootdisk: boot.bin bload.sys
	dd if=/dev/zero of=floppy.bin bs=1024 count=1440
	mformat -i floppy.bin
	dd if=boot.bin conv=notrunc of=floppy.bin bs=512 count=1
	mcopy -i floppy.bin bload.sys ::/
