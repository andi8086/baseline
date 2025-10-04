#ifndef PE_MEM_H
#define PE_MEM_H

#include <stdint.h>
#include "heapm32.h"

#pragma pack(push, 1)
typedef struct {
        uint16_t lim_15_0;
        uint16_t base_15_0;
        uint8_t base_23_16;
        uint8_t access;
        uint8_t lim_19_16_flags;
        uint8_t base_31_24;
} seg_desc_t;
#pragma pack(pop)

#define PAGE_SIZE 4096UL


#define SEG_DESC_ACC_PRESENT    0x80
#define SEG_DESC_ACC_DPL(x)     ((x) << 5)
#define SEG_DESC_ACC_CODE       0x10
#define SEG_DESC_ACC_DATA       0x10
#define SEG_DESC_ACC_SYS        0x00

#define SEG_DESC_ACC_EXEC       0x08
#define SEG_DESC_ACC_DATA_DIR   0x04
#define SEG_DESC_ACC_EXEC_CONF  0x04

#define SEG_DESC_ACC_CODE_READ  0x02
#define SEG_DESC_ACC_DATA_WRITE 0x02

#define SEG_DESC_ACC_ACCESSED   0x01

#define SEG_DESC_FLAGS_GRAN     8
#define SEG_DESC_FLAGS_SIZE32   4
#define SEG_DESC_FLAGS_SIZE16   0
#define SEG_DESC_FLAGS_SIZE64   2

#define SEG_DESC_ACC_SYS_TSS16_AVAIL    1
#define SEG_DESC_ACC_SYS_TSS16_BUSY     3
#define SEG_DESC_ACC_SYS_TSS32_AVAIL    9
#define SEG_DESC_ACC_SYS_TSS32_BUSY     0xB
#define SEG_DESC_ACC_SYS_LDT            2
#define SEG_DESC_ACC_SYS_TSS64_AVAIL    9
#define SEG_DESC_ACC_SYS_TSS64_BUSY     0xB


void init_seg_desc(seg_desc_t *seg,
                   uint32_t base,
                   uint32_t limit,
                   uint8_t access,
                   uint8_t flags);


typedef struct {
        uint16_t lim;
        uint32_t base;
} gdt_t;


void page_table_init(void);


typedef enum {
        ARENA_LOW,
        ARENA_HIGH
} arena_type_t;


typedef struct {
        arena_type_t type;
        uint32_t base;
        uint32_t size;
        hm_ctx_t hm_ctx;
} kmem_arena_t;


void kmem_arena_add(uint32_t base, uint32_t size);


#define MEM_PAGE_PRESENT 1

#define PT_ENTRIES 1024
#define KERNEL_PAGE_DIR_ADDR ((1UL << 22) - PAGE_SIZE) // @ 4 MB physical address
#define KERNEL_PAGE_TABLE_ADDR (KERNEL_PAGE_DIR_ADDR + PAGE_SIZE)
#define KERNEL_PAGE_TABLE_END (2UL << 22)
#define KERNEL_STACK_END      ((2UL << 22) + (1UL << 20))


void *kmalloc_high(uint32_t size);
void kfree(void *p);

#endif
