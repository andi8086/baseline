cpu 8086
bits 16

.code segment start=0h


start:
        mov ax, 70h
        mov es, ax
        mov ds, ax

        mov al, 3
        mov bx, 2
        mov cl, 1
        mov ah, 09h
        int 10h

        mov di, vbe_info
        mov ax, 0x4f00
        int 10h

        mov ax, word [vbe_modes + 2]
        mov ds, ax
        mov si, word [vbe_modes]
        push si
next_mode:
        pop si
                lodsw
        push si
        cmp ax, 0xffff
        je end_mode_list
        push ds
                call print_mode
        pop ds
        jmp next_mode

end_mode_list:
        pop si
        jmp $


print_mode:
        mov byte [buffer + 5], 0
        mov byte [buffer + 4], ' '
        mov di, buffer + 3
        mov cx, 4
        std
next_char:
        push ax
        and ax, 0x000f
        mov bx, ax
        lea si, [hex_chars + bx]
        movsb
        pop ax
        shr ax, 1
        shr ax, 1
        shr ax, 1
        shr ax, 1
        loop next_char
        cld

        mov ax, 70h
        mov ds, ax
        mov si, buffer
        call puts

        ret


puts:
        lodsb
        test al, al
        jz ends
        call putc
        jmp puts
ends:
        ret


putc:
        mov bx, 5
        mov ah, 0eh
        mov cx, 1
        int 10h
        ret


hex_chars: db "0123456789ABCDEF"
buffer:    resb 8

vbe_info:
        db "VBE2"
vbe_version: resw 1
        resd 1          ; oem pointer
vbe_caps: resd 1
vbe_modes: resd 1
        resb 512-18
