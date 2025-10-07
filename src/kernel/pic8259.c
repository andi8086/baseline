#include "pic8259.h"

#include "kio.h"

void pic_irq_ack(uint8_t irq)
{
        if (irq >= 8) {
                outb(PIC_S_CMD, PIC_EOI);
        }

        outb(PIC_M_CMD, PIC_EOI);
}


void pic_init(uint8_t int_offset1, uint8_t int_offset2)
{
        outb(PIC_M_CMD, ICW1_INIT | ICW1_ICW4);
        io_delay();
        outb(PIC_S_CMD, ICW1_INIT | ICW1_ICW4);
        io_delay();
        outb(PIC_M_DAT, int_offset1);
        io_delay();
        outb(PIC_S_DAT, int_offset2);
        io_delay();
        outb(PIC_M_DAT, 1 << PIC_CASCADE_IRQ);
        io_delay();
        outb(PIC_S_DAT, 2);
        io_delay();

        outb(PIC_M_DAT, ICW4_8086);
        io_delay();
        outb(PIC_S_DAT, ICW4_8086);
        io_delay();

        outb(PIC_M_DAT, 0);
        outb(PIC_S_DAT, 0);
}


void pic_disable(void)
{
        outb(PIC_M_DAT, 0xFF);
        outb(PIC_S_DAT, 0xFF);
}
