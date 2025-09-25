#ifndef BT_RB_H
#define BT_RB_H

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
#define BT_RB_VAL_TYPE uint64_t

typedef enum { RED, BLACK } bt_color_t;


typedef struct btrb_node {
        BT_RB_VAL_TYPE val;
        bt_color_t color;
        void *user_data;
        struct btrb_node *left;
        struct btrb_node *right;
        struct btrb_node *parent;
} btrb_node_t;

typedef void (*btrb_iterator_cb)(btrb_node_t *node);

btrb_node_t *btrb_nil(void);
void btrb_delete(btrb_node_t **root, btrb_node_t *v);
btrb_node_t *btrb_search(btrb_node_t **root, BT_RB_VAL_TYPE n);
void btrb_insert(btrb_node_t **root, BT_RB_VAL_TYPE n, void *user_data,
                 btrb_node_t *prealloc);
void btrb_delete_by_val(btrb_node_t **root, BT_RB_VAL_TYPE n);
void btrb_iterate_in_order(btrb_node_t *root, btrb_iterator_cb cb);
bool btrb_is_nil(btrb_node_t *n);

btrb_node_t *btrb_min_at_least(btrb_node_t **root, BT_RB_VAL_TYPE n);
btrb_node_t *btrb_max_at_most(btrb_node_t **root, BT_RB_VAL_TYPE n);
btrb_node_t *btrb_max(btrb_node_t **root);
btrb_node_t *btrb_min(btrb_node_t **root);

btrb_node_t *btrb_next_larger(btrb_node_t *node);
btrb_node_t *btrb_next_smaller(btrb_node_t *node);

/* internal functions, but needed for unit tests */
void btrb_left_rotate(btrb_node_t **root, btrb_node_t *x);
void btrb_right_rotate(btrb_node_t **root, btrb_node_t *y);
void btrb_transplant(btrb_node_t **root, btrb_node_t *u, btrb_node_t *v);
void btrb_delete_fixup(btrb_node_t **root, btrb_node_t *x);
void btrb_insert_fixup(btrb_node_t **root, btrb_node_t *z);
void btrb_delete_by_val(btrb_node_t **root, BT_RB_VAL_TYPE n);
#endif
