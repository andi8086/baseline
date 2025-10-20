#include <stdarg.h>
#include <stdint.h>

#include "c_printf.h"


#define PRINTF_POINTER_LONG  8
#define PRINTF_POINTER_SHORT 4

#define PRINTF_MODE_NONE  0
#define PRINTF_MODE_FLAGS 1
#define PRINTF_MODE_WIDTH 2
#define PRINTF_MODE_PREC  3
#define PRINTF_MODE_LEN   4
#define PRINTF_MODE_TYPE  5
#define PRINTF_MODE_OUTPUT 6

#define PRINTF_FLAGS_MINUS  1
#define PRINTF_FLAGS_PLUS   2
#define PRINTF_FLAGS_SPACE  4
#define PRINTF_FLAGS_0      8
#define PRINTF_FLAGS_GROUP 16
#define PRINTF_FLAGS_HASH  32

#define PRINTF_LEN_LONG 1

#define PRINTF_TYPE_SINT 1
#define PRINTF_TYPE_UINT 2
#define PRINTF_TYPE_OCTAL 3
#define PRINTF_TYPE_HEX_LOWER 4
#define PRINTF_TYPE_HEX_UPPER 5
#define PRINTF_TYPE_CSTRING 6
#define PRINTF_TYPE_CHAR 7
#define PRINTF_TYPE_POINTER 8
#define PRINTF_OUTPUT_LENGTH 9
#define PRINTF_TYPE_UNKNOWN 10

#define STRLEN_MAX 128

static int pf_strlen(char __far *s)
{
        int l = 0;
        while (*(s++) && l < STRLEN_MAX) l++;
        return l;
}

/* each type printer returns the number
   of buffer bytes needed and truncates
   if the buffer is insufficient */

/* This function is for unsigned numbers and interprets the
 * flags to calculate needed padding (spaces) and number of
 * leading zeroes
 *
 * input:
 * digits = number of digits of the bare integer without sign
 * signsize = number of digits for the sign or prefix
 * flags = the flags extracted from 'fmt'
 * p = the precision as extracted from 'fmt'
 * w = the width as extracted from 'fmt'
 * s = 1: include sign, 0: do not include sign
 *
 * output:
 * *lz = number of leading zeroes
 * *pad = number of padding spaces (left or right)
 * */
static void wps_lzpad(int digits, int signsize, uint8_t flags,
                      int w, int p, int s,
                      uint8_t *lz, uint8_t *pad)
{
        int prec;
        int prec_set;
        int grouping;

        grouping = flags & PRINTF_FLAGS_GROUP;

        if (grouping) {
                /* like in posix printf, only the real digits
                 * of the number are grouped, not the leading
                 * zeroes, which is very ugly indeed
                 * */
                digits += digits > 3 ? (digits - 1) / 3 : 0;
        }

        /* we must distinguish %04.d and %04d, which both give
         * precision == 0, but in one case explicitely set,
         * in the other not set. The first case does not
         * padd with zeroes, the 2nd does */
        prec_set = p != UINT8_MAX;

        if (p == UINT8_MAX) {
                p = 0;
        }

        *lz = 0;
        *pad = 0;

        /* the minimum precision is the number of digits */
        prec = digits;

        /* the sign (prefix for hex or octal) counts as digit(s) */
        if (s) {
                prec += signsize;
        }

        /* if precision specifies more than digits, we must
         * add leading zeroes */
        if (p > digits) {
                *lz = p - digits;
        }

        /* if the width is larger than precision (including sign),
         * we add padding */
        if (w > prec + *lz) {
                *pad = w - prec - *lz;
        }

        /* if the zero-flag is set, then the padding is only done
         * with zeroes, if no precision is specified,
         * except if the left alignment is set (minus flag), then
         * the padding is never done with zeroes */
        if ((flags & PRINTF_FLAGS_0) && !(flags & PRINTF_FLAGS_MINUS) &&
            w && !prec_set) {
                *lz += *pad;
                *pad = 0;
        }
}


static int bputn(char __far **d, char __far *d_max, char c, int n)
{
        int count = 0;

        while (n-- && *d < d_max) {
                *((*d)++) = c;
                count++;
        }

        return count;
}


