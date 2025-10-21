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


/* taken from i86.h */

#define _FP_OFF(__p) ((unsigned)(__p))

unsigned short _FP_SEG(const volatile void __far *);
#pragma aux _FP_SEG = \
        __parm __caller [__ax __dx] \
        __value [__dx] \
        __modify __exact []

#define _MK_FP(__s,__o) (((unsigned short)(__s)):>((void __near *)(__o)))


void strncpy(void far *dst, void far *src, uint16_t count);
void memset(void far *dst, uint8_t val, uint16_t count);
int strncmp(char __far *dst, char __far *src, uint16_t count);
void memcpy(void __far *dst, void __far *src, uint16_t count);

#endif
