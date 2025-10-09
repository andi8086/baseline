.intel_syntax noprefix

.global trap_handler
.global trap_handler_code
.global int_handler
.global irq0_handler
.global irq1_handler
.global irq2_handler
.global irq3_handler
.global irq4_handler
.global irq5_handler
.global irq6_handler
.global irq7_handler
.global irq8_handler
.global irq9_handler
.global irq10_handler
.global irq11_handler
.global irq12_handler
.global irq13_handler
.global irq14_handler
.global irq15_handler


.align 4

trap_handler:
        pushad
        cld     // needed for sysV ABI
        call _trap_handler
        popad
        iret

trap_handler_code:
        pushad
        cld
        call _trap_handler_code
        popad
        iret

int_handler:
        pushad
        cld
        call _int_handler
        popad
        iret

irq0_handler:
        pushad
        cld
        call _irq0_handler
        popad
        iret

irq1_handler:
        pushad
        cld
        call _irq1_handler
        popad
        iret

irq2_handler:
        pushad
        cld
        call _irq2_handler
        popad
        iret

irq3_handler:
        pushad
        cld
        call _irq3_handler
        popad
        iret

irq4_handler:
        pushad
        cld
        call _irq4_handler
        popad
        iret

irq5_handler:
        pushad
        cld
        call _irq5_handler
        popad
        iret

irq6_handler:
        pushad
        cld
        call _irq6_handler
        popad
        iret

irq7_handler:
        pushad
        cld
        call _irq7_handler
        popad
        iret

irq8_handler:
        pushad
        cld
        call _irq8_handler
        popad
        iret

irq9_handler:
        pushad
        cld
        call _irq9_handler
        popad
        iret

irq10_handler:
        pushad
        cld
        call _irq10_handler
        popad
        iret

irq11_handler:
        pushad
        cld
        call _irq11_handler
        popad
        iret

irq12_handler:
        pushad
        cld
        call _irq12_handler
        popad
        iret

irq13_handler:
        pushad
        cld
        call _irq13_handler
        popad
        iret

irq14_handler:
        pushad
        cld
        call _irq14_handler
        popad
        iret

irq15_handler:
        pushad
        cld
        call _irq15_handler
        popad
        iret
