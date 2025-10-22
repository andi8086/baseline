#include "mem.h"

#include <stdint.h>
#include <stddef.h>


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


void memcpy(void __far *dst, void __far *src, uint16_t count)
{
        char __far *d = (char __far *)dst;
        char __far *s = (char __far *)src;

        while (count--) {
                *(d++) = *(s++);
        }
}


int strlen(char __far *str)
{
        int l = 0;

        while (*(str++)) {
                l++;
        }
        return l;
}


char __far *strtok(char __far *str, char *sep)
{
        static char __far *token;
        static char __far *next;
        char *c;

        if (str) {
                token = str;
                next = token;
        } else {
                token = next;
        }

        while (next && *next) {
                c = sep;
                while (*c) {
                        if (*next == *c) {
                                *next = 0;
                                next++;
                                return token;
                        } 
                        c++;
                }   
                next++;
        }
        next = NULL;
        return token;
}
