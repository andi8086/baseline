#include "conio.h"
#include "int86.h"
#include "mem.h"
#include "dev.h"
#include "c_printf.h"


int getc(void)
{
        int in;

        in = defconsole.stdin->get(defconsole.stdin);

        return in;
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
        int c;
        int caret_notation = 0;

        while (dst - buffer < 128) {
                c = getc();
//                printf("%02X : %02X\r\n", c >> 8, c & 0xFF);
                /* if we are below 0x20 in low 8-bits and the
                   upper 8 bits are nonzero, we display the carret
                   notation */
                if ((c & 0xFF) == 0x1B) {
                        /* ESC cancels current buffer input */
                        puts("\\\r\n");
                        memset(buffer, 0, 128);
                        dst = buffer;
                        continue;
                }               
 
                if ((c & 0xFF) < 0x20 &&
                    (c & 0xFF) != 8 && (c & 0xFF) != 13) {
                        /* display caret notation */
                        *dst = c & 0xFF;
                        putc('^');
                        caret_notation = 1;
                } else {
                        *dst = c & 0xFF;
                        caret_notation = 0;
                }

                if (*dst == 13) {
                        *dst = 0;
                        return;
                }
                if (*dst == 8) {
                        if (dst > buffer) {
                                if (*(dst-1) < 0x20) {
                                        /* erase two chars for caret not. */
                                        putc(0x8);
                                        putc(0x20);
                                        putc(0x8);
                                }
                                putc(0x8);
                                putc(0x20);
                                putc(0x8);
                                *dst = 0;
                                dst--;
                        } 
                        *dst = 0;
                        continue;
                }

                if (!caret_notation) {
                        putc(*dst);
                } else {
                        putc(*dst + 0x40);
                }
                dst++;
        }
}
