#ifndef BDA_H
#define BDA_H

#include <stdint.h>

typedef struct {
        uint16_t boot_floppy_drive : 1;
        uint16_t math_coproc       : 1;
        uint16_t ps2_mouse         : 1;
        uint16_t reserved          : 1;
        uint16_t graphics_card     : 2;
        uint16_t floppy_drives     : 2;
        uint16_t dma_controller    : 1;
        uint16_t num_com_ports     : 3;
        uint16_t have_joystick     : 1;
        uint16_t have_modem        : 1;
        uint16_t num_lpt_ports     : 2;
} equipment_word_t;


#define COM_IO_ADDR(n)     (0x00400000 + 2*n)
#define LPT_IO_ADDR(n)     (0x00400008 + 2*n)

#define EQUIPMENT_WORD_ADDR 0x00400010


#endif
