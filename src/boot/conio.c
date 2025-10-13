#include "conio.h"
#include "int86.h"
#include "mem.h"
#include "dev.h"


char getc(void)
{
        return defconsole.stdin->get(defconsole.stdin);
}


void putc(char c)
{
        defconsole.stdout->put(defconsole.stdout, c);
}


void puts(char *s)
{
        while (*s) {
                defconsole.stdout->put(defconsole.stdout, *s);
                s++;
        }
}


/* will not work if buffer is on stack of function! */
char buffer[8];
const char near *hex_chars = "0123456789ABCDEF";


void dump16(uint16_t num)
{
        int16_t i;
        uint16_t n = num;
        uint16_t idx;

        buffer[4] = ' ';
        buffer[5] = '\0';

        for (i = 3; i >= 0; i--) {
                idx = n & 15;
                buffer[i] = *(hex_chars + idx);
                n >>= 4;
        }

        puts(buffer);

        return;

        __asm("ADD AX, 0");
}


