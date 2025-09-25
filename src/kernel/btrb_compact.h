#ifndef BT_RBC_H
#define BT_RBC_H

/************************************************************************
 *               SELF BALANCING RED-BLACK BINARY TREES
 *
 * Based on "Introduction to Algorithms", 3rd ed., Cormen, et al.
 * With additional functionality
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

#include <stdint.h>
#include <stdbool.h>

/* Header for red-black self-balancing binary trees */
#define BT_RBC_VAL_TYPE uint32_t

#define RED   0
#define BLACK 1

/* all 32-bit pointers are relative to a 64-bit base */

#pragma pack(push, 1)
typedef struct btrbc_node {
        uint32_t val : 31;
        uint32_t color : 1;
        uint32_t user_data;
        uint32_t left;
        uint32_t right;
        uint32_t parent;
} btrbc_node_t;
#pragma pack(pop)

typedef struct {
        btrbc_node_t **root;
        uint32_t nil;
        uintptr_t base;
} btrbc_ctx_t;


typedef void (*btrbc_iterator_cb)(btrbc_node_t *node);

void btrbc_init(btrbc_ctx_t *ctx, btrbc_node_t **root, uintptr_t base,
                btrbc_node_t *nil_node);

btrbc_node_t *btrbc_nil(btrbc_ctx_t *ctx);
void btrbc_delete(btrbc_ctx_t *ctx, btrbc_node_t *v);
btrbc_node_t *btrbc_search(btrbc_ctx_t *ctx, BT_RBC_VAL_TYPE n);
void btrbc_insert(btrbc_ctx_t *ctx, BT_RBC_VAL_TYPE n, void *user_data,
                  btrbc_node_t *prealloc);
void btrbc_delete_by_val(btrbc_ctx_t *ctx, BT_RBC_VAL_TYPE n);
void btrbc_iterate_in_order(btrbc_ctx_t *ctx, btrbc_iterator_cb cb);
bool btrbc_is_nil(btrbc_ctx_t *ctx, btrbc_node_t *n);

btrbc_node_t *btrbc_min_at_least(btrbc_ctx_t *ctx, BT_RBC_VAL_TYPE n);
btrbc_node_t *btrbc_max_at_most(btrbc_ctx_t *ctx, BT_RBC_VAL_TYPE n);
btrbc_node_t *btrbc_max(btrbc_ctx_t *ctx);
btrbc_node_t *btrbc_min(btrbc_ctx_t *ctx);

btrbc_node_t *btrbc_next_larger(btrbc_ctx_t *ctx, btrbc_node_t *node);
btrbc_node_t *btrbc_next_smaller(btrbc_ctx_t *ctx, btrbc_node_t *node);

/* internal functions, but needed for unit tests */
void btrbc_left_rotate(btrbc_ctx_t *ctx, btrbc_node_t *x);
void btrbc_right_rotate(btrbc_ctx_t *ctx, btrbc_node_t *y);
void btrbc_transplant(btrbc_ctx_t *ctx, btrbc_node_t *u, btrbc_node_t *v);
void btrbc_delete_fixup(btrbc_ctx_t *ctx, btrbc_node_t *x);
void btrbc_insert_fixup(btrbc_ctx_t *ctx, btrbc_node_t *z);
void btrbc_delete_by_val(btrbc_ctx_t *ctx, BT_RBC_VAL_TYPE n);
#endif
