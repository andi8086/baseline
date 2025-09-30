#include "kio.h"

#include <stdint.h>


void outb(uint16_t port, uint8_t val)
{
        asm volatile ("out dx, al" : : "d"(port), "a"(val));
}


void outw(uint16_t port, uint16_t val)
{
        asm volatile ("out dx, ax" : : "d"(port), "a"(val));
}


void outd(uint16_t port, uint32_t val)
{
        asm volatile ("out dx, eax" : : "d"(port), "a"(val));
}


uint8_t inb(int16_t port)
{
        uint8_t val;
        asm volatile ("in al, dx" : "=a"(val) : "d"(port));
        return val;
}


uint16_t inw(uint16_t port)
{
        uint16_t val;
        asm volatile ("in ax, dx" : "=a"(val) : "d"(port));
        return val;
}


uint32_t ind(uint16_t port)
{
        uint32_t val;
        asm volatile ("in eax, dx" : "=a"(val) : "d"(port));
        return val;
}

