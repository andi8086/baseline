#include "pe_mem.h"


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

Hence, in order to map the whole 4G of memory,
we need 1024 page directory entries, because
the page directory index has 10 bits.

Every page directory entry is 32 bits wide,
meaning, we need 4 KB of memory for it.
*/

uint32_t page_directory[1024] __attribute__((aligned(4096)));

/* Our kernel runs at 0x100000, i.e. 1 MB hence
   to be able to turn paging on, we need to initialize
   the corresponding page directory entries with page tables
   with identity mapping */

/* 3 MB of kernel memory need 768 pages, the first 256 are
   for the lower 1 MB (VM86 if needed) */

uint32_t page_table_kernel[1024] __attribute__((aligned(4096)));

#define MEM_PAGE_PRESENT 1


void page_table_init(void)
{
        /* be sure all page dir entries are 0, i.e.
           no page table is present */
        for (int i = 0; i < 1024; i++) {
                page_directory[i] = 0;
        }

        /* for an address space of 0x0000 0000 - 0x003F FFFF
           we need 1 page table a 1024 page table entries a 4K pages */
        page_directory[0] = (uint32_t)(uintptr_t)page_table_kernel;
        page_directory[0] |= MEM_PAGE_PRESENT;

        /* initialize the kernel page table, and put in the addresses
           0x00000000 - 0x003FF000
        */
        for (uint32_t i = 0; i < 1024; i++) {
                page_table_kernel[i] = (i << 12);
                page_table_kernel[i] |= MEM_PAGE_PRESENT;
        }

        /* load CR3 register and enable paging */
        asm (
                "mov eax, offset page_directory\n"
                "mov cr3, eax\n"
                "mov eax, cr0\n"
                "or eax, 0x80000000\n"
                "mov cr0, eax\n"
        );


}

