ENTRY(.kmain)

SECTIONS {
        . = 0x100000;
.boot : {
        . = ALIGN(8);
        mb_header_start = .;
        KEEP(*(.multiboot))
        }
.text : {
        . = ALIGN(8);
        kernel_start = .;
        *(.text)
        kernel_end = .;
}
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
