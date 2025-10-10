#include <stddef.h>

#include "klib.h"

#include <stdint.h>


int memcmp(const void *a, const void *b, unsigned long size)
{
        if (!size) {
                return 0;
        }

        while (--size && *(char *)a == *(char *)b) {
                a = (char *)a + 1;
                b = (char *)b + 1;
        }

        return (*((unsigned char *)a) - *((unsigned char *)b));
}


void memcpy(void *dst, void *src, unsigned long size)
{
        uint8_t *d = dst;
        uint8_t *s = src;
        if (!size) {
                return;
        }




        while (--size) {
                *d = *s;
                d++;
                s++;
        }
}


void memcpy_fast(uint8_t *pDest, uint8_t *pSrc, uint32_t len)
{
        /* by thickster */
        uint32_t srcCnt, destCnt, newLen, endLen, longLen;
        uint32_t *pLongSrc, *pLongDest, longWord1, longWord2;
        uint32_t methodSelect;

        // Determine the number of bytes in the first word of src and dest
        srcCnt = 4 - ((uint32_t)pSrc & 0x03);
        destCnt = 4 - ((uint32_t)pDest & 0x03);

        // Copy the initial bytes to the destination
        memcpy(pDest, pSrc, destCnt);

        // Determine the number of bytes remaining
        newLen = len - destCnt;

        // Determine how many full long words to copy to the destination
        longLen = newLen / 4;
        // Determine number of lingering bytes to copy at the end
        endLen = newLen & 0x03;
        // Pick the initial long destination word to copy to
        pLongDest = (uint32_t *)(pDest + destCnt);
        // Pick the initial source word to start our algorithm at
        if (srcCnt <= destCnt) {
                // Advance to pSrc at the start of the next full word
                pLongSrc = (uint32_t *)( pSrc + srcCnt);
        } else {
                // There are still source bytes remaining in the first word
                // Set pSrc to the start of the first full word
                pLongSrc = (uint32_t *)(pSrc + srcCnt - 4);
        }

        // There are 4 different longWord copy methods
        methodSelect = (srcCnt - destCnt) & 0x03;

        // Just copy one-to-one
        if (methodSelect == 0) {
                // Just copy the specified number of long words
                while (longLen-- > 0) {
                        *pLongDest++ = *pLongSrc++;
                }
        } else if (methodSelect == 1) {
                // Get the first long word
                longWord1 = *pLongSrc++;
                // Copy words created by combining 2 adjacent long words
                while (longLen-- > 0) {
                        // Get the next 32-bit word
                        longWord2 = *pLongSrc++;
                        // Write to the destination
                        *pLongDest++ = (longWord1 >> 24) | (longWord2 << 8);
                }
                // Re-use the word just retrieved
                longWord1 = longWord2;
        } else if (methodSelect == 2) {
                // Get the first long word
                longWord1 = *pLongSrc++;
                // Copy words created by combining 2 adjacent long words
                while (longLen-- > 0) {
                        // Get the next 32-bit word
                        longWord2 = *pLongSrc++;
                        // Write to the destination
                        *pLongDest++ = (longWord1 >> 16) | (longWord2 << 16);
                        // Re-use the word just retrieved
                        longWord1 = longWord2;
                }
        } else {
                // (methodSelect == 3)
                // Get the first long word
                longWord1 = *pLongSrc++;
                // Copy words created by combining 2 adjacent long words
                while (longLen-- > 0) {
                        // Get the next 32-bit word
                        longWord2 = *pLongSrc++;
                        // Write to the destination
                        *pLongDest++ = (longWord1 >> 8) | (longWord2 << 24);
                        // Re-use the word just retrieved
                        longWord1 = longWord2;
                }
        }

        // Copy any remaining bytes
        if (endLen != 0) {
                // The trailing bytes will be copied next
                pDest = (uint8_t *)pLongDest;
                // Determine where the trailing source bytes are located
                pSrc += len - endLen;
                // Copy the remaining bytes
                memcpy(pDest, pSrc, endLen);
        }
}


void memmove(void *dst, void *src, uint32_t size)
{
        uint8_t *d = dst;
        uint8_t *s = src;
        if (!size) {
                return;
        }

        while (--size) {
                *d = *s;
                d++;
                s++;
        }
}


void memset(void *dst, uint8_t val, uint32_t size)
{
        uint8_t *d = dst;
        if (!size) {
                return;
        }

        while (--size) {
                *d = val;
                d++;
        }
}
