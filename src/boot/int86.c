#include "int86.h"


void vid_int86(regs86_t *r_in, regs86_t *r_out)
{
        __asm {
                push ax
                push bx
                push cx
                push dx
                push es
                push di
                push si
                pushf
                push bp
                mov bp, [r_in]
                mov ax, [bp + 8]
                mov es, ax
                mov ax, [bp]
                mov bx, [bp + 2]
                mov cx, [bp + 4]
                mov dx, [bp + 6]
                mov di, [bp + 10]
                mov si, [bp + 14]
                pop bp
                push bp
                int 10h
                pop bp
                push bp
                mov bp, [r_out]
                mov [bp], ax
                pushf
                pop ax
                mov [bp + 12], ax
                mov [bp + 14], si
                mov [bp + 10], di
                mov ax, es
                mov [bp + 8], ax
                mov [bp + 6], dx
                mov [bp + 4], cx
                mov [bp + 2], bx
                pop bp
                popf
                pop si
                pop di
                pop si
                pop es
                pop dx
                pop cx
                pop bx
                pop ax
        }
}


void disk_int86(regs86_t *r_in, regs86_t *r_out)
{
        __asm {
                push ax
                push bx
                push cx
                push dx
                push es
                push di
                push si
                pushf
                push bp
                mov bp, [r_in]
                mov ax, [bp + 8]
                mov es, ax
                mov ax, [bp]
                mov bx, [bp + 2]
                mov cx, [bp + 4]
                mov dx, [bp + 6]
                mov di, [bp + 10]
                mov si, [bp + 14]
                pop bp
                push bp
                int 13h
                pop bp
                push bp
                mov bp, [r_out]
                mov [bp], ax
                pushf
                pop ax
                mov [bp + 12], ax
                mov [bp + 14], si
                mov [bp + 10], di
                mov ax, es
                mov [bp + 8], ax
                mov [bp + 6], dx
                mov [bp + 4], cx
                mov [bp + 2], bx
                pop bp
                popf
                pop si
                pop di
                pop si
                pop es
                pop dx
                pop cx
                pop bx
                pop ax
        }
}

void ser_int86(regs86_t *r_in, regs86_t *r_out)
{
        __asm {
                push ax
                push bx
                push cx
                push dx
                push es
                push di
                push si
                pushf
                push bp
                mov bp, [r_in]
                mov ax, [bp + 8]
                mov es, ax
                mov ax, [bp]
                mov bx, [bp + 2]
                mov cx, [bp + 4]
                mov dx, [bp + 6]
                mov di, [bp + 10]
                mov si, [bp + 14]
                pop bp
                push bp
                int 14h
                pop bp
                push bp
                mov bp, [r_out]
                mov [bp], ax
                pushf
                pop ax
                mov [bp + 12], ax
                mov [bp + 14], si
                mov [bp + 10], di
                mov ax, es
                mov [bp + 8], ax
                mov [bp + 6], dx
                mov [bp + 4], cx
                mov [bp + 2], bx
                pop bp
                popf
                pop si
                pop di
                pop si
                pop es
                pop dx
                pop cx
                pop bx
                pop ax
        }
}

