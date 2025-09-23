#include <inttypes.h>
#include <stddef.h>

#include "klib.h"

void *find_rsdp(void)
{
        char *start = (char *)0x80000;

        while ((uintptr_t)start < 0xA0000) {
                if (memcmp(start, "RSD PTR ", 8) == 0) {
                        return start;
                }
                start++;
        }

        start = (char *)0xE0000;

        while ((uintptr_t)start < 0x100000) {
                if (memcmp(start, "RSD PTR ", 8) == 0) {
                        return start;
                }
                start++;
        }

        return NULL;
}
