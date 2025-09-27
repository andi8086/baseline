#ifndef KACPI_H
#define KACPI_H


#include <stdint.h>


#pragma pack(1)
typedef struct {
        char sign[8];
        uint8_t cksum;
        char oemid[6];
        uint8_t rev;
        uint32_t rsdt_addr;
} rsdp_header_t;


typedef struct {
        char sign[4];
        uint32_t length;
        uint8_t rev;
        uint8_t cksum;
        char oemid[6];
        char oemtableid[8];
        uint32_t oemrev;
        uint32_t creatorid;
        uint32_t creatorrev;
} acpi_sdt_header_t;


typedef struct {
        acpi_sdt_header_t h;
        uint32_t sdts[0];
} rsdt_t;

typedef struct {
        uint8_t type;
        uint8_t length;
} apic_header_t;


#define APIC_PCAT_COMPAT 1   // have a 8259 dual setup

typedef struct {
        acpi_sdt_header_t h;
        uint32_t lapic_addr;
        uint32_t flags;
} madt_t;


#define LAPIC_CPU_ENABLED        1
#define LAPIC_CPU_ONLINE_CAPABLE 2

typedef struct {
        uint8_t type;
        uint8_t length;
        uint8_t acpi_cpu_id;
        uint8_t apic_id;
        uint32_t flags;
} cpu_lapic_t;


typedef struct {
        uint8_t type;
        uint8_t length;
        uint8_t ioapic_id;
        uint8_t resv;
        uint32_t ioapic_addr;
        uint32_t gsi_base;
} io_apic_t;


#define APIC_INT_FLAGS_LOW_ACTIVE      2
#define APIC_INT_FLAGS_LEVEL_TRIGGERED 8


typedef struct {
        uint8_t type;
        uint8_t length;
        uint8_t bus;
        uint8_t irq;
        uint32_t gsi;
        uint16_t flags;
} io_apic_int_override_t;


void *kacpi_find_rsdp(void);
void *kacpi_find_madt(void *rsdp);
void kacpi_madt_init(void *madt);


#endif
