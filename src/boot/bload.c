#include "int86.h"
#include "conio.h"
#include "vesa.h"
#include "dev.h"
#include "bda.h"
#include "c_printf.h"


int main(void);
uint16_t read_far16(uint16_t seg, uint16_t offs);
uint16_t cpu_detect(void);

uint8_t boot_drive;



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
        char bufr[128];

        res = dev_init();

        puts((char *)hello_msg);

        cpu = cpu_detect();

        sys_get_equipment();


/*        res = vesa_init();
        if (res) {
                puts("VESA init failed\r\n");
                while (1);
        }
*/

//        mode = vesa_find_mode(640, 480, 24, &fblo, &fbhi);

        // vararg_test(4, 16384u, 32768u, 65535u, 1024u);
        puts("Boot drive is ");
        putc(boot_drive + 'A');
        puts(":\r\n");
//       puts("VESA mode found: ");
//       dump16(mode);
        puts("\r\n");
//        res = vesa_mode_set(mode);

        //c_snprintf(bufr, 128, "%08lu\r\n", 1048576);
        c_snprintf(bufr, 127, "%'+8.10d\r\n", -32767);
        puts((char *)bufr);

        puts("AA\r\n");

kernel_halt:
        goto kernel_halt;

        return 0;
}

