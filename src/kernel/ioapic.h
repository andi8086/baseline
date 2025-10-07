#ifndef IOAPIC_H
#define IOAPIC_H


#include <stdint.h>


#define IOAPIC_REGSEL 0x00
#define IOAPIC_REGWIN 0x10

#define IOAPIC_REG_ID   0
#define IOAPIC_REG_VER  1
#define IOAPIC_REG_ARB  2


#define IOAPIC_MEM(base, x) *(volatile uint32_t *)((base) + (x))

#define IOAPIC_IRQ_REG_LO(irq) (0x10 + ((irq) << 1))
#define IOAPIC_IRQ_REG_HI(irq) (0x10 + ((irq) << 1) + 1)


typedef struct {
        uint32_t base;
        uint32_t gsi_base;
        uint8_t ver;
        uint8_t id;
        uint8_t redir_max;
} ioapic_t;


void ioapic_register(uint32_t base, uint32_t gsi_base);

void ioapic_reg_write(ioapic_t *i, uint8_t reg, uint32_t val);
uint32_t ioapic_reg_read(ioapic_t *i, uint8_t reg);


#define IOAPIC_IRQ_FIXED        0
#define IOAPIC_IRQ_LOWPRIO      1
#define IOAPIC_IRQ_SMI          2
#define IOAPIC_IRQ_NMI          4
#define IOAPIC_IRQ_INIT         5
#define IOAPIC_IRQ_EXTINT       7

#define IOAPIC_DEST_CPU_PHYS    0
#define IOAPIC_DEST_CPU_LOGIC   1

#define IOAPIC_IRQ_ACTIVE_HIGH  0
#define IOAPIC_IRQ_ACTIVE_LOW   1

#define IOAPIC_TRIGGER_EDGE     0
#define IOAPIC_TRIGGER_LEVEL    1

#define IOAPIC_IRQ_ENABLE       0
#define IOAPIC_IRQ_DISABLE      1


void ioapic_irq_config(uint8_t irq,
                       uint8_t vector, uint8_t dlv_mode,
                       uint8_t dst_mode, uint8_t polarity,
                       uint8_t trg_mode, uint8_t irq_mask,
                       uint8_t dst);

void ioapic_irq_mask(uint8_t irq, uint8_t mask);

#define ioapic_irq_enable(irq) ioapic_irq_mask(irq, IOAPIC_IRQ_ENABLE)
#define ioapic_irq_disable(irq) ioapic_irq_mask(irq, IOAPIC_IRQ_DISABLE)

#endif
