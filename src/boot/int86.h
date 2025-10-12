#ifndef INT86_H
#define INT86_H

#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
        uint16_t _ax;
        uint16_t _bx;
        uint16_t _cx;
        uint16_t _dx;
        uint16_t _es;
        uint16_t _di;
        uint16_t _flags;
} regs86_t;
#pragma pack(pop)

#define offset_of(s, x) (&((s)0)->x)


void vid_int86(regs86_t *r_in, regs86_t *r_out);



#endif
