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
