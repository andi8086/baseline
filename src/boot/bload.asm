cpu 8086
bits 16

.code segment start=0h


start:
        mov al, 3
        mov bx, 2
        mov cl, 1
        mov ah, 09h
        int 10h

        jmp $
