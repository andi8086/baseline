#include "pc_pit.h"
#include "kio.h"


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
