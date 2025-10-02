.code32
.intel_syntax noprefix

.global spin_lock
.global spin_unlock

.align 16
spin_lock:
        push %ebp
        push %edi
        mov  %ebp, %esp
        mov  %eax, 1
        mov  %edi, [%ebp-8]
retry:  xchg [%edi], %eax
        test %eax, %eax
        jnz  spinloop
        pop  %edi
        pop  %ebp
        ret
.align 8
spinloop:
        pause
        cmp [%edi], %al
        jne retry
        jmp spinloop

.align 16
spin_unlock:
        push %ebp
        mov  %ebp, [%esp-8]        // trick to save a register
        mov  dword ptr [%ebp], 0   // release lock
        pop  %ebp
        ret
