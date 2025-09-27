#include "pc_pit.h"


void outb(uint16_t port, uint8_t val)
{
        asm volatile ("out dx, al" : : "d"(port), "a"(val));
}


void outw(uint16_t port, uint16_t val)
{
        asm volatile ("out dx, ax" : : "d"(port), "a"(val));
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



void pit_wait_ms(uint16_t amount)
{
       outb(0x43, 0x30); /* mode 0 for counter 0*/

       amount = amount * 0x4A9;

       outb(0x40, amount & 0xFF);
       outb(0x40, amount >> 8);
       do {
                outb(0x43, 0xE2);
       } while (!(inb(0x40) & 0x80));
}
