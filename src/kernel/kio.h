#ifndef KIO_H
#define KIO_H


#include <stdint.h>



void outb(uint16_t port, uint8_t val);
void outw(uint16_t port, uint16_t val);
void outd(uint16_t port, uint32_t val);
uint8_t inb(int16_t port);
uint16_t inw(uint16_t port);
uint32_t ind(uint16_t port);

#endif
