#include "int86.h"
#include "conio.h"
#include "vesa.h"


int main(void);
uint16_t read_far16(uint16_t seg, uint16_t offs);


void _cstart(void)
{
        __asm {
                mov ax, 70h
                mov ds, ax
                mov es, ax
                mov ss, ax
                mov sp, 0FFFFh
        }
        main();
}


int main(void)
{
        int res;

        res = vesa_init();
        if (res) {
                puts("VESA init failed\r\n");
                while (1);
        }

        while (1);
        return 0;
}



uint16_t read_far16(uint16_t seg, uint16_t offs)
{
        _asm {
                push es
                push bx
                mov bx, dx
                mov es, ax
                mov ax, es:[bx]
                pop bx
                pop es
        }
}

