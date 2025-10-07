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
