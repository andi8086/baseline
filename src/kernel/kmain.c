#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6

#include <stdint.h>


extern char mb_header_start;
extern char kdata_end;
extern char kbss_end;
extern char kernel_start;
extern char kernel_end;

void kmain(void);

#pragma pack(1)
typedef struct {
        uint16_t type;
        uint16_t flags;
        uint32_t size;
} mb_tag_header_t;

struct multiboot_header {
        uint32_t magic;
        uint32_t arch;
        uint32_t hlen;
        uint32_t chksum;
        mb_tag_header_t tag0;
        uint32_t header_addr;
        uint32_t load_addr;
        uint32_t load_end_addr;
        uint32_t bss_end_addr;
        mb_tag_header_t tag1;
        uint32_t entry;
        uint32_t _dummy;
        mb_tag_header_t term;
} mb_header  __attribute__((section(".multiboot"))) = {
        .magic = MULTIBOOT2_HEADER_MAGIC,
        .arch = 0,
        .hlen = sizeof(mb_header),
        .chksum = -(sizeof(mb_header)+MULTIBOOT2_HEADER_MAGIC),
        .tag0 = {
                .type = 2,
                .flags = 0,
                .size = 24,
        },
        .header_addr = (uint32_t)&mb_header_start,
        .load_addr = 0x100000,
        .load_end_addr = (uint32_t)&kdata_end,
        .bss_end_addr = (uint32_t)&kbss_end,
        .tag1 = {
                .type = 3,
                .flags = 0,
                .size = 12,
        },
        .entry = (uint32_t)&kmain,
        .term = {
                .type = 0,
                .flags = 0,
                .size = 8
        }
};


#include "pe_mem.h"
#include "kacpi.h"

__attribute__((aligned(64)))
seg_desc_t gdt_descs[5] = {0};

__attribute__((aligned(64))) gdt_t gdt;


void dump32(uint32_t v)
{
        char *vid = (char *)0xB8000 + 160;

        uint8_t c;
        const int shifts[6] = {28, 24, 20, 16, 12, 8, 4, 0};

        for (int j = 0; j < 8; j++) {
                c = ((v >> shifts[j]) & 15);
                if (c > 9) c += 'A' - 10; else c += '0';
                *vid = c; vid += 2;
        }

}


void kmain(void)
{
        char *v = (char *)0xB8000;
        *v = 'R';

        init_seg_desc(&gdt_descs[1], 0, 0xFFFFF, 0x9A, 0x0C);
        init_seg_desc(&gdt_descs[2], 0, 0xFFFFF, 0x92, 0x0C);

        init_seg_desc(&gdt_descs[3], 0, 0xFFFFF, 0xFA, 0x0C);
        init_seg_desc(&gdt_descs[4], 0, 0xFFFFF, 0xF2, 0x0C);

        gdt.base = (uintptr_t)gdt_descs;
        gdt.lim = sizeof(gdt_descs);

        asm (
                "lgdt [gdt]\n"
                "mov ax, 0x10\n"
                "mov ds, ax\n"
                "mov es, ax\n"
                "mov ss, ax\n"
                "mov esp, 0x400000\n"
                "mov es, ax\n"
                "mov fs, ax\n"
                "mov gs, ax\n"
                "ljmp 8:1f\n"
                "1:\n"
        );


        void *rsdp = find_rsdp();

        dump32((uintptr_t)rsdp);

        while (1) {};
}
