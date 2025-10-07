#include "ksmp_apic.h"
#include "klib.h"
#include "pc_pit.h"
#include "video/vcon.h"


uint32_t smp_lapic_addr;

/* We do not support more than 256 CPUs */
cpu_info_t cpu_table[256];


static uint8_t num_cpus = 0;


#define LAPIC_REG(x) (*((volatile uint32_t *)\
                        (uintptr_t)(smp_lapic_addr + (x))))

extern vcon_t boot_console;

int cpu_wake(cpu_info_t *cpu)
{
        volatile uint8_t *ap_boot_flag = (uint8_t *)0x8FFE;

        *ap_boot_flag = 0;

        // clear LAPIC errors
        LAPIC_REG(0x280) = 0;

        /* Set target of LAPIC interrupt */
        /*      Bits 27..24 is destination in physical mode */
        /*      However, this way we can only wake 16 CPU cores */
        LAPIC_REG(0x310) = (uint32_t)cpu->apic_id << 24;

        /* Triger INIT IPI */
        LAPIC_REG(0x300) = (LAPIC_REG(0x300) & 0xFFF00000) | 0x00C500;

        /* wait for delivery */
        do {
                asm volatile ("pause" ::: "memory");
        } while (LAPIC_REG(0x300) & (1 << 12));

        /* Select target CPU */
        LAPIC_REG(0x310) = (uint32_t)cpu->apic_id << 24;

        /* Deassert INIT */
        LAPIC_REG(0x300) = (LAPIC_REG(0x300) & 0xFFF00000) | 0x008500;

        /* wait for delivery */
        do {
                asm volatile ("pause" ::: "memory");
        } while (LAPIC_REG(0x300) & (1 << 12));

        pit_wait_ms(10);

        for (int i = 0; i < 2; i++) {
                // clear LAPIC errors
                LAPIC_REG(0x280) = 0;
                LAPIC_REG(0x310) = (uint32_t)cpu->apic_id << 24;
                LAPIC_REG(0x300) = (LAPIC_REG(0x300) & 0xFFF0F800) |
                        0x00608; // startup at 0x0800:0x0000 */

                /* wait for delivery */
                do {
                        asm volatile ("pause" ::: "memory");
                } while (LAPIC_REG(0x300) & (1 << 12));

                pit_wait_ms(1);
        }

        while (!*ap_boot_flag);

        return 0;
}


void cpu_add(uint8_t acpi_id, uint8_t apic_id)
{
        cpu_table[num_cpus].acpi_id = acpi_id;
        cpu_table[num_cpus].apic_id = apic_id;

        num_cpus++;
}


uint8_t cpu_get_count(void)
{
        return num_cpus;
}


extern void (*ap_cpu_start)(void);


uint8_t cpu_wake_all(void)
{
        /* prepare trampoline code to wake up CPUs */
        memcpy((void *)0x8000, &ap_cpu_start, 4096);

        uint8_t bsp_id;
        uint8_t cpus_woken = 0;

        /* retrieve our own apic id */
        asm volatile (
                "mov eax, 1\n"
                "cpuid\n"
                "shr ebx, 24\n"
                : "=b"(bsp_id) ::);

        for (int i = 0; i < num_cpus; i++) {
                if (i > 15) {
                        break;
                }
                if (cpu_table[i].apic_id == bsp_id) {
                        continue;
                }
                cpu_wake(&cpu_table[i]);
                cpus_woken++;
        }
        return cpus_woken;
}
