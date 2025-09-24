#ifndef PSF2_H
#define PSF2_H

typedef struct {
        char magic[4];
        uint32_t version;
        uint32_t headersize;
        uint32_t flags;
        uint32_t nglyphs;
        uint32_t bpg;
        uint32_t height;
        uint32_t width;
} psf2_header_t;


#endif
