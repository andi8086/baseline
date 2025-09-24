#ifndef VCON_H
#define VCON_H

#include <stdint.h>
#include "fb.h"

typedef struct {
        uint32_t width;
        uint32_t height;
        uint32_t start_addr;
        uint32_t pitch;
        uint32_t fc;
        uint32_t bc;
        uint32_t row;
        uint32_t col;
        v_framebuffer_t *fb; /* if set, use pitch of fb, to directly display */
} vcon_t;


void vcon_init(vcon_t *vcon, uint32_t addr, uint32_t width, uint32_t height);
void vcon_puts(vcon_t *vcon, char *s);
void vcon_set_fb(vcon_t *vcon, v_framebuffer_t *fb);
void vcon_putc(vcon_t *vcon, unsigned char c);
void vcon_printf(vcon_t *vcon, const char *format, ...);

#endif
