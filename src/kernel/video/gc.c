#include "gc.h"
#include "../pe_mem.h"
#include "fb.h"
#include "../klib.h"


gc_t *gc_create(uint32_t width, uint32_t height)
{
        uint8_t pixel_size = BPP_DEFAULT >> 3;
        /* Allocate memory for virtual frame buffer */
        gc_t *gc =
                kmalloc_high(sizeof(gc_t) +
                             width * height * pixel_size);
        if (!gc) {
                return NULL;
        }

        gc->width = width;
        gc->height = height;
        gc->vpitch = width * pixel_size;
        gc->gmem = (uint32_t)((uintptr_t)gc + sizeof(gc_t));

        gc->mincx = -1;
        gc->mincy = -1;
        gc->maxcx = -1;
        gc->maxcy = -1;
        return gc;
}


void gc_destroy(gc_t *gc)
{
        kfree(gc);
}


void gc_putpixel(gc_t *gc, int32_t x, int32_t y, uint32_t color)
{
        uint32_t *dst = (uint32_t *)gc->gmem;
        dst += x + y * (gc->vpitch >> 2);
        *dst = color;
        if (x < gc->mincx || gc->mincx == -1) {
                gc->mincx = x;
        }
        if (y < gc->mincy || gc->mincy == -1) {
                gc->mincy = y;
        }
        if (x > gc->maxcx || gc->maxcx == -1) {
                gc->maxcx = x;
        }
        if (y > gc->maxcy || gc->maxcy == -1) {
                gc->maxcy = y;
        }
}


void gc_update_fb(gc_t *gc, int32_t ux, int32_t uy)
{
        /* (mincx, mincy)-(maxcx, maxcy) defines a block
           to be redrawn onto the frame buffer */
        if (gc->mincx == -1 || gc->mincy == -1 ||
            gc->maxcx == -1 || gc->maxcy == -1) {
                video_gc_put(gc, ux, uy);
        } else {
                video_gc_put_block(gc, ux, uy, gc->mincx, gc->mincy,
                                   gc->maxcx - gc->mincx + 1,
                                   gc->maxcy - gc->mincy + 1);
        }
        gc->mincx = -1;
        gc->mincy = -1;
        gc->maxcx = -1;
        gc->maxcy = -1;
}


void gc_clear(gc_t *gc, uint32_t bgcolor)
{
        for (int32_t y = 0; y < gc->height; y++) {
                for (int32_t x = 0; x < gc->width; x++) {
                        gc_putpixel(gc, x, y, bgcolor);
                }
        }
}


void gc_scroll_up(gc_t *gc, uint32_t dy)
{
        /* start offset for data to scroll */
        uint8_t *start = (uint8_t *)gc->gmem + gc->vpitch * dy;
        uint8_t *dst = (uint8_t *)gc->gmem;
        uint8_t *lower_block = (uint8_t *)gc->gmem + gc->vpitch * (gc->height - dy);

        memcpy_fast(dst, start, gc->vpitch * (gc->height - dy));
}



void gc_fill_vblock(gc_t *gc, uint32_t offset, uint32_t count, uint32_t val)
{
        uint32_t *start = (uint32_t *)(gc->gmem + offset);

        while (count--) {
                *(start++) = val;
        }
}

