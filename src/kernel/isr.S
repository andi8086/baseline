.intel_syntax noprefix

.global trap_handler
.global trap_handler_code
.global int_handler

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
