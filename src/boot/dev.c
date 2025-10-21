#include "dev.h"
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include "conio.h"
#include "int86.h"


chardev_t keyboard;
chardev_t bios_term;

console_t defconsole;


static char bios_keyb_get(void *ctx)
{
        (void)ctx;
        return 0;
}


static int bios_keyb_put(void *ctx, char c)
{
        (void)ctx;
        (void)c;
        return 0;
}


static char vt_get(void *ctx)
{
        uint16_t k = 0;

        (void)ctx;

        /* read a character from the keyboard
           in blocking mode, this can be changed later */
        _asm {
                "xor ax, ax"
                "int 16h"
                "mov k, ax"
        }

        return (char)k;
}


static int vt_put(void *ctx, char c)
{
        (void)ctx;
        (void)c;

        /* This will emulate a vt102 terminal later,
           but for now we just output the character */
        _asm {
                "push cx"
                "push bx"
                "mov ah, 0Eh"
                "mov al, c"
                "mov bx, 0"
                "mov cx, 1"
                "int 10h"
                "pop bx"
                "pop cx"
        }

        return 0;
}


int dev_init(void)
{
        keyboard.ctx = NULL;
        keyboard.get = bios_keyb_get;
        keyboard.put = bios_keyb_put;

        bios_term.ctx = NULL;
        bios_term.get = vt_get;
        bios_term.put = vt_put;

        defconsole.stdin = &keyboard;
        defconsole.stdout = &bios_term;
        defconsole.stderr = &bios_term;

        return 0;
}


void ser_init(void)
{
        regs86_t rin, rout;        

        rin._ax = 0x0E7;
        rin._dx = 0;
        ser_int86(&rin, &rout);
}

void ser_putc(char c)
{
        regs86_t rin, rout;

        rin._ax = 0x0100 | c;
        rin._dx = 0;

        ser_int86(&rin, &rout);
}
