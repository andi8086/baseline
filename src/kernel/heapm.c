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
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "heapm.h"


#ifdef HEAPM_USE_MUTEX_X
#        define MUTEX_LOCK   xmutex_lock(&ctx->lock)
#        define MUTEX_UNLOCK xmutex_unlock(&ctx->lock)
#else
#        define MUTEX_LOCK
#        define MUTEX_UNLOCK
#endif


static void fblock_chain_pop(hm_pfx_t *pfx)
{
        /* here we remove the fblock from the chain. The remaining fblocks are
         * referenced by other prefixes, so we clear this pfx' fblock to make
         * it available */

        hm_fblock_t *prev;
        hm_fblock_t *next;

        prev = pfx->fblock.prev;
        next = pfx->fblock.next;

        if (prev) {
                prev->next = next;
        }
        if (next) {
                next->prev = prev;
        }
        pfx->fblock.prev = NULL;
        pfx->fblock.next = NULL;
        pfx->fblock.size = 0;
}


static void ftree_add(hm_ctx_t *ctx, hm_fblock_t *block, btrb_node_t *node)
{
        btrb_insert(&ctx->ftree_root, block->size, block, node);
        block->in_tree = true;
}


static void fblock_remove(hm_ctx_t *ctx, hm_pfx_t *pfx)
{
        /* first we must check if the previous f-block is in the ftree, if yes,
         * we must remove it and add the next fblock into the ftree, so it has
         * one and only one reference to the fblock chain. Afterwards we can
         * remove the fblock from the chain to make it available */

        if (!pfx->fblock.in_tree) {
                fblock_chain_pop(pfx);
                return;
        }

        btrb_delete(&ctx->ftree_root, &pfx->fnode);
        pfx->fblock.in_tree = false;
        pfx->fblock.size    = 0;

        if (pfx->fblock.next) {
                /* the tree node adjacent to fblock (inside the same mem prefix
                 * MUST be usable here */
                btrb_insert(&ctx->ftree_root, pfx->fblock.next->size,
                            pfx->fblock.next, FBLOCK_TO_NODE(pfx->fblock.next));
                pfx->fblock.next->in_tree = true;
        }
}


static void fblock_grow(hm_ctx_t *ctx, hm_pfx_t *pfx, size_t inc_size)
{
        fblock_remove(ctx, pfx);

        pfx->fblock.size += inc_size;

        if (pfx->fblock.size) {
                ftree_add(ctx, &pfx->fblock, &pfx->fnode);
        }
}


static void fblock_chain_push(hm_pfx_t *pfx, hm_fblock_t *fblock)
{
        hm_fblock_t *next = pfx->fblock.next;
        pfx->fblock.next  = fblock;
        fblock->prev      = &pfx->fblock;
        fblock->next      = next;
}


static void fblock_add(hm_ctx_t *ctx, hm_pfx_t *pfx, uintptr_t base,
                       size_t size)
{
        pfx->fblock.base    = base;
        pfx->fblock.size    = size;
        pfx->fblock.in_tree = false;
        pfx->fblock.next    = NULL;
        pfx->fblock.prev    = NULL;

        /* look if there is already a block of this size in the ftree */
        btrb_node_t *tmp = btrb_search(&ctx->ftree_root, size);
        if (tmp) {
                /* yes, get corresponding pfx and push into chain */
                hm_pfx_t *fpfx = FNODE_TO_PFX(tmp);
                fblock_chain_push(fpfx, &pfx->fblock);
                return;
        }

        /* no, we have to add it to the ftree */
        btrb_insert(&ctx->ftree_root, pfx->fblock.size, &pfx->fblock,
                    &pfx->fnode);
        pfx->fblock.in_tree = true;
}


void hm_init(hm_ctx_t *ctx, void *base, size_t size)
{
#ifdef HEAPM_USE_MUTEX_X
        xmutex_init(&ctx->lock);
#endif
        MUTEX_LOCK;
        ctx->mem_start = base;
        ctx->mem_size  = size;

        /* reserve space for ftree and atree roots, with corresponding block
         * information */

        hm_pfx_t *root_pfx = (hm_pfx_t *)base;

        ctx->atree_root = btrb_nil();
        ctx->ftree_root = btrb_nil();

        root_pfx->abase = (uintptr_t)base;
        root_pfx->asize = sizeof(hm_pfx_t);
        btrb_insert(&ctx->atree_root, (uintptr_t)base, root_pfx,
                    &root_pfx->anode);

        fblock_add(ctx, root_pfx, (uintptr_t)base + sizeof(hm_pfx_t),
                   size - sizeof(hm_pfx_t));
        MUTEX_UNLOCK;
}


