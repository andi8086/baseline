#ifndef KLIB_H
#define KLIB_H

#include <stdint.h>


int memcmp(const void *a, const void *b, unsigned long size);
void memcpy(void *, void *, unsigned long size);
void memset(void *dst, uint8_t val, uint32_t size);
void memmove(void *dst, void *src, uint32_t size);

void memcpy_fast(uint8_t *pDest, uint8_t *pSrc, uint32_t len);


#endif
