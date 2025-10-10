#ifndef KPRINTF_H
#define KPRINTF_H


#include <uacpi/uacpi.h>
#include "video/vcon.h"


uacpi_i32 uacpi_vsnprintf(
    uacpi_char *buffer, uacpi_size capacity, const uacpi_char *fmt,
    uacpi_va_list vlist
);

uacpi_i32 uacpi_snprintf(
    uacpi_char *buffer, uacpi_size capacity, const uacpi_char *fmt, ...
);


extern vcon_t boot_console;

void kprintf(char *fmt, ...);
void kcolor(uint32_t fc, uint32_t bg);


#endif
