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

void ser_puts(char *s)
{
        while (*s) {
                ser_putc(*s);
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


void ser_printf(char __far *fmt, ...)
{
        va_list p;
        char __far *buff = _MK_FP(_FP_SEG(printf_buffer),
                                  _FP_OFF(printf_buffer));

        va_start(p, fmt);
        c_vsnprintf(buff, 128, fmt, p);
        va_end(p);

        ser_puts(printf_buffer);
}


void buffered_input(char __far *buffer)
{
        char __far *dst = buffer;

        while (dst - buffer < 128) {
                *dst = getc();
                if (*dst == 13) {
                        *dst = 0;
                        return;
                }
                if (*dst == 8) {
                        if (dst > buffer) {
                                putc(0x8);
                                putc(0x20);
                                putc(0x8);
                                *dst = 0;
                                dst--;
                        } 
                        *dst = 0;
                        continue;
                }
                putc(*dst);
                dst++;
        }
}
