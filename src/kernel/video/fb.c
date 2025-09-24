#include "fb.h"
#include "../fonts/psf2.h"

v_framebuffer_t vfb;

psf2_header_t *console_font;


int video_init(uint32_t fb_addr, uint32_t w, uint32_t h, uint8_t bpp,
               uint32_t pitch)
{
        vfb.addr = (uint32_t *)fb_addr;
        vfb.width = w;
        vfb.height = h;
        vfb.bpp = bpp;
        vfb.pitch = pitch >> 2;
        return 0;
}


void video_putpixel(uint32_t x, uint32_t y, uint32_t color)
{
        *(vfb.addr + vfb.pitch * y + x) = color;
}


void video_putchar(uint32_t x, uint32_t y, unsigned char c, uint32_t fc, uint32_t bc)
{
//        uint8_t *glyph_start = (uint8_t *)console_font + 32 + console_font->bpg * c;
        uint8_t *glyph_start = (uint8_t *)console_font + 16 * (uint32_t)c;

        uint8_t *gp = glyph_start;

        for (uint32_t py = y; py < y + 16; py++) {
                uint32_t px = x;

                uint8_t mask = 0x80;
                for (int i = 0; i < 8; i++) {
                        if (*gp & mask) {
                                video_putpixel(px, py, fc);
                        } else {
                                video_putpixel(px, py, bc);
                        }
                        px++;
                        mask >>= 1;
                }
                gp++;
        }
}
