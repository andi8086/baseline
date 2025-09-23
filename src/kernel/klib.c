#include <stddef.h>

#include "klib.h"


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