#ifndef HEAPM_MALLOC_LINE_STORE
void *hm_alloc(hm_ctx_t *ctx, size_t size)
{
        return hm_aligned_alloc(ctx, size, 0);
}
#endif


#ifdef HEAPM_MALLOC_LINE_STORE
void *hm_aligned_alloc_d(hm_ctx_t *ctx, size_t size, size_t align,
                         uint32_t line)
#else
void *hm_aligned_alloc(hm_ctx_t *ctx, size_t size, size_t align)
#endif
{
        if (!size) {
                /* would be possible, but doesn't make sense */
                return NULL;
        }

        /* add size of prefix to size */
        size_t rsize = size + sizeof(hm_pfx_t);

        MUTEX_LOCK;

        btrb_node_t *fnode = btrb_min_at_least(&ctx->ftree_root, rsize);

        if (!fnode) {
                /* we are out of memory */
                MUTEX_UNLOCK;
                return NULL;
        }

        hm_fblock_t *fblock = (hm_fblock_t *)fnode->user_data;

        uint64_t alignment_padding = 0;

        if (align > 1) {
                /* create alignment mask for address */
                uint64_t mask = align - 1;
                uint64_t addr_remainder =
                    (fblock->base + sizeof(hm_pfx_t)) & mask;

                if (addr_remainder == 0) {
                        goto already_aligned;
                }

                alignment_padding = align - addr_remainder;
                rsize += alignment_padding;

                while (fnode->val < rsize) {
                        fnode = btrb_next_larger(fnode);

                        if (!fnode) {
                                MUTEX_UNLOCK;
                                return NULL;
                        }
                }

                fblock = (hm_fblock_t *)fnode->user_data;

                addr_remainder    = (fblock->base + sizeof(hm_pfx_t)) & mask;
                alignment_padding = align - addr_remainder;
        }

already_aligned:

        /* create new pfx and store allocation info */
        hm_pfx_t *new_pfx = (hm_pfx_t *)fblock->base;

        new_pfx->abase = fblock->base;
        new_pfx->asize = rsize;

        new_pfx->fblock.base = 0;
        new_pfx->fblock.size = 0;
        new_pfx->fblock.next = NULL;
        new_pfx->fblock.prev = NULL;

        if (fblock->size - rsize) {
                fblock_add(ctx, new_pfx, fblock->base + rsize,
                           fblock->size - rsize);
        }
        fblock_remove(ctx, FNODE_TO_PFX(fnode));
        btrb_insert(&ctx->atree_root, new_pfx->abase, new_pfx, &new_pfx->anode);

        /* The trick here is to store the padding value padded as well, directly
         * before the user area, so that the address of the pfx can be
         * reconstructed in a free call - in case of zero padding, the value
         * zero gets written to location of new_pfx->padding directly */

        uint32_t *padd_val =
            (uint32_t *)(((uintptr_t)&new_pfx->padding) + alignment_padding);
        *padd_val = alignment_padding;

#ifdef HEAPM_MALLOC_LINE_STORE
        new_pfx->malloc_line = line;
#endif

        MUTEX_UNLOCK;
        return (void *)(new_pfx->abase + sizeof(hm_pfx_t) + alignment_padding);
}

#ifndef HEAPM_FATAL_HANDLER
#        define HEAPM_FATAL_HANDLER abort
#endif

void hm_free(hm_ctx_t *ctx, void *p)
{
        uint32_t *padding_val = (uint32_t *)p;
        padding_val--;

        hm_pfx_t *pfx = (hm_pfx_t *)((uintptr_t)p - *padding_val);
        pfx--;
        /* in case you wondered what just happend here:
         *      Directly before the user area which p points to, we have
         *      stored a 32 bit value of the padding between the prefix
         *      and the user area.
         *      The pfx pointer is restored by first shifting p by
         *      *padding_val bytes and then shift it again by
         *      the size of its dereferenced data (sizeof(hm_pfx_t)) */

        MUTEX_LOCK;

        /* find adjacent preceding allocated pfx */
        btrb_node_t *adj_pre_node = btrb_next_smaller(&pfx->anode);
        if (!adj_pre_node) {
                /* this should never happen */
                HEAPM_FATAL_HANDLER();
        }
        btrb_delete(&ctx->atree_root, &pfx->anode);

        hm_pfx_t *pre_pfx = (hm_pfx_t *)adj_pre_node->user_data;
        fblock_grow(ctx, pre_pfx,
                    pfx->asize + pfx->fblock.size + pre_pfx->fblock.size);

        fblock_remove(ctx, pfx);

        MUTEX_UNLOCK;
}


