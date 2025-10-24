#ifndef CONIO_H
#define CONIO_H

#include <stdint.h>


int getc(void);
void puts(char near *s);
void putc(char c);
void printf(char __far *fmt, ...);
void ser_printf(char __far *fmt, ...);

void buffered_input(char __far *buffer);

#endif
