.code32
.intel_syntax noprefix


#define MUTEX_USED   0x80000000
#define MUTEX_LOCKED 0x00000001

.global spin_lock
.global spin_unlock
.global spin_lock_try_once

.align 16
spin_lock:
        push %ebp
        mov  %ebp, %esp
        push %edi
        mov  %eax, MUTEX_USED | MUTEX_LOCKED
        mov  %edi, [%ebp+8]
retry:  xchg [%edi], %eax
        test %eax, MUTEX_LOCKED  // was it locked?
        jnz  spinloop            // yes, goto spinloop
        pop  %edi                // no, so it is now
        pop  %ebp
        ret
.align 8
spinloop:
        pause                   // sleep until mem access
        cmp [%edi], %al         // locked bit in al, test if set in [edi]
        jne retry               // no? then retry above, seems unlocked now
        jmp spinloop            // sleep again

.align 16
spin_unlock:
        push %ebp
        mov  %ebp, [%esp+8]                 // trick to save a register
        and  dword ptr [%ebp], MUTEX_USED   // release lock, but mark as used
        pop  %ebp
        ret

.align 16
spin_lock_try_once:
        push %ebp
        mov  %ebp, %esp
        push %edi
        mov  %eax, MUTEX_USED | MUTEX_LOCKED
        mov  %edi, [%ebp+8]
        xchg [%edi], %eax
        pop %edi
        pop %ebp
        ret
