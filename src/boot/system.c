#include "system.h"
#include "bda.h"
#include "conio.h"
#include "int86.h"


static equipment_t equipment;


extern uint16_t cpu_detect(void);


void sys_get_equipment(equipment_t **e)
{
        equipment_word_t far *ew =
                (equipment_word_t far *)BDA_EQUIPMENT_WORD_ADDR;
        regs86_t rin, rout;

        int i;

        equipment.cpu_type = cpu_detect();

        equipment.num_com_ports = ew->num_com_ports;
        equipment.num_lpt_ports = ew->num_lpt_ports;

        equipment.num_floppies = ew->floppy_drives;

//      This won't work on a system older then 286/AT
//
//        equipment.num_hdds = hdds;
//
//      instead int 13 will return number of drives
//      in dl, if input dl was >= 80h. Later systems
//      can also do this for floppy drives, but simulation
//      of a pentium II system with 86Box shows it works

        equipment.num_hdds = 0;

        rin._ax = 0x0800;
        rin._dx = 0x0080;
        disk_int86(&rin, &rout);

        if (!(rout._flags & FLAGS_CARRY)) {
                equipment.num_hdds = rout._dx & 0xFF;
        }


        for (i = 0; i < equipment.num_com_ports; i++) {
                equipment.com_io_addr[i] = *(uint16_t far *)COM_IO_ADDR(i);
        }

        for (i = 0; i < equipment.num_lpt_ports; i++) {
                equipment.lpt_io_addr[i] = *(uint16_t far *)LPT_IO_ADDR(i);
        }


        switch (equipment.cpu_type) {
        case CPU_TYPE_8086: printf("CPU: 8086/8088\r\n"); break;
        case CPU_TYPE_V20: printf("CPU: NEC V20/V30\r\n"); break;
        case CPU_TYPE_80186: printf("CPU: 80186/80188\r\n"); break;
        case CPU_TYPE_80286: printf("CPU: 80286\r\n"); break;
        case CPU_TYPE_80386: printf("CPU: 80386\r\n"); break;
        default: break;
        }


        puts("COM Ports: ");
        for (i = 0; i < equipment.num_com_ports; i++) {
                printf("COM%u at %03Xh", i + 1, equipment.com_io_addr[i]);
                if (i < equipment.num_com_ports - 1) {
                        putc(',');
                        putc(' ');
                }
        }
        puts("\r\n");

        puts("LPT Ports: ");
        for (i = 0; i < equipment.num_lpt_ports; i++) {
                printf("LPT%u at %03Xh", i + 1, equipment.lpt_io_addr[i]);
                if (i < equipment.num_lpt_ports - 1) {
                        putc(',');
                        putc(' ');
                }
        }
        puts("\r\n");

        printf("%u FDDs reported by BIOS\r\n", equipment.num_floppies);
        printf("%u HDDs reported by BIOS\r\n", equipment.num_hdds);

        *e = &equipment;
}
