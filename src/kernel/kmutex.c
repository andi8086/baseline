#include "kmutex.h"

#include "pe_mem.h"
#include "klib.h"


static kmutex_t *kmutex_table;


kmutex_t *kmutex_create(void)
{
        uint32_t kmutex_table_size =
                sizeof(kmutex_t) * KMUTEX_MAX;

        kmutex_t *tmp = NULL;

        if (!kmutex_table) {
                kmutex_table = kmalloc_high(
                        kmutex_table_size);
                memset(kmutex_table, 0, kmutex_table_size);
                tmp = &kmutex_table[0];
        } else {
                for (uint32_t i = 0; i < KMUTEX_MAX; i++) {
                        if (!kmutex_table[i]) {
                                tmp = &kmutex_table[i];
                                break;
                        }
                }
        }

        if (!tmp) {
                return NULL;
        }

        *tmp |= KMUTEX_USED;
        return tmp;
}


void kmutex_free(kmutex_t *m)
{
        *m = 0;
}


extern void spin_lock(kmutex_t *m);
extern void spin_unlock(kmutex_t *m);
extern void spin_lock_try_once(kmutex_t *m);

void kmutex_unlock(kmutex_t *m)
{
        *m = 0x80000000;
}


void kmutex_lock(kmutex_t *m)
{
        spin_lock(m);
}


bool kmutex_lock_try_once(kmutex_t *m)
{
        spin_lock_try_once(m);
        if (*m & KMUTEX_LOCKED) {
                return true;
        }
        return false;
}