uint64_t hm_max(hm_ctx_t *ctx)
{
        MUTEX_LOCK;

        /* return the maximally allocatable memory block size */
        btrb_node_t *tmp = btrb_max(&ctx->ftree_root);

        if (tmp && !btrb_is_nil(tmp)) {
                MUTEX_UNLOCK;
                return tmp->val - sizeof(hm_pfx_t);
        }

        MUTEX_UNLOCK;
        return 0;
}


uint64_t hm_available(hm_ctx_t *ctx, bool net)
{
        /* traverse all free blocks and add their sizes. if the net flag is
         * set, subtract the sizes of the additionally allocated prefixes to
         * get the pure usable space */
        uint64_t free = 0;

        MUTEX_LOCK;
        btrb_node_t *tmp = btrb_min(&ctx->ftree_root);
        do {
                if (!tmp || btrb_is_nil(tmp)) {
                        break;
                }

                hm_fblock_t *fb = tmp->user_data;

                do {
                        free += fb->size;
                        if (net) {
                                free -= sizeof(hm_pfx_t);
                        }
                        fb = fb->next;
                } while (fb);

                tmp = btrb_next_larger(tmp);

        } while (!btrb_is_nil(tmp) && tmp);

        MUTEX_UNLOCK;
        return free;
}


uint64_t hm_allocated(hm_ctx_t *ctx)
{
        uint64_t allocated = 0;

        MUTEX_LOCK;

        btrb_node_t *tmp = btrb_min(&ctx->atree_root);
        do {
                if (!tmp || btrb_is_nil(tmp)) {
                        break;
                }
                hm_pfx_t *pfx = tmp->user_data;
                allocated += pfx->asize;
                tmp = btrb_next_larger(tmp);
        } while (!btrb_is_nil(tmp) && tmp);

        MUTEX_UNLOCK;
        return allocated;
}


#ifdef HEAPM_DEBUG
void show_mem(hm_ctx_t *ctx)
{
        btrb_node_t *tmp;

        printf("-----------------------------------------\n");
        printf("Memory start at %llx\n", ctx->mem_start);
        /* first iterate all allocated blocks */
        printf("\n");

        uintptr_t start = (uintptr_t)ctx->mem_start;

        tmp = btrb_min(&ctx->atree_root);
        do {
                if (!tmp || btrb_is_nil(tmp)) {
                        printf("No allocated memory\n");
                        break;
                }

                hm_pfx_t *pfx = tmp->user_data;
                printf("allocated memory %08" PRIx64 "-%08" PRIx64 " [%" PRIu64
                       " bytes, anode @ "
                       "%04" PRIx64 "]",
                       pfx->abase - start, pfx->abase + pfx->asize - start,
                       pfx->asize,
                       (uintptr_t)&pfx->anode - (uintptr_t)ctx->mem_start);
                if (pfx->abase == (uintptr_t)ctx->mem_start) {
                        printf(" (heap manager root)\n");
                } else {
                        printf("\n");
                }
                tmp = btrb_next_larger(tmp);
        } while (!btrb_is_nil(tmp) && tmp);

        printf("\n");
        tmp = btrb_min(&ctx->ftree_root);
        do {
                if (!tmp || btrb_is_nil(tmp)) {
                        printf("No free memory\n");
                        break;
                }

                hm_fblock_t *fb = tmp->user_data;
                do {
                        printf("free memory %08" PRIx64 "-%08" PRIx64
                               " [%" PRIu64 "]\n",
                               fb->base - start, fb->base + fb->size - start,
                               fb->size);
                        fb = fb->next;
                } while (fb);

                tmp = btrb_next_larger(tmp);
        } while (!btrb_is_nil(tmp) && tmp);
}
#endif
