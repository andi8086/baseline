#include <inttypes.h>
#include <stddef.h>


#include "klib.h"
#include "kacpi.h"
#include "ioapic.h"
#include "ksmp_apic.h"


void *kacpi_find_rsdp(void)
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


void *kacpi_find_madt(void *rsdp)
{
        rsdp_header_t *rsdp_h = (rsdp_header_t *)rsdp;

        rsdt_t *r = (rsdt_t *)(uintptr_t)rsdp_h->rsdt_addr;
        int entries = (r->h.length - sizeof(r->h)) / 4;

        for (int i = 0; i < entries; i++) {
                acpi_sdt_header_t *h =
                        (acpi_sdt_header_t *)(uintptr_t)r->sdts[i];
                if (!memcmp(h->sign, "APIC", 4)) {
                        return (void *)h;
                }
        }

        return NULL;
}


void kacpi_madt_init(void *madt)
{
        madt_t *m = (madt_t *)madt;

        uint32_t size = m->h.length;

        smp_lapic_addr = m->lapic_addr;
        apic_header_t *apic = (apic_header_t *)((uint8_t *)m + sizeof(madt_t));

        while ((uint8_t *)apic < (uint8_t *)m + size) {
//                vcon_printf(&boot_console, "Type %p found\n", apic->type);
                if (apic->type == 0) {
                        cpu_lapic_t *cpu_lapic = (cpu_lapic_t *)apic;
                        if ((cpu_lapic->flags & LAPIC_CPU_ENABLED) ||
                            (cpu_lapic->flags & LAPIC_CPU_ONLINE_CAPABLE)) {
                                cpu_add(cpu_lapic->acpi_cpu_id,
                                        cpu_lapic->apic_id);
                        }

                } else if (apic->type == 1) {
                        io_apic_t *io_apic = (io_apic_t *)apic;
                        ioapic_register(io_apic->ioapic_addr,
                                        io_apic->gsi_base);
//                        vcon_printf(&boot_console, "I/O APIC at %p, GSI base = %p\n",
//                                    io_apic->ioapic_addr, io_apic->gsi_base);
                } else if (apic->type == 2) {
                        io_apic_int_override_t *into = (io_apic_int_override_t *)apic;
//                        vcon_printf(&boot_console, "GSI %p, BUS %p, IRQ %p (",
//                                    into->gsi, into->bus, into->irq);
                        if (into->flags & APIC_INT_FLAGS_LOW_ACTIVE) {
//                                vcon_printf(&boot_console, "Low active, ");
                        } else {
//                                vcon_printf(&boot_console, "High active, ");
                        }
                        if (into->flags & APIC_INT_FLAGS_LEVEL_TRIGGERED) {
//                                vcon_printf(&boot_console, "Level triggered");
                        } else {
//                                vcon_printf(&boot_console, "Edge triggered");
                        }
//                        vcon_printf(&boot_console, ")\n");
                }

                apic = (apic_header_t *)((uint8_t *)apic + apic->length);
        }

//        vcon_printf(&boot_console, "%p CPUs registered\n", cpu_get_count());
}
