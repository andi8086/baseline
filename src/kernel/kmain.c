#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6
#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289

#include <stdint.h>
#include "klib.h"

extern char mb_header_start;
extern char kdata_end;
extern char kbss_end;
extern char kernel_start;
extern char kernel_end;

void _start(void);
void kmain(uint32_t, uint32_t);

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
        mb_tag_header_t tag2;
        uint32_t width;
        uint32_t height;
        uint32_t bpp;
        uint32_t _dummy2;
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
        .entry = (uint32_t)&_start,
        .tag2 = {
                .type = 5,      /* request framebuffer */
                .flags = 0,     /* not optional ! */
                .size = 20
        },
        .width = 1280,
        .height = 1024,
        .bpp = 32,
        .term = {
                .type = 0,
                .flags = 0,
                .size = 8
        }
};


#include "pe_mem.h"
#include "kacpi.h"
#include "video/fb.h"
#include "video/vcon.h"
#include "fonts/psf2.h"
#include "ksmp_apic.h"
#include "kint.h"
#include "pic8259.h"
#include "ioapic.h"
#include "pci.h"
#include <uacpi/uacpi.h>
#include "kdev.h"
#include "kprintf.h"


extern psf2_header_t *console_font;
// extern char _binary_ATIEgaWonder800p_8x16_bin_start;
extern char _binary_TSVGA_ET4000_8x16_bin_start;


__attribute__((aligned(64)))
seg_desc_t gdt_descs[5] = {0};

__attribute__((aligned(64))) gdt_t gdt;


void dump32(uint32_t v)
{
        char *vid = (char *)0xB8000 + 160;

        uint8_t c;
        const int shifts[8] = {28, 24, 20, 16, 12, 8, 4, 0};

        for (int j = 0; j < 8; j++) {
                c = ((v >> shifts[j]) & 15);
                if (c > 9) c += 'A' - 10; else c += '0';
                *vid = c; vid += 2;
        }

}


__attribute__((naked)) void _start(void)
{
        asm (
                "pushd 0\n"
                "popf\n"
                "push ebx\n"
                "push eax\n"
                "call kmain\n"
                "1: hlt\n"
                "jmp 1b\n"
        );
}




#define MBI_TAG_FRAMEBUFFER 8

typedef struct {
        uint32_t type;
        uint32_t size;
        uint64_t fb_addr;
        uint32_t fb_pitch;
        uint32_t fb_width;
        uint32_t fb_height;
        uint8_t  fb_bpp;
} mb_fbi_t;

typedef struct {
        uint32_t type;
        uint32_t size;
} mb_tag_t;

uint16_t *mbi_size;
mb_tag_t *mbi_tags;

extern char _binary_logo_data_start;
extern char _binary_logo_data_end;

vcon_t boot_console;

uint32_t *kesp = (uint32_t *)KERNEL_STACK_END;
rsdp_header_t *rsdp; /* also needed for uACPI */

void kmain(uint32_t magic, uint32_t addr)
{
        char *v = (char *)0xB8000;
        *v = 'R';

        mbi_size = (uint16_t *)addr;
        mbi_tags = (mb_tag_t *)(addr + 8);

        /* check magic */
        if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
                return;
        }

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
                "mov esp, kesp\n"
                "mov es, ax\n"
                "mov fs, ax\n"
                "mov gs, ax\n"
                "ljmp 8:1f\n"
                "1:\n"
        );

        mb_tag_t *tag;
        mb_fbi_t *fbit;
        for (tag = mbi_tags; tag->type != 0;
             tag = (mb_tag_t *)((uint8_t *)tag +
                   ((tag->size + 7) & ~7))) {
                switch (tag->type) {
                case MBI_TAG_FRAMEBUFFER:
                        fbit = (mb_fbi_t *)tag;
                        video_init(fbit->fb_addr, fbit->fb_width,
                                   fbit->fb_height, fbit->fb_bpp,
                                   fbit->fb_pitch);
                        break;
                }
        }

        if (!vfb.addr) {
                /* error, no display */
                while (1);
        }

        console_font = (psf2_header_t *)&_binary_TSVGA_ET4000_8x16_bin_start;
        uint32_t *logo = (uint32_t *)&_binary_logo_data_start;
        uint32_t *logo_end = (uint32_t *)&_binary_logo_data_end;

        rsdp = kacpi_find_rsdp();

        if (rsdp) {
                rsdt_t *rsdt = (rsdt_t *)(uintptr_t)(rsdp->rsdt_addr);
                int entries = (rsdt->h.length - sizeof(rsdt->h)) / 4;
                void *madt = kacpi_find_madt(rsdp);
                if (madt) {
                        kacpi_madt_init(madt);
                }
        }

