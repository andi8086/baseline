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


typedef struct {
        uint16_t attributes;
        uint8_t window_a;
        uint8_t window_b;
        uint16_t granularity;
        uint16_t window_size;
        uint16_t segmant_a;
        uint16_t segment_b;
        uint16_t win_func_ptr[2];
        uint16_t pitch;
        uint16_t width;
        uint16_t height;
        uint8_t w_char;
        uint8_t y_char;
        uint8_t planes;
        uint8_t bpp;
        uint8_t banks;
        uint8_t memory_model;
        uint8_t bank_size;
        uint8_t image_pages;
        uint8_t reserved0;

        uint8_t red_mask;
        uint8_t red_position;
        uint8_t green_mask;
        uint8_t green_position;
        uint8_t blue_mask;
        uint8_t blue_position;
        uint8_t reserved_mask;
        uint8_t reserved_position;
        uint8_t direct_color_attributes;

        uint16_t framebuffer[2];
        uint16_t off_screen_mem_off[2];
        uint16_t off_screen_mem_size;
        uint8_t reserved[206];
} vbe_mode_info_t;
#pragma pack(pop)

/* flag to request linear framebuffer */
#define VESA_REQUEST_LFB 0x4000

int vesa_init(void);
uint16_t vesa_find_mode(int16_t x, int16_t y, uint8_t bpp,
                        uint16_t *fblo, uint16_t *fbhi);
int vesa_mode_set(uint16_t mode);


#endif
