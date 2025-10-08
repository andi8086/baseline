#ifndef KMUTEX_H
#define KMUTEX_H


#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>


#define KMUTEX_LOCKED (UINT32_C(1))
#define KMUTEX_USED   (UINT32_C(1) << 31)


typedef uint32_t kmutex_t;


#define KMUTEX_MAX 262144


kmutex_t *kmutex_create(void);
void kmutex_free(kmutex_t *m);
void kmutex_unlock(kmutex_t *m);
void kmutex_lock(kmutex_t *m);
bool kmutex_lock_try_once(kmutex_t *m);

#endif
