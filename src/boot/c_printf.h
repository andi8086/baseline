#ifndef C_PRINTF_H
#define C_PRINTF_H


#include <stdarg.h>


void c_vsnprintf(char __far *buffer, int max, char __far *fmt, va_list p);
void c_snprintf(char __far *buffer, int max, char __far *fmt, ...);


#endif