#define MBI_TAG_MEMORY_MAP 6
        typedef struct {
                uint32_t type;
                uint32_t size;
                uint32_t entry_size;
                uint32_t entry_version;
        } mbi_mem_map_t;

        typedef struct {
                uint64_t base_addr;
                uint64_t len;
                uint32_t type;
                uint32_t res;
        } mbi_mem_map_entry_t;

        mbi_mem_map_t *mem_map;
        mbi_mem_map_entry_t *mem_map_e;

        for (tag = mbi_tags; tag->type != 0;
             tag = (mb_tag_t *)((uint8_t *)tag +
                   ((tag->size + 7) & ~7))) {
                switch (tag->type) {
                case MBI_TAG_MEMORY_MAP:
                        mem_map = (mbi_mem_map_t *)tag;
                        mem_map_e = (mbi_mem_map_entry_t *)
                                ((uint8_t *)tag + sizeof(mbi_mem_map_t));
                        while ((uint8_t *)mem_map_e <
                               (uint8_t *)mem_map + mem_map->size) {

                                switch (mem_map_e->type) {
                                case 1: kmem_arena_add(mem_map_e->base_addr,
                                                       mem_map_e->len);
                                        break;
                                case 3: // ACPI info
                                        break;
                                case 4: // preserved
                                        break;
                                case 5: // defective
                                        break;
                                default: // reserved
                                        break;

                                }

                                mem_map_e = (mbi_mem_map_entry_t *)
                                        ((uint8_t *)mem_map_e + mem_map->entry_size);
                        }

                        break;
                }
        }

        pat_init();
        page_table_init();


        gc_t *gc = gc_create(640, 960);
        vcon_init(&boot_console, gc, 34, 21);
        vcon_color(&boot_console, 0xAAAAAA, 0x000055);
        vcon_clear(&boot_console);
        vcon_printf(&boot_console, "Starting Baseline...\n");
        vcon_printf(&boot_console, "Kernel 0.01\n");

/*        for (int y = 0; y < vfb.height; y++) {
                for (int x = 0; x < vfb.width; x++) {
                        video_putpixel(x, y, 0x000000);
                }
        }
*/
/*        int y = 127;
        int logo_width = 844;
        int xleft = 640 - logo_width/2, x = xleft;

        uint32_t *pixel_data = logo;
        while (pixel_data < logo_end) {
                if (((pixel_data - logo) % logo_width) == 0) {
                        y++;
                        x = xleft;
                }
                uint32_t r = *(uint8_t *)pixel_data;
                uint32_t g = *((uint8_t *)pixel_data + 1);
                uint32_t b = *((uint8_t *)pixel_data + 2);
                video_putpixel(x, y, (r << 16) | (g << 8) | b);
                x++;
                pixel_data++;
        }

        vcon_init(&boot_console,
                  (uint32_t)(vfb.addr + 384*1280),
                  1280, 640);
        vcon_puts(&boot_console, "Kernel v0.01\n");
*/
        uint8_t smp_cpus = cpu_wake_all();

//        vcon_printf(&boot_console, "%p CPUs running...\n", smp_cpus + 1);
        idt_init();

        /* first we move the IRQ handlers out of the way to not
           collide with CPU exception vectors */
        pic_init(0x20, 0x28);
        /* we disable all IRQs in the PIC because we want to use IOAPIC */
        pic_disable();

        /* configure IOAPIC to route IRQs 0-15 to the bootstrap CPU */
        for (int i = 0; i < 15; i++) {
                ioapic_irq_config(i, 0x20 + i, IOAPIC_IRQ_FIXED,
                                  IOAPIC_DEST_CPU_PHYS, IOAPIC_IRQ_ACTIVE_LOW,
                                  IOAPIC_TRIGGER_LEVEL, 0, 0);
        }

        uacpi_status ret = uacpi_initialize(0);
        if (uacpi_unlikely_error(ret)) {
                vcon_printf(&boot_console, uacpi_status_to_string(ret));
        }


        ret = uacpi_namespace_load();
        if (uacpi_unlikely_error(ret)) {
                vcon_printf(&boot_console, uacpi_status_to_string(ret));
        }

        ret = uacpi_namespace_initialize();
        if (uacpi_unlikely_error(ret)) {
                vcon_printf(&boot_console, uacpi_status_to_string(ret));
        }
//        uacpi_finalize_gpe_initialization();
        pci_init();
        uacpi_devices_enumerate();

        /* enable interrupts for testing */
        asm("sti\n");

        while (1) {};
}



void abort(void)
{
        while (1) {};
}
