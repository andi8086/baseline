#ifndef KINT_H
#define KINT_H


#include <stdint.h>
#include <stdbool.h>


#pragma pack(push, 1)
typedef struct {
        uint16_t offset_lo;
        uint16_t seg;
        uint8_t res_;
        uint8_t type;
        uint16_t offset_hi;
} idt_entry_t;


typedef struct {
        uint16_t size;
        uint32_t ptr;
} idt_t;


typedef struct {
        uint32_t edi;
        uint32_t esi;
        uint32_t ebp;
        uint32_t esp;
        uint32_t ebx;
        uint32_t edx;
        uint32_t ecx;
        uint32_t eax;
        uint32_t code;
        uint32_t eip;
        uint16_t cs;
        uint16_t res_;
        uint32_t eflags;
} trap_frame_code_t;


typedef struct {
        uint32_t edi;
        uint32_t esi;
        uint32_t ebp;
        uint32_t esp;
        uint32_t ebx;
        uint32_t edx;
        uint32_t ecx;
        uint32_t eax;
        uint32_t eip;
        uint16_t cs;
        uint16_t res_;
        uint32_t eflags;
} int_frame_t;
#pragma pack(pop)


#define IGATE_PRESENT 0x80
#define IGATE_TYPE_TRAP32 0xF
#define IGATE_TYPE_INT32  0xE
#define IGATE_TYPE_TRAP16 0x7
#define IGATE_TYPE_INT16  0x6


typedef enum {
        IDT_TRAP,
        IDT_INT
} idt_type_t;


void idt_entry_init(idt_entry_t *e, uint16_t seg, uint32_t offset,
                    idt_type_t typem, uint8_t dpl);
void idt_init(void);

extern void trap_handler(void);
extern void trap_handler_code(void);
extern void int_handler(void);

extern void irq0_handler(void);
extern void irq1_handler(void);
extern void irq2_handler(void);
extern void irq3_handler(void);
extern void irq4_handler(void);
extern void irq5_handler(void);
extern void irq6_handler(void);
extern void irq7_handler(void);
extern void irq8_handler(void);
extern void irq9_handler(void);
extern void irq10_handler(void);
extern void irq11_handler(void);
extern void irq12_handler(void);
extern void irq13_handler(void);
extern void irq14_handler(void);
extern void irq15_handler(void);

bool irq_handler_register(int irq, uint32_t func);


#endif
