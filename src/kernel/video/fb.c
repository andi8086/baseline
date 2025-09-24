#include "fb.h"


v_framebuffer_t vfb;


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
