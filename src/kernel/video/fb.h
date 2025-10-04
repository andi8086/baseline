#ifndef FB_H
#define FB_H

#include <stdint.h>
#include "gc.h"


#define BPP_DEFAULT 32


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
void video_gc_put(gc_t *gc, int32_t x, int32_t y);
void video_gc_put_block(gc_t *gc, int32_t x, int32_t y,
                        int32_t gx, int32_t gy, uint32_t gw, uint32_t gh);


#endif
