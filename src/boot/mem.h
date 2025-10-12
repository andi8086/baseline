#ifndef MEM_H
#define MEM_H

#include <stdint.h>


#define MK_FAR(seg, offs) ((uint32_t)seg * 65536 | offs)


uint16_t _SEG_ES(void);


#endif
