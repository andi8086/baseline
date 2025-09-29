#include "pe_mem.h"

#include "video/fb.h"
#include "video/vcon.h"
#include "ksmp_apic.h"

void init_seg_desc(seg_desc_t *seg,
                   uint32_t base,
                   uint32_t limit,
                   uint8_t access,
                   uint8_t flags)
{
        seg->access = access;
        seg->base_15_0 = base & 0xFFFF;
        seg->base_23_16 = base >> 16;
        seg->lim_15_0 = limit & 0xFFFF;
        seg->lim_19_16_flags =
                ((limit >> 16) & 0xF) | (flags << 4);
        seg->base_31_24 = base >> 24;
}


/* linear address in 32-bit protected mode is

   31       22 21       12 11           0
     pdir         ptbl          offset
     index        index

Hence, in order to map the whole 4G of memory, we need 1024 page directory
entries, because the page directory index has 10 bits.

Every page directory entry is 32 bits wide, meaning, we need 4 KB of memory for
it.
*/


uint32_t *page_dir;

void page_table_init(void)
{
        /* we assume 8 MB of RAM minimum and put the page directory
           and all kernel page tables in the 4 MB following the kernel */
        page_dir = (uint32_t *)KERNEL_PAGE_DIR_ADDR;
        uint32_t *page_tables = (uint32_t *)KERNEL_PAGE_TABLE_ADDR;

        /* perform identity mapping of whole 4G address space */
        for (uint32_t pd_idx = 0; pd_idx < PT_ENTRIES; pd_idx++) {
                page_dir[pd_idx] = (uint32_t)(page_tables + pd_idx * PT_ENTRIES);
                page_dir[pd_idx] |= 1;

                uint32_t *pt = page_tables + pd_idx * PT_ENTRIES;
                for (uint32_t pt_idx = 0; pt_idx < PT_ENTRIES; pt_idx++) {
                        pt[pt_idx] = (pd_idx << 22) | (pt_idx << 12) | 1;
                }
        }

        /* load CR3 register and enable paging */
        asm (
                "mov eax, page_dir\n"
                "mov cr3, eax\n"
                "mov eax, cr0\n"
                "or eax, 0x80000000\n"
                "mov cr0, eax\n"
        );

        return;
}


#define KMEM_ARENA_MAX 16

static int kmem_n_arenas = 0;
static kmem_arena_t kmem_arenas[KMEM_ARENA_MAX];


void kmem_arena_add(uint32_t base, uint32_t size)
{
        if (kmem_n_arenas == KMEM_ARENA_MAX) {
                return;
        }

        extern char mb_header_start;
        /* check if chunk includes the kernel */
        if (base < (uintptr_t)&mb_header_start &&
            base + size > KERNEL_STACK_END) {
                if (size <= KERNEL_STACK_END - base) {
                        return;
                }
                size = size - (KERNEL_STACK_END - base);
                base = KERNEL_STACK_END;
        }

        extern vcon_t boot_console;
        vcon_printf(&boot_console, "\nkmem_arena: added arena from %p to %p", base, base + size);

        kmem_arena_t *kmt = &kmem_arenas[kmem_n_arenas];

        kmt->base = base;
        kmt->size = size;

        if (base < (1UL << 20)) {
                kmt->type = ARENA_LOW;
        } else {
                kmt->type = ARENA_HIGH;
        }

        /* initialize heap manager for arena */
        hm_init(&kmt->hm_ctx, (void *)base, size);

        kmem_n_arenas++;
}

