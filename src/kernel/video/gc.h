#ifndef GC_H
#define GC_H


#include <stdint.h>


/* Graphics context */
#pragma pack(push, 1)
typedef struct {
        uint32_t gmem;   /* virtual frame buffer */
        uint32_t vpitch;
        uint32_t width;
        uint32_t height;
        int32_t mincx, mincy;
        int32_t maxcx, maxcy;
} gc_t;
#pragma pack(pop)


gc_t *gc_create(uint32_t width, uint32_t height);
void gc_destroy(gc_t *gc);

void gc_update_fb(gc_t *gc, int32_t ux, int32_t uy);

void gc_putpixel(gc_t *gc, int32_t x, int32_t y, uint32_t color);
void gc_clear(gc_t *gc, uint32_t bgcolor);
void gc_scroll_up(gc_t *gc, uint32_t dy);
void gc_fill_vblock(gc_t *gc, uint32_t offset, uint32_t count, uint32_t val);

#endif