static uint8_t print_hex(char __far *d, char __far *d_max, uint8_t flags,
                         uint8_t width, uint8_t prec, unsigned long int u,
                         char *hexchars)
{
        /* maximal 8 hex digits for 32-bit values (used on 16-bit machines) */
        char hexbuff[9];
        int i;
        char *hd;
        int count = 0;
        int digits;
        int display_sign, left_align;
        char s;
        uint8_t pad, lz;
        display_sign = flags & PRINTF_FLAGS_HASH;
        left_align = flags & PRINTF_FLAGS_MINUS;

        hexbuff[8] = '\0';
        s = hexchars[10] + 'x' - 'a';

        hd = &hexbuff[7];
        for (i = 0; i < 8; i++) {
                *hd = hexchars[u & 0xF];
                u >>= 4;
                if (!u) {
                        break;
                }
                hd--;
        }

        digits = pf_strlen(hd);
        wps_lzpad(digits, 2, flags, width, prec, display_sign, &lz, &pad);

        if (!left_align) count += bputn(&d, d_max, ' ', pad);

        /* display 0X or 0x if hashtag flag */
        if (display_sign) {
                *(d++) = '0';
                count++;
                if (d == d_max) {
                        return count;
                }
                *(d++) = s;
                count++;
                if (d == d_max) {
                        return count;
                }
        }

        count += bputn(&d, d_max, '0', lz);

        /* here hd points to the beginning of the hex string */
        while (*hd && d < d_max) { *(d++) = *(hd++); count++; }

        /* trailing spaces */
        if (left_align) count += bputn(&d, d_max, ' ', pad);

        return count;
}


static uint8_t print_octal(char __far *d, char __far *d_max, uint8_t flags,
                           uint8_t width, uint8_t prec, unsigned long int u)
{
        /* one octal digit encodes 3 bits,
         * hence 16 bits have 6 digits
         *       32 bits have 11 digits */

        char buffer[12];
        int i;
        int count = 0;
        int digits;
        int display_sign, left_align;
        uint8_t pad, lz;
        char *od = &buffer[10];

        display_sign = flags & PRINTF_FLAGS_HASH;
        left_align = flags & PRINTF_FLAGS_MINUS;

        buffer[11] = '\0';

        for (i = 0; i < 11; i++) {
                *od = (u & 7) + '0';
                u >>= 3;
                if (!u) {
                        break;
                }
                od--;
        }

        digits = pf_strlen(od);
        wps_lzpad(digits, 1, flags, width, prec, display_sign, &lz, &pad);

        if (!left_align) count += bputn(&d, d_max, ' ', pad);

        /* display 0 as prefix if hashtag flag */
        if (flags & PRINTF_FLAGS_HASH) {
                *(d++) = '0';
                count++;
        }

        count += bputn(&d, d_max, '0', lz);

        while (*od && d < d_max) { *(d++) = *(od++); count++; } /* digits */

        if (left_align) count += bputn(&d, d_max, ' ', pad);

        return count;
}

uint8_t divide32(unsigned long int *u, unsigned long int divisor)
{
        /* we cannot divide by the divisor, but we can
           subtract the divisor until the dividend is smaller */

        uint8_t r = 0;

        while (*u >= divisor && r < 10) {
                *u -= divisor;
                r++;
        }

        return r;
}


static uint8_t print_luint(char __far *d, char __far *d_max, uint8_t flags,
                           uint8_t width, uint8_t prec, unsigned long int u,
                           int sign)
{
        int digits;
        uint8_t lz, pad;
        int i, count;
        int grouping;
        int display_sign = 0;
        int left_align = flags & PRINTF_FLAGS_MINUS;
        char s;
        unsigned long div_table[10]; // 4 billion maximum (2^32 ~ 4 * 10^9)
        uint8_t divisor;

        div_table[0] = 1UL;
        div_table[1] = 10UL;
        div_table[2] = 100UL;
        div_table[3] = 1000UL;
        div_table[4] = 10000UL;
        div_table[5] = 100000UL;
        div_table[6] = 1000000UL;
        div_table[7] = 10000000UL;
        div_table[8] = 100000000UL;
        div_table[9] = 1000000000UL;

        s = '+'; /* s stores the minus sign */
        if (sign) {
                /* extract the sign and make the number positive */
                if (u & (1UL << 31)) {
                        u = ~u + 1;
                        s = '-';
                }
        }

        if (s == '-') {
                display_sign = 1;
        }

        if (s == '+' && (flags & PRINTF_FLAGS_PLUS)) {
                display_sign = 1;
        }

        if (s == '+' && (flags & PRINTF_FLAGS_SPACE)) {
                display_sign = 1;
                s = ' ';
        }

        grouping = flags & PRINTF_FLAGS_GROUP;
        /* first find out how many digits the number has */
        digits = 1;

        if (u > 9UL) digits++;
        if (u > 99UL) digits++;
        if (u > 999UL) digits++;
        if (u > 9999UL) digits++;
        if (u > 99999UL) digits++;
        if (u > 999999UL) digits++;
        if (u > 9999999UL) digits++;
        if (u > 99999999UL) digits++;
        if (u > 999999999UL) digits++;

        divisor = digits - 1;

        wps_lzpad(digits, 1, flags, width, prec, display_sign, &lz, &pad);

        count = 0;

        /* leading spaces */
        if (!left_align) count += bputn(&d, d_max, ' ', pad);

        if (display_sign && d < d_max) {
                /* output sign */
                *(d++) = s;
                count++;
        }

        /* leading zeroes */
        count += bputn(&d, d_max, '0', lz);

        /* output digits */
        for (i = 0; i < digits && d < d_max; i++, d++) {
                if (flags & PRINTF_FLAGS_GROUP) {
                        if (i != 0 && i != digits && ((digits - i) % 3 == 0)) {
                                *(d++) = ',';
                                count++;
                        }
                        if (d == d_max) {
                                break;
                        }

                }
                *d = divide32(&u, div_table[divisor]);
                *d += '0';
                divisor--;
                count++;
        }

        /* trailing spaces */
        if (left_align) count += bputn(&d, d_max, ' ', pad);

        return count;
}


