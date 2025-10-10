#include "kint.h"

#include "video/vcon.h"


idt_entry_t idt[256] __attribute__((aligned(16)));
idt_t idtptr __attribute__((aligned(16)));


void idt_entry_init(idt_entry_t *e, uint16_t seg, uint32_t offset,
                    idt_type_t type, uint8_t dpl)
{
        switch (type) {
        case IDT_TRAP:
                e->type = IGATE_PRESENT | (dpl << 5) |
                          IGATE_TYPE_TRAP32;
                break;
        default:
                e->type = IGATE_PRESENT | (dpl << 5) |
                          IGATE_TYPE_INT32;
                break;
        }

        e->offset_lo = offset & UINT16_MAX;
        e->offset_hi = offset >> 16;
        e->seg = seg;
}


extern vcon_t boot_console;


void _int_handler(int_frame_t frame)
{
        asm("cli\nhlt\n");
}


void _trap_handler(int_frame_t frame)
{
        boot_console.fc = 0xFF0000;
        vcon_printf(&boot_console, "Fault\n");
        gc_update_fb(boot_console.gc, 64, 64);

        asm("cli\nhlt\n");
}


void _trap_handler_code(trap_frame_code_t frame)
{
        boot_console.fc = 0xFFFF00;
        boot_console.bc = 0x0000FF;
        vcon_printf(&boot_console, "\n");
        vcon_printf(&boot_console, "Exception at CS:EIP = %p:%p\n",
                    frame.cs, frame.eip);
        vcon_printf(&boot_console, "code: %p, eflags %p\n", (uint32_t)frame.code,
                frame.eflags);
        vcon_printf(&boot_console, "EAX = %p     EBX = %p\n", frame.eax, frame.ebx);
        vcon_printf(&boot_console, "ECX = %p     EDX = %p\n", frame.ecx, frame.edx);
        vcon_printf(&boot_console, "ESI = %p     EDI = %p\n", frame.esi, frame.edi);
        vcon_printf(&boot_console, "EBP = %p     ESP = %p\n", frame.ebp, frame.esp);
        vcon_printf(&boot_console, "\n");
        gc_update_fb(boot_console.gc, 64, 64);
        asm("cli\nhlt\n");
}


/* stubs defined in isr.S, call functios with underscore */
const uintptr_t irq_handlers[16] = {
        (uintptr_t)irq0_handler,
        (uintptr_t)irq1_handler,
        (uintptr_t)irq2_handler,
        (uintptr_t)irq3_handler,
        (uintptr_t)irq4_handler,
        (uintptr_t)irq5_handler,
        (uintptr_t)irq6_handler,
        (uintptr_t)irq7_handler,
        (uintptr_t)irq8_handler,
        (uintptr_t)irq9_handler,
        (uintptr_t)irq10_handler,
        (uintptr_t)irq11_handler,
        (uintptr_t)irq12_handler,
        (uintptr_t)irq13_handler,
        (uintptr_t)irq14_handler,
        (uintptr_t)irq15_handler
};

#define IRQ_CB_MAX 4
void (*irq_isr_cb[16][IRQ_CB_MAX])(void) = { 0 };

#define CALL_ISR_CB(irq) \
        for (int i = 0; i < IRQ_CB_MAX; i++) { \
                if (irq_isr_cb[irq][i]) { \
                        irq_isr_cb[irq][i](); \
                } \
        } \


bool irq_handler_register(int irq, uint32_t func)
{
        for (int i = 0; i < IRQ_CB_MAX; i++) {
                if (!irq_isr_cb[irq][i]) {
                        irq_isr_cb[irq][i] = (void *)func;
                        return true;
                }
        }
        return false;
}


void _irq0_handler(int_frame_t frame)
{
        CALL_ISR_CB(0);
}


void _irq1_handler(int_frame_t frame)
{
        CALL_ISR_CB(1);
}


void _irq2_handler(int_frame_t frame)
{
        CALL_ISR_CB(2);
}


void _irq3_handler(int_frame_t frame)
{
        CALL_ISR_CB(3);
}


void _irq4_handler(int_frame_t frame)
{
        CALL_ISR_CB(4);
}


void _irq5_handler(int_frame_t frame)
{
        CALL_ISR_CB(5);
}


void _irq6_handler(int_frame_t frame)
{
        CALL_ISR_CB(6);
}


void _irq7_handler(int_frame_t frame)
{
        CALL_ISR_CB(7);
}


void _irq8_handler(int_frame_t frame)
{
        CALL_ISR_CB(8);
}


void _irq9_handler(int_frame_t frame)
{
        CALL_ISR_CB(9);
}


void _irq10_handler(int_frame_t frame)
{
        CALL_ISR_CB(10);
}


void _irq11_handler(int_frame_t frame)
{
        CALL_ISR_CB(11);
}


void _irq12_handler(int_frame_t frame)
{
        CALL_ISR_CB(12);
}


void _irq13_handler(int_frame_t frame)
{
        CALL_ISR_CB(13);
}


void _irq14_handler(int_frame_t frame)
{
        CALL_ISR_CB(14);
}


void _irq15_handler(int_frame_t frame)
{
        CALL_ISR_CB(15);
}


void idt_init(void)
{
        idtptr.ptr = (uintptr_t)idt;
        idtptr.size = sizeof(idt) - 1;

        idt_entry_init(&idt[0], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[1], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[2], 0x08, (uintptr_t)&int_handler, IDT_INT, 0);
        idt_entry_init(&idt[3], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[4], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[5], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[6], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[7], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[8], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[9], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[0x0A], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[0x0B], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[0x0C], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[0x0D], 0x08, (uintptr_t)&trap_handler_code, IDT_TRAP, 0);
        idt_entry_init(&idt[0x0E], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[0x10], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[0x11], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[0x12], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[0x13], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[0x14], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);
        idt_entry_init(&idt[0x15], 0x08, (uintptr_t)&trap_handler, IDT_TRAP, 0);

        for (int irq = 0; irq < 0x0F; irq++) {
                idt_entry_init(&idt[0x20 + irq], 0x08,
                        (uintptr_t)irq_handlers[irq], IDT_INT, 0);

        }

        asm (
                "mov eax, offset idtptr\n"
                "lidt [eax]\n"
               /* "mov eax, -1\n"
                "mov ebx, 0xAAAAAAAA\n"
                "mov ecx, 0xBBBBBBBB\n"
                "mov edx, 0xCCCCCCCC\n"
                "mov esi, 0xDDDDDDDD\n"
                "mov edi, 0xEEEEEEEE\n"
                "mov ax, 0x40\n"
                "sub ax, 0x40\n"
                "mov fs, bx\n" // provoke exception
                "mov byte ptr fs:[0], al\n" */
        );
}
