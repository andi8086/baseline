.code16
.intel_syntax noprefix

.global ap_cpu_start
.global ap_boot_flag

ap_cpu_start:
        cli
        cld
        mov %ax, %cs
        mov %ds, %ax
        mov %es, %ax
        mov %al, 1
        mov %di, 0xFFE
        mov [%di], al
        sti
1:      hlt
        jmp 1b

.org 0xFFE
ap_boot_flag: .byte 0
