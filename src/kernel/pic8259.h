#ifndef PIC8259_H
#define PIC8259_H


#include <stdint.h>


#define PIC_M_CMD 0x20
#define PIC_M_DAT 0x21
#define PIC_S_CMD 0xA0
#define PIC_S_DAT 0xA1

/* End of Interrupt Code */
#define PIC_EOI 0x20


#define ICW1_ICW4       1
#define ICW1_SINGLE     2
#define ICW1_INTERVAL4  4
#define ICW1_LEVEL      8
#define ICW1_INIT       16

#define ICW4_8086       1
#define ICW4_AUTO       2
#define ICW4_BUF_SLAVE  8
#define ICW4_BUF_MASTER 12
#define ICW4_SFNM       16

#define PIC_CASCADE_IRQ 2


void pic_irq_ack(uint8_t irq);
void pic_init(uint8_t int_offset1, uint8_t int_offset2);
void pic_disable(void);


#endif
