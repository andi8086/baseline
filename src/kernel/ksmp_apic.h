#ifndef KSMP_APIC_H
#define KSMP_APIC_H


#include <stdint.h>


#define LAPIC_ICMR     0x300
#define LAPIC_ICMR_DST 0x310

extern uint32_t smp_lapic_addr;


typedef struct {
        uint8_t acpi_id; /* don't mix up these two :) */
        uint8_t apic_id;
} cpu_info_t;


void cpu_add(uint8_t acpi_id, uint8_t apic_id);
uint8_t cpu_get_count(void);
int cpu_wake(cpu_info_t *cpu);
uint8_t cpu_wake_all(void);

#endif
