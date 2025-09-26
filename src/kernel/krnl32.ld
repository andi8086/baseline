ENTRY(._start)


SECTIONS {
        . = 0x100000;
.boot (READONLY) : {
        . = ALIGN(8);
        mb_header_start = .;
        KEEP(*(.multiboot))
        }
.text (READONLY) : {
        . = ALIGN(8);
        kernel_start = .;
        *(.text)
        kernel_end = .;
        }
        . = ALIGN(CONSTANT(COMMONPAGESIZE));
.data : {
        . = ALIGN(8);
        *(.data)
        kdata_end = .;
        }
.bss : {
        . = ALIGN(8);
        *(.bss)
        kbss_end = .;
        }
}
