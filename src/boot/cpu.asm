.8086


_TEXT segment public use16 'code'
        assume cs:_TEXT, ds:_TEXT, es:nothing

CPU_TYPE_8086  EQU 0
CPU_TYPE_V20   EQU 1
CPU_TYPE_80186 EQU 2
CPU_TYPE_80286 EQU 3
CPU_TYPE_80386 EQU 4


cpu_detect_ proc near public
        push dx
        pushf
        push cx

        xor  ax, ax
        push ax
        popf
        pushf
        pop  ax

        ; on the 8086, some flags cannot be
        ; pushed to zero

        and ax, 0f000h
        cmp ax, 0f000h
        je @@less_than_286

        mov dl, CPU_TYPE_80286

        ; check if we have a i386
        mov  ax, 7000h
        push ax
        popf
        pushf
        pop  ax

        and  ax, 7000h
        jz @@done

        inc dl          ; we have a i386 or greater
        jmp @@done

@@less_than_286:

        mov dl, CPU_TYPE_80186

        ; 80186 / 80188  mask out upper 3 bits of the
        ; shift maximum of 31 before shifting
        ; so a shift count of 33 means a shift count of 1

        mov al, 0ffh
        mov cl, 33
        shr al, cl
        jnz @@done

        ; maybe a NEC V20

        mov dl, CPU_TYPE_V20

        sti
        push si

        mov si, 0
        mov cx, 0ffffh
        rep lods BYTE PTR es:[si]

        pop si

        ; in the test above, 'rep lods' is a nonsensical
        ; combination, together with 'es:', it has two
        ; prefixes. If an interrupt occurs, the 8086 loses
        ; the first prefix, which causes 'rep' to abort.
        ; the NEC V20 however will finish the loop.

        or cx, cx
        jz @@done

        mov dl, CPU_TYPE_8086
@@done:
        xor dh, dh
        mov ax, dx

        pop cx
        popf
        pop dx
        ret
cpu_detect_ endp

_TEXT ends

end