static uint8_t print_str(char __far *d, char __far *d_max, uint8_t flags,
                         uint8_t width, uint8_t prec, char __far *str)
{
        /* %[W].[P]s says that we have W characters in the buffer
           where P characters come from the string and W-P are
           spaces with no spaces if W <= P. W == 0 is undefined
           we treat it as equal to P in this case. */
        int slen, count;

        slen = pf_strlen(str);
        if (prec > slen) {
                /* precision cannot be more than strlen */
                /* an unspecified precision is initialized to UINT8_MAX,
                   so it is also corrected here */
                prec = slen;
        }
        if (prec < slen) {
                /* precision can truncate the string down to zero */
                slen = prec;
        }

        if (width < prec) {
                /* totallen can never truncate below prec */
                width = prec;
        }

        /* for a C-String, we first check the size */
        count = 0;

        while (d < d_max && (*str || count < width)) {

                if (flags & PRINTF_FLAGS_MINUS) {
                        /* left aligned */
                        if (count < prec) {
                                *d = *str;
                                str++;
                        } else if (count < width) {
                                *d = ' ';
                        } else {
                                break;
                        }
                } else {
                        /* right aligned */
                        if (width > slen && count < width - slen) {
                                *d = ' ';
                        } else if (count < width) {
                                *d = *str;
                                str++;
                        } else {
                                break;
                        }
                }

                d++;
                count++;
        }

        return count;
}




