.code16
.intel_syntax noprefix

/* where this code is moved to, absolute linear addr */
#define RELOC_ADDR 0x00008000

.global ap_cpu_start
// .global pm_enter
// .global pm_gdt
// .global pm_gdt_table

// start code for CPU core is maximally 4K in size
// the last byte is used as boot flag (offset 0xFFE)

ap_cpu_start:
        cli
        cld
        mov ax, cs
        mov ds, ax
        mov es, ax
        mov al, 1
        mov di, 0xFFE
        mov [di], al   // tell the kernel core is up
        mov eax, offset pmgdt - ap_cpu_start
        lgdt [eax]
        mov eax, cr0
        or eax, 1
        mov cr0, eax    // enter protected mode
        .byte 0x66
        .byte 0xEA
        .long offset pm_enter // we jump to the unrelocated code!
        .word 0x08

// here we are above 1M, unrelocated!, reload GDT to kernel GDT
// then we are able to free up space below 1M after all cores are up
.code32
pm_enter:
        mov ax, 0x10
        mov ds, ax

        lgdt [gdt]
        jmp 0x08:reload_cs
reload_cs:

        mov ss, ax
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

// FIXME: we must setup a per-core stack!
1:
        hlt
        jmp 1b

.align 16
pmgdt:
        .word 23
gdtaddr: .long offset pmgdttable - ap_cpu_start + RELOC_ADDR

.align 16
pmgdttable:
        // 0x00
        .long 0
        .long 0
        // 0x08
        .word 0xffff
        .word 0
        .byte 0
        .byte 0b10011010
        .byte 0b11001111
        .byte 0
        // 0x10
        .word 0xffff
        .word 0
        .byte 0
        .byte 0b10010010
        .byte 0b11001111
        .byte 0
pmgdttableend:
        .byte 0

