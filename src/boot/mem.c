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


static int _strtok_is_delim(char c, char __far *delim)
{
        for (; *delim && c != *delim; delim++);
        return *delim && c == *delim;
}


char __far *strtok(char __far *str, char __far *delim)
{
        /* This function behaves like strtok from C standard lib */
        static char __far *backup_string;
        char *ret;

        /* if no string is passed we continue with the stored one */
        if (!str) str = backup_string;
        /* if the stored one is also done, we quit */
        if (!str) return NULL;

        /* first we remove any leading delimiters */
        while (_strtok_is_delim(*(str++), delim));
        /* undo last increment and test if finished */
        if (!*(--str)) return NULL;

        /* now we store the current token, before we insert a 0 terminator at
         * the next found delimiter. We then store the continuation point right
         * after the found delimiter and return the saved token */
        ret = str;
        while (*str) {
                if (_strtok_is_delim(*str, delim)) {
                        *str = 0;
                        backup_string = str + 1;
                        return ret;
                }
                str++;
        }

        /* if str is finished we still have one last token left */
        backup_string = str;
        return ret;
}


void str_append(char __far *str, char __far *what)
{
        int len = strlen(str);
        str += len;
        memcpy(str, what, strlen(what) + 1); 
}


void str_rm_until(char __far *str, char what)
{
        int len = strlen(str);
        str += len;
        for (; len; len--) {
                if (*str == what) {
                        *str = 0;
                        return;
                }
                str--;
        }
}
