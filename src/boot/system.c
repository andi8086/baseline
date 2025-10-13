#include "system.h"
#include "bda.h"
#include "conio.h"


static equipment_t equipment;

void sys_get_equipment(void)
{
        equipment_word_t far *ew =
                (equipment_word_t far *)EQUIPMENT_WORD_ADDR;
        int i;

        equipment.num_com_ports = ew->num_com_ports;
        equipment.num_lpt_ports = ew->num_lpt_ports;
        equipment.num_floppies = ew->floppy_drives;

        for (i = 0; i < equipment.num_com_ports; i++) {
                equipment.com_io_addr[i] = *(uint16_t far *)COM_IO_ADDR(i);
        }

        for (i = 0; i < equipment.num_lpt_ports; i++) {
                equipment.lpt_io_addr[i] = *(uint16_t far *)LPT_IO_ADDR(i);
        }

        puts("COM Ports: ");
        for (i = 0; i < equipment.num_com_ports; i++) {
                dump16(equipment.com_io_addr[i]);
        }
        puts("\r\n");

        puts("LPT Ports: ");
        for (i = 0; i < equipment.num_lpt_ports; i++) {
                dump16(equipment.lpt_io_addr[i]);
        }
        puts("\r\n");

        puts("Floppy Drives: ");
        for (i = 0; i < equipment.num_floppies; i++) {
                putc('A' + i);
                putc(':');
                putc(' ');
        }

        puts("\r\n");
}
