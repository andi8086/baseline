#include "fb.h"
#include "gc.h"
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
        vfb.pitch = pitch;

        return 0;
}


void video_gc_put(gc_t *gc, int32_t x, int32_t y)
{
        /* calculate the starting offset in the frame buffer */
        uint32_t pixel_size = vfb.bpp >> 3;
        uint32_t start_offset;

        uint32_t vmax = vfb.height * vfb.pitch - 1;

        uint32_t *vfb_max = (uint32_t *)((uintptr_t)vfb.addr + vmax);

        start_offset = y * vfb.pitch + x * pixel_size;

        uint32_t *dst = (uint32_t *)((uintptr_t)
                vfb.addr + start_offset);

        uint32_t *src = (uint32_t *)(uint32_t)gc->gmem;

        int32_t gx = 0, gy = 0;

        while (gy < gc->height && dst < vfb_max) {

                gx = 0;

                while (gx < gc->width && dst < vfb_max && x + gx < vfb.width) {
                        *dst = *src;
                        dst++;
                        src++;
                        gx++;
                }

                if (dst >= vfb_max) {
                        break;
                }

                src += gc->width - gx;
                dst += (vfb.pitch >> 2) - gx;
                gy++;
        }
}


void video_gc_put_block(gc_t *gc, int32_t x, int32_t y,
                        int32_t gx, int32_t gy, uint32_t gw, uint32_t gh)
{
        if (gw > gc->width) {
                gw = gc->width;
        }
        if (gh > gc->height) {
                gh = gc->height;
        }

        /* calculate the starting offset in the frame buffer */
        uint32_t pixel_size = vfb.bpp >> 3;
        uint32_t dest_offset, src_offset;

        uint32_t vmax = vfb.height * vfb.pitch - 1;
        uint32_t *vfb_max = (uint32_t *)((uintptr_t)vfb.addr + vmax);

        dest_offset = y * vfb.pitch + x * pixel_size;
        uint32_t *dst = (uint32_t *)((uintptr_t)vfb.addr + dest_offset);

        src_offset = gy * gc->vpitch + gx * pixel_size;
        uint32_t *src = (uint32_t *)((uintptr_t)gc->gmem + src_offset);

        uint32_t pixel_count = 0;

        while (gh-- && dst < vfb_max) {

                gx = 0;
                pixel_count = 0;

                while (pixel_count < gw && dst < vfb_max &&
                       x + pixel_count < vfb.width) {

                        *dst = *src;
                        dst++;
                        src++;
                        pixel_count++;
                }

                if (dst >= vfb_max) {
                        break;
                }

                src += (gc->vpitch >> 2) - pixel_count;
                dst += (vfb.pitch >> 2) - pixel_count;
        }
}
