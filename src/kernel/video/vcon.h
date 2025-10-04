#ifndef VCON_H
#define VCON_H

#include <stdint.h>
#include "gc.h"

typedef struct {
        uint32_t fc;
        uint32_t bc;
        uint32_t rows;
        uint32_t cols;
        uint32_t row;
        uint32_t col;
        gc_t *gc;
} vcon_t;


void vcon_init(vcon_t *vcon, gc_t *gc);
void vcon_puts(vcon_t *vcon, char *s);
void vcon_putc(vcon_t *vcon, unsigned char c);
void vcon_printf(vcon_t *vcon, const char *format, ...);

#endif
