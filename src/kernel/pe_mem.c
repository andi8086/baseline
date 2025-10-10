#include "pe_mem.h"
#include "video/fb.h"

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

#define PAGE_PRESENT 1

#define PAGE_PAT 0x80
#define PAGE_PCD 0x10
#define PAGE_PWT 0x08


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
                        uint32_t linear_addr = (pd_idx << 22) | (pt_idx << 12);
                        pt[pt_idx] = linear_addr | PAGE_PRESENT;
                        if (linear_addr >= (uintptr_t)vfb.addr &&
                            linear_addr < (uintptr_t)vfb.addr +
                                                vfb.height * vfb.pitch) {
                                /* enable write combine caching for
                                   frame buffer */
                                pt[pt_idx] |= PAGE_PCD |  PAGE_PWT;
                        }
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

        uint32_t a_start = base;
        uint32_t a_end = base + size;
        uint32_t k_end = KERNEL_STACK_END;
        uint32_t k_start = (uintptr_t)&mb_header_start;

        /* check if chunk includes the kernel */
        if (a_start <= k_start && a_end > k_end) {
                if (size <= k_end - a_start) {
                        return;
                }
                size = size - (k_end - a_start);
                base = k_end;
        }

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


void *kmalloc_high(uint32_t size)
{
        /* find heapm32 context for high arena */
        for (int i = 0; i < kmem_n_arenas; i++) {
                kmem_arena_t *a = (kmem_arena_t *)&kmem_arenas[i];
                if (a->type == ARENA_HIGH) {
                        void *p = hm_alloc(&a->hm_ctx, size);
                        if (p) {
                                return p;
                        }
                }
        }
        return NULL;
}


void kfree(void *p)
{
        /* FIXME */
        return;
        uintptr_t paddr = (uintptr_t)p;
        /* we must find the pointers heap context */
        for (int i = 0; i < kmem_n_arenas; i++) {
                kmem_arena_t *a = (kmem_arena_t *)&kmem_arenas[i];
                if (paddr > (uintptr_t)a->hm_ctx.mem_start &&
                    paddr <= ((uintptr_t)a->hm_ctx.mem_start +
                             (uintptr_t)a->hm_ctx.mem_size)) {
                        hm_free(&a->hm_ctx, p);
                        return;
                }
        }
}


void msr_pat_get(uint32_t *lo, uint32_t *hi)
{
        uint32_t msr = 0x277;
        asm volatile ("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}


void msr_pat_set(uint32_t lo, uint32_t hi)
{
        uint32_t msr = 0x277;
        asm volatile ("wrmsr" :: "a"(lo), "d"(hi), "c"(msr));
}


#define PAT_UC 0        /* uncachable */
#define PAT_WC 1        /* write combine */
#define PAT_WT 4        /* write through */
#define PAT_WP 5        /* write protect */
#define PAT_WB 6        /* write back */
#define PAT_UNC 7       /* uncached (UC-) */

void pat_init(void)
{
        /* modify entry #3 for write combine cache */
        uint32_t lo;
        uint32_t hi;
        msr_pat_get(&lo, &hi);

        /* modify the 3rd entry */
        lo &= 0xFFFFFF;
        lo |= PAT_WC << 24;
        msr_pat_set(lo, hi);
}
