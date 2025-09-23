ENTRY(.kmain)

SECTIONS {
.boot : {
        mb_header_start = .;
        KEEP(*(.multiboot))
        }
.text : {
        . = 0x10000;
        kernel_start = .;
        *(.text)
        kernel_end = .;
}
.data : { *(.data)
        kdata_end = .;
        }
.bss : { *(.bss)
        kbss_end = .;
        }
}
