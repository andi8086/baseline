#ifndef HEAPM_BT_RB
#define HEAPM_BT_RB
/************************************************************************
 *                      Modular Heap Memory Manager
 *
 *      Copyright (c) 2023 Andreas J. Reichel
 *      MIT License
 *
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 ************************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "btrb.h"

#ifdef HEAPM_USE_MUTEX_X
#        include "../mutex/xmutex.h"
#endif


#pragma pack(push, 1)
typedef struct hm_fblock {
        uintptr_t base;
        size_t size;
        struct hm_fblock *next;
        struct hm_fblock *prev;
        bool in_tree;
} hm_fblock_t;

typedef struct {
        uintptr_t abase;
        size_t asize;
#ifdef HEAPM_MALLOC_LINE_STORE
        uint32_t malloc_line;
#endif
        btrb_node_t fnode;
        btrb_node_t anode;
        hm_fblock_t fblock;
        uint32_t padding; /* warning, MUST be here, do not read
                             this value as it gets moved in memory! */
} hm_pfx_t;

#define FBLOCK_TO_NODE(fb)                                      \
        ((btrb_node_t *)((uintptr_t)fb + (uintptr_t) &          \
                         ((hm_pfx_t *)0)->fnode - (uintptr_t) & \
                         ((hm_pfx_t *)0)->fblock))


#define FNODE_TO_PFX(fn) \
        ((hm_pfx_t *)((uintptr_t)fn - (uintptr_t) & ((hm_pfx_t *)0)->fnode))


typedef struct {
#ifdef HEAPM_USE_MUTEX_X
        xmutex_t lock;
#endif
        void *mem_start;
        size_t mem_size;
        btrb_node_t *ftree_root;
        btrb_node_t *atree_root;
} hm_ctx_t;
#pragma pack(pop)


#ifdef HEAPM_DEBUG
#        include <stdio.h>
void show_mem(hm_ctx_t *ctx);
#endif

void hm_init(hm_ctx_t *ctx, void *base, size_t size);

#ifdef HEAPM_MALLOC_LINE_STORE
void *hm_aligned_alloc_d(hm_ctx_t *ctx, size_t size, size_t align,
                         uint32_t line);

#        define hm_alloc(ctx, size) hm_aligned_alloc_d(ctx, size, 0, __LINE__)
#        define hm_aligned_alloc(ctx, size, align) \
                hm_aligned_alloc_d(ctx, size, align, __LINE__)
#else

void *hm_alloc(hm_ctx_t *ctx, size_t size);
void *hm_aligned_alloc(hm_ctx_t *ctx, size_t size, size_t align);

#endif

void hm_free(hm_ctx_t *ctx, void *p);
uint64_t hm_max(hm_ctx_t *ctx);
uint64_t hm_available(hm_ctx_t *ctx, bool net);
uint64_t hm_allocated(hm_ctx_t *ctx);

#endif
