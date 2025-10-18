#ifndef SYSTEM_H
#define SYSTEM_H


#include <stdint.h>


#define CPU_TYPE_8086  0
#define CPU_TYPE_V20   1
#define CPU_TYPE_80186 2
#define CPU_TYPE_80286 3
#define CPU_TYPE_80386 4


typedef struct {
        uint8_t cpu_type;
        uint8_t num_floppies;
        uint8_t num_hdds;
        uint8_t num_com_ports;
        uint8_t num_lpt_ports;
        uint16_t com_io_addr[4];
        uint16_t lpt_io_addr[3];
        uint8_t int13ex;
} equipment_t;


void sys_get_equipment(equipment_t **e);


#endif
