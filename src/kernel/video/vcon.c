#include "vcon.h"
#include "../fonts/psf2.h"

#include <stdarg.h>
#include <stdbool.h>


void vcon_init(vcon_t *vcon, gc_t *gc, int16_t winx, int16_t winy)
{
        vcon->gc = gc;
        vcon->rows = gc->width / 16;
        vcon->cols = gc->width / 8;

        vcon->row = 0; /* relative */
        vcon->col = 0;

        vcon->fc = 0x7F7F7F;
        vcon->bc = 0x000000;

        vcon->winx = winx;
        vcon->winy = winy;
}


static void vcon_home(vcon_t *vcon)
{
        vcon->col = 0;
}


static void vcon_newline(vcon_t *vcon)
{
        vcon->row++;
        vcon->col = 0;

        gc_update_fb(vcon->gc, vcon->winx, vcon->winy);
}


void vcon_putc(vcon_t *vcon, unsigned char c)
{
        if (c == '\n') {
                vcon_newline(vcon);
                return;
        }

        if (c == '\r') {
                vcon_home(vcon);
                return;
        }

        uint32_t col, row;
        extern psf2_header_t *console_font;
        col = vcon->col;
        row = vcon->row;

        /* store minimum changed x and y into invalidate coords */
        if (vcon->gc->mincx < 0 || col * 8 < vcon->gc->mincx) {
                vcon->gc->mincx = col * 8;
        }
        if (vcon->gc->mincy < 0 || row * 16 < vcon->gc->mincy) {
                vcon->gc->mincy = row * 16;
        }

        uint8_t *glyph_start = (uint8_t *)console_font + 16 * (uint32_t)c;

        uint8_t *gp = glyph_start;
        for (uint32_t py = row * 16; py < (row + 1) * 16; py++) {

                uint32_t px = col * 8;
                uint32_t *paddr = (uint32_t *)vcon->gc->gmem;
                paddr += py * (vcon->gc->vpitch >> 2) + px;

                uint8_t mask = 0x80;
                for (int i = 0; i < 8; i++) {
                        if (*gp & mask) {
                                *(paddr + i) = vcon->fc;
                        } else {
                                *(paddr + i) = vcon->bc;
                        }
                        mask >>= 1;
                }
                gp++;
        }

        vcon->col++;
        if (vcon->col >= vcon->cols) {
                vcon->col = 0;
                vcon->row++;
        }

        if (vcon->col == 0) {
                vcon->gc->maxcx = vcon->cols * 8 - 1;
        } else if (vcon->col * 8 - 1 > vcon->gc->maxcx) {
                vcon->gc->maxcx = vcon->col * 8 - 1;
        }

        if (vcon->row * 16 - 1 > vcon->gc->maxcy) {
                vcon->gc->maxcy = vcon->row * 16 - 1;
        }
}


void vcon_puts(vcon_t *vcon, char *s)
{
        while (*s) {
                vcon_putc(vcon, *s);
                s++;
        }
}


void vcon_printf_p(vcon_t *vcon, uint32_t p)
{
        uint8_t c;
        const int shifts[8] = {28, 24, 20, 16, 12, 8, 4, 0};

        for (int j = 0; j < 8; j++) {
                c = ((p >> shifts[j]) & 15);
                if (c > 9) c += 'A' - 10; else c += '0';
                vcon_putc(vcon, (unsigned char)c);
        }
}


void vcon_printf(vcon_t *vcon, const char *format, ...)
{
        va_list args;
        va_start(args, format);

        bool in_specifier = false;

        if (!vcon || !format) {
                return;
        }

        while (*format) {
                if (*format != '%') {
                        if (in_specifier) {
                                switch (*format) {
                                case 'p':
                                        vcon_printf_p(
                                                vcon, va_arg(args, uint32_t));
                                        break;
                                default:

                                        break;
                                }
                                in_specifier = false;
                        } else {
                                vcon_putc(vcon, *format);
                        }
                } else {
                        if (in_specifier) {
                                // we have %%
                                vcon_putc(vcon, *format);
                        }

                        in_specifier = !in_specifier;
                }

                format++;
        }

        va_end(args);
}


void vcon_color(vcon_t *vcon, uint32_t fc, uint32_t bc)
{
       vcon->fc = fc;
       vcon->bc = bc;
}


void vcon_clear(vcon_t *vcon)
{
        gc_clear(vcon->gc, vcon->bc);
        gc_update_fb(vcon->gc, vcon->winx, vcon->winy);
}
