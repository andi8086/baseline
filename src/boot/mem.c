#include "mem.h"

#include <stdint.h>


uint16_t _SEG_ES(void)
{
        __asm {
                mov ax, es
        }
}
