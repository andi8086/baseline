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
        .load_addr = 0x10000,
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

void kmain(void)
{
        char *v = (char *)0xB8000;
        *v = 'R';

        while (1) {};
}
