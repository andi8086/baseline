#include "mem.h"

#include <stdint.h>



/*
uint16_t _SEG_ES(void)
{
        uint16_t _foo;
        __asm {
                mov ax, es
                mov _foo, ax
        }
        return _foo;
}
*/
/*
uint16_t _SEG_DS(void)
{
        uint16_t _foo;
        __asm {
                mov ax, ds
                mov _foo, ax
        }
        return _foo;
}
*/

void strncpy(void far *dst, void far *src, uint16_t count)
{
        char far *s = (char far *)src;
        char far *d = (char far *)dst;

        while (count-- && *s) {
                *(d++) = *(s++);
        }
}


void memset(void far *dst, uint8_t val, uint16_t count)
{
        char far *d = (char far *)dst;
        while (count--) {
                *d = val;
                d++;
        }
}


int strncmp(char __far *dst, char __far *src, uint16_t count)
{
        while (count--) {
                if (*dst != *src) {
                        return *dst - *src;
                }
                dst++;
                src++;
        }
        return 0;
}
