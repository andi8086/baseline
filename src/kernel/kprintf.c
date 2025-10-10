#include "kprintf.h"

#include <uacpi/uacpi.h>
#include "video/vcon.h"
#include "video/gc.h"


void kprintf(char *fmt, ...)
{
        char buffer[80];
        va_list l, l2;
        va_start(l, fmt);
        va_copy(l2, l);
        uacpi_vsnprintf(buffer, 80, fmt, l2);
        va_end(l2);

        vcon_printf(&boot_console, buffer);
        gc_update_fb(boot_console.gc, 64, 64);
}