void c_vsnprintf(char __far *buffer, int max, char __far *fmt, va_list p)
{
        char __far *d = buffer;
        char __far *d_max = buffer + max - 1;
        int mode = PRINTF_MODE_NONE;
        uint8_t flags;
        uint8_t width;
        uint8_t prec;
        uint8_t len;
        uint8_t type;
        /* fmt is %[param][flags][width][.prec][len]type */

        while (*fmt && d < d_max) {
                if (*fmt == '%') {
                        flags = 0;
                        width = 0;
                        prec = UINT8_MAX;
                        len = 0;
                        type = 0;
                        if (mode == PRINTF_MODE_FLAGS) {
                                /* '%%' prints a '%' */
                                mode = PRINTF_MODE_NONE;
                                fmt++;
                                *(d++) = '%';
                                continue;
                        } else {
                                mode = PRINTF_MODE_FLAGS;
                        }
                } else if (mode == PRINTF_MODE_FLAGS) {
                        switch (*fmt) {
                        case '-': flags |= PRINTF_FLAGS_MINUS; break;
                        case '+': flags |= PRINTF_FLAGS_PLUS; break;
                        case ' ': flags |= PRINTF_FLAGS_SPACE; break;
                        case '0': flags |= PRINTF_FLAGS_0; break;
                        case '\'': flags |= PRINTF_FLAGS_GROUP; break;
                        case '#': flags |= PRINTF_FLAGS_HASH; break;
                        default:
                                mode = PRINTF_MODE_WIDTH;
                                fmt--;
                        }
                } else if (mode == PRINTF_MODE_WIDTH) {
                        /* in the width field as tested in linux,
                           a leading 0 does not mean octal for the
                           width itself! */
                        if (*fmt >= '0' && *fmt <= '9') {
                                width *= 10;
                                width += *fmt - '0';
                        } else if (*fmt == '*') {
                                width = va_arg(p, int);
                        } else if (*fmt == '.') {
                                mode = PRINTF_MODE_PREC;
                        } else {
                                mode = PRINTF_MODE_LEN;
                                fmt--;
                        }
                } else if (mode == PRINTF_MODE_PREC) {
                        if (*fmt >= '0' && *fmt <= '9') {
                                if (prec == UINT8_MAX) {
                                        prec = 0;
                                }
                                prec *= 10;
                                prec += *fmt - '0';
                        } else if (*fmt == '*') {
                                prec = va_arg(p, int);
                        } else {
                                mode = PRINTF_MODE_LEN;
                                fmt--;
                        }
                } else if (mode == PRINTF_MODE_LEN) {
                        switch (*fmt) {
                        case 'l': len = PRINTF_LEN_LONG; break;
                        default:
                                mode = PRINTF_MODE_TYPE;
                                fmt--;
                                break;
                        }
                } else if (mode == PRINTF_MODE_TYPE) {
                        switch (*fmt) {
                        case 'd':
                        case 'i':
                                type = PRINTF_TYPE_SINT;
                                break;
                        case 'u':
                                type = PRINTF_TYPE_UINT;
                                break;
                        case 'x':
                                type = PRINTF_TYPE_HEX_LOWER;
                                break;
                        case 'X':
                                type = PRINTF_TYPE_HEX_UPPER;
                                break;
                        case 'o':
                                type = PRINTF_TYPE_OCTAL;
                                break;
                        case 's':
                                type = PRINTF_TYPE_CSTRING;
                                break;
                        case 'c':
                                type = PRINTF_TYPE_CHAR;
                                break;
                        case 'p':
                                type = PRINTF_TYPE_POINTER;
                                break;
                        default:
                                mode = PRINTF_TYPE_UNKNOWN;
                                break;
                        }

                        mode = PRINTF_MODE_OUTPUT;
                        fmt--;
                } else if (mode == PRINTF_MODE_OUTPUT) {
                        void __far *ptr;
                        char c;
                        char __far *str;
                        signed long int lsint;
                        unsigned long int luint;

                        switch (type) {
                        case PRINTF_TYPE_POINTER:
                                ptr = va_arg(p, void __far *);
                                d += print_hex(d, d_max, PRINTF_FLAGS_HASH,
                                               0,
                                               len == PRINTF_LEN_LONG ?
                                               PRINTF_POINTER_LONG :
                                               PRINTF_POINTER_SHORT,
                                               (uint32_t)ptr,
                                               "0123456789abcdef");
                                break;
                        case PRINTF_TYPE_CHAR:
                                c = (char)va_arg(p, int);
                                *(d++) = c;
                                break;
                        case PRINTF_TYPE_SINT:
                                if (len == PRINTF_LEN_LONG) {
                                        lsint = va_arg(p,
                                                long signed int);
                                } else {
                                        lsint = va_arg(p, signed int);
                                }
                                d += print_luint(d, d_max, flags, width,
                                                 prec, lsint, 1);
                                break;
                        case PRINTF_TYPE_UINT:
                                if (len == PRINTF_LEN_LONG) {
                                        luint = va_arg(p,
                                                long unsigned int);
                                } else {
                                        luint = va_arg(p, unsigned int);
                                }
                                d += print_luint(d, d_max, flags, width,
                                                 prec, luint, 0);
                                break;
                        case PRINTF_TYPE_OCTAL:
                                if (len == PRINTF_LEN_LONG) {
                                        luint = va_arg(p,
                                                long unsigned int);
                                } else {
                                        luint = va_arg(p, unsigned int);
                                }
                                d += print_octal(d, d_max, flags, width,
                                                 prec, luint);
                                break;
                        case PRINTF_TYPE_CSTRING:
                                str = va_arg(p, char __far *);
                                d += print_str(d, d_max, flags, width, prec,
                                               str);
                                break;
                        case PRINTF_TYPE_HEX_LOWER:
                                if (len == PRINTF_LEN_LONG) {
                                        luint = va_arg(p,
                                                long unsigned int);
                                } else {
                                        luint = va_arg(p, unsigned int);
                                }
                                d += print_hex(d, d_max, flags, width, prec,
                                               luint, "0123456789abcdef");
                                break;
                        case PRINTF_TYPE_HEX_UPPER:
                                if (len == PRINTF_LEN_LONG) {
                                        luint = va_arg(p,
                                                long unsigned int);
                                } else {
                                        luint = va_arg(p, unsigned int);
                                }
                                d += print_hex(d, d_max, flags, width, prec,
                                               luint, "0123456789ABCDEF");
                                break;
                        default:

                                break;
                        }
                        mode = PRINTF_MODE_NONE;
                } else if (mode == PRINTF_MODE_NONE) {
                        *d = *fmt;
                        d++;
                }

                fmt++;
        }

        *d = '\0';

}


void c_snprintf(char __far *buffer, int max, char __far *fmt, ...)
{
        va_list l;
        va_start(l, fmt);
        c_vsnprintf(buffer, max, fmt, l);
        va_end(l);
}
