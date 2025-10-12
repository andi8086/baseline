#ifndef VESA_H
#define VESA_H


#include <stdint.h>


#pragma pack(push, 1)
typedef struct {
        char id[4];
        uint16_t version;
        uint16_t reserved[2];
        uint16_t caps[2];
        uint16_t mode_pointer_lo;
        uint16_t mode_pointer_hi;
} vesa_buffer_t;


typedef struct {
        uint16_t mode;
        int16_t width;
        int16_t height;
        uint8_t bpp;
        char *mode_str;
} vesa_mode_t;
#pragma pack(pop)


int vesa_init(void);


#endif
