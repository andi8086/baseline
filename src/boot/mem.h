#ifndef MEM_H
#define MEM_H

#include <stdint.h>


#define MK_FAR(seg, offs) ((uint32_t)seg * 65536 | offs)

/* Weird OpenWatcom Alien Syntax! */
uint16_t _SEG_ES(void);
#pragma aux _SEG_ES = \
"mov ax, es" \
value [ax];

uint16_t _SEG_DS(void);
#pragma aux _SEG_DS = \
"mov ax, ds" \
value [ax];


void strncpy(void far *dst, void far *src, uint16_t count);


#endif
