#ifndef C_PRINTF_H
#define C_PRINTF_H


#include <stdarg.h>


void c_vsnprintf(char *buffer, int max, char *fmt, va_list p);
void c_snprintf(char *buffer, int max, char *fmt, ...);


#endif
