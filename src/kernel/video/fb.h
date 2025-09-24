#ifndef FB_H
#define FB_H

#include <stdint.h>

typedef struct {
        uint32_t *addr;
        uint32_t width;
        uint32_t height;
        uint32_t pitch;
        uint8_t bpp;
} v_framebuffer_t;


extern v_framebuffer_t vfb;

int video_init(uint32_t fb_addr, uint32_t w, uint32_t h, uint8_t bpp,
               uint32_t pitch);
void video_putpixel(uint32_t x, uint32_t y, uint32_t color);
void video_putchar(uint32_t x, uint32_t y, unsigned char c,
                   uint32_t fc, uint32_t bc);

#endif
