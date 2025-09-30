#include <stddef.h>

#include "klib.h"

#include <stdint.h>


int memcmp(const void *a, const void *b, unsigned long size)
{
        if (!size) {
                return 0;
        }

        while (--size && *(char *)a == *(char *)b) {
                a = (char *)a + 1;
                b = (char *)b + 1;
        }

        return (*((unsigned char *)a) - *((unsigned char *)b));
}


void memcpy(void *dst, void *src, unsigned long size)
{
        uint8_t *d = dst;
        uint8_t *s = src;
        if (!size) {
                return;
        }

        while (--size) {
                *d = *s;
                d++;
                s++;
        }
}


void memmove(void *dst, void *src, uint32_t size)
{
        uint8_t *d = dst;
        uint8_t *s = src;
        if (!size) {
                return;
        }

        while (--size) {
                *d = *s;
                d++;
                s++;
        }
}


void memset(void *dst, uint8_t val, uint32_t size)
{
        uint8_t *d = dst;
        if (!size) {
                return;
        }

        while (--size) {
                *d = val;
                d++;
        }
}
