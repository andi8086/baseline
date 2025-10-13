all: bootdisk

boot.bin: boot.asm
	$(info ******************** BOOT SECTOR  **********************)
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
# -zp1: byte-pack structures

%.o: %.c
	$(info WCC $@)
	@wcc -0 -d0 -s -wx -zl -ms -zq -zp1 $<

%.o: %.asm
	$(info WASM $@)
	@wasm -0 -mt -zcm=tasm $<

space := $(empty) $(empty)
comma := ,

BLOAD_OBJc := bload.o,int86.o,vesa.o,conio.o,mem.o,dev.o,cpu.o,system.o
BLOAD_OBJs = $(subst $(comma),$(space),$(BLOAD_OBJc))


bload.sys: $(BLOAD_OBJs)
	$(info ********************* BLOAD.SYS ************************)
	wlink FILE $(BLOAD_OBJc) FORMAT RAW BIN NAME $@ OPTION NODEFAULTLIBS, START=_cstart_, QUIET

bootdisk: boot.bin bload.sys
	$(info ********************* BOOTDISK *************************)
	dd if=/dev/zero of=floppy.bin bs=1024 count=1440
	mformat -i floppy.bin
	dd if=boot.bin conv=notrunc of=floppy.bin bs=512 count=1
	mcopy -i floppy.bin bload.sys ::/
