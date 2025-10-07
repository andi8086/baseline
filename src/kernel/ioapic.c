#include "ioapic.h"

#include <stddef.h>


/* we maximally support 4 IO-APICs */
ioapic_t ioapics[4];
int nioapics;


void ioapic_register(uint32_t base, uint32_t gsi_base)
{
        ioapic_t *ioapic = &ioapics[nioapics];

        ioapic->base = base;

        ioapic->id = (ioapic_reg_read(ioapic, IOAPIC_REG_ID) >> 24) & 0xF0;

        uint32_t v = ioapic_reg_read(ioapic, IOAPIC_REG_VER);
        ioapic->ver = (uint8_t)v;
        ioapic->redir_max = (uint8_t)(v >> 16);

        ioapic->gsi_base = gsi_base;

        nioapics++;
}


void ioapic_reg_write(ioapic_t *i, uint8_t reg, uint32_t val)
{
        IOAPIC_MEM(i->base, IOAPIC_REGSEL) = reg;
        IOAPIC_MEM(i->base, IOAPIC_REGWIN) = val;
}


uint32_t ioapic_reg_read(ioapic_t *i, uint8_t reg)
{
        IOAPIC_MEM(i->base, IOAPIC_REGSEL) = reg;
        return IOAPIC_MEM(i->base, IOAPIC_REGWIN);
}


void ioapic_irq_config(uint8_t irq,
                       uint8_t vector, uint8_t dlv_mode,
                       uint8_t dst_mode, uint8_t polarity,
                       uint8_t trg_mode, uint8_t irq_mask,
                       uint8_t dst)
{
        /* find correct ioapic */
        ioapic_t *p = NULL;

        for (int n = 0; n < nioapics; n++) {
                if (irq >= ioapics[n].gsi_base &&
                    irq < ioapics[n].gsi_base + ioapics[n].redir_max) {
                        p = &ioapics[n];
                        break;
                }
        }
        if (!p) {
                return;
        }

        uint32_t dlvmode = (uint32_t)(dlv_mode << 8) & 7;
        uint32_t dstmode = (uint32_t)(dst_mode & 1) << 12;
        uint32_t pinpol = (uint32_t)(polarity & 1) << 13;
        uint32_t trigger = (uint32_t)(trg_mode & 1) << 15;
        uint32_t maskbit = (uint32_t)(irq_mask & 1) << 16;

        uint32_t destcpu = (uint32_t)dst << 24;

        ioapic_reg_write(p, IOAPIC_IRQ_REG_LO(irq - p->gsi_base),
                dlvmode | dstmode | pinpol | trigger | maskbit | vector);

        ioapic_reg_write(p, IOAPIC_IRQ_REG_HI(irq - p->gsi_base), destcpu);
}


void ioapic_irq_mask(uint8_t irq, uint8_t mask)
{
        /* find correct ioapic */
        ioapic_t *p = NULL;

        for (int n = 0; n < nioapics; n++) {
                if (irq >= ioapics[n].gsi_base &&
                    irq < ioapics[n].gsi_base + ioapics[n].redir_max) {
                        p = &ioapics[n];
                        break;
                }
        }
        if (!p) {
                return;
        }

        uint32_t tmp;

        tmp = ioapic_reg_read(p, IOAPIC_IRQ_REG_LO(irq - p->gsi_base));
        tmp &= ~(1UL << 16);
        tmp |= (uint32_t)(mask & 1) << 16;
        ioapic_reg_write(p, IOAPIC_IRQ_REG_HI(irq - p->gsi_base), tmp);
}


