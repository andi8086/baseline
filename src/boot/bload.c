#include "int86.h"
#include "conio.h"
#include "vesa.h"
#include "dev.h"


int main(void);
uint16_t read_far16(uint16_t seg, uint16_t offs);
uint16_t cpu_detect(void);

uint8_t boot_drive;


#define CPU_TYPE_8086  0
#define CPU_TYPE_V20   1
#define CPU_TYPE_80186 2
#define CPU_TYPE_80286 3
#define CPU_TYPE_80386 4


void _cstart(void)
{
        __asm {
                mov ax, 70h
                mov ds, ax
                mov es, ax
                mov ss, ax
                mov sp, 0FFFFh
                mov boot_drive, dl
        }
        main();
}


const char *hello_msg = "Baseline, v0.1\r\n(C)2025 by Andreas J. Reichel\r\n";



int main(void)
{
        int res;
        uint16_t mode, fblo, fbhi;
        uint16_t cpu;

        res = dev_init();

        puts(hello_msg);

        cpu = cpu_detect();

        switch (cpu) {
        case CPU_TYPE_8086:
                puts("CPU: 8086/8088\r\n");
                break;
        case CPU_TYPE_V20:
                puts("CPU: NEC V20/V30\r\n");
                break;
        case CPU_TYPE_80186:
                puts("CPU: 80186/80188\r\n");
                break;
        case CPU_TYPE_80286:
                puts("CPU: 80286\r\n");
                break;
        case CPU_TYPE_80386:
                puts("CPU: i386\r\n");
                break;
        default:
                puts("CPU: unknown\r\n");
                break;
        }


/*        res = vesa_init();
        if (res) {
                puts("VESA init failed\r\n");
                while (1);
        }
*/

        mode = vesa_find_mode(640, 480, 24, &fblo, &fbhi);

        // vararg_test(4, 16384u, 32768u, 65535u, 1024u);
        puts("Boot drive is ");
        putc(boot_drive + 'A');
        puts(":\r\n");
        puts("VESA mode found: ");
        dump16(mode);
        puts("\r\n");
//        res = vesa_mode_set(mode);


kernel_halt:
        goto kernel_halt;

        return 0;
}

