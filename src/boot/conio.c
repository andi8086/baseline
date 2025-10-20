#include "conio.h"
#include "int86.h"
#include "mem.h"
#include "dev.h"
#include "c_printf.h"


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


static char printf_buffer[128];


void printf(char __far *fmt, ...)
{
        va_list p;
        char __far *buff = _MK_FP(_FP_SEG(printf_buffer),
                                  _FP_OFF(printf_buffer));

        va_start(p, fmt);
        c_vsnprintf(buff, 128, fmt, p);
        va_end(p);

        puts(printf_buffer);
}
