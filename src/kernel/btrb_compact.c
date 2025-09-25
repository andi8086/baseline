#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "btrb_compact.h"

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


static inline btrbc_node_t *_btrbcp64(uintptr_t base, uint32_t p)
{
        return (btrbc_node_t *)(base + p);
}


static inline uint32_t _btrbcp32(uintptr_t base, void *p)
{
        return (uint32_t)((uintptr_t)p - base);
}

#define P64(x) _btrbcp64(ctx->base, x)

#define P32(x) _btrbcp32(ctx->base, x)


void btrbc_init(btrbc_ctx_t *ctx, btrbc_node_t **root, uintptr_t base,
                btrbc_node_t *nil_node)
{

        nil_node->parent = 0;
        nil_node->left   = 0;
        nil_node->right  = 0;
        nil_node->val    = 0;
        nil_node->color  = BLACK;
        ctx->base        = base;

        ctx->nil = P32(nil_node);

        ctx->root = root;
        *root     = nil_node;
}


btrbc_node_t *btrbc_nil(btrbc_ctx_t *ctx)
{
        return P64(ctx->nil);
}


void btrbc_left_rotate(btrbc_ctx_t *ctx, btrbc_node_t *x)
{
        btrbc_node_t *y = P64(x->right);
        x->right        = y->left;
        if (y->left != ctx->nil) {
                P64(y->left)->parent = P32(x);
        }
        y->parent = x->parent;
        if (x->parent == ctx->nil) {
                *ctx->root = y;
        } else if (x == P64(P64(x->parent)->left)) {
                P64(x->parent)->left = P32(y);
        } else {
                P64(x->parent)->right = P32(y);
        }
        y->left   = P32(x);
        x->parent = P32(y);
}


void btrbc_right_rotate(btrbc_ctx_t *ctx, btrbc_node_t *y)
{
        btrbc_node_t *x = P64(y->left);
        y->left         = x->right;
        if (x->right != ctx->nil) {
                P64(x->right)->parent = P32(y);
        }
        x->parent = y->parent;
        if (y->parent == ctx->nil) {
                *ctx->root = x;
        } else if (y == P64(P64(y->parent)->right)) {
                P64(y->parent)->right = P32(x);
        } else {
                P64(y->parent)->left = P32(x);
        }
        x->right  = P32(y);
        y->parent = P32(x);
}


void btrbc_transplant(btrbc_ctx_t *ctx, btrbc_node_t *u, btrbc_node_t *v)
{
        if (u->parent == ctx->nil) {
                *ctx->root = v;
        } else if (P32(u) == P64(u->parent)->left) {
                P64(u->parent)->left = P32(v);
        } else {
                P64(u->parent)->right = P32(v);
        }
        v->parent = u->parent;
}


void btrbc_delete_fixup(btrbc_ctx_t *ctx, btrbc_node_t *x)
{
        if (*ctx->root == P64(ctx->nil)) {
                return;
        }
        while (x != *ctx->root && x->color == BLACK) {
                if (P32(x) == P64(x->parent)->left) {
                        btrbc_node_t *w = P64(P64(x->parent)->right);
                        if (w->color == RED) {
                                w->color              = BLACK;
                                P64(x->parent)->color = RED;
                                btrbc_left_rotate(ctx, P64(x->parent));
                                w = P64(P64(x->parent)->right);
                        }
                        if (P64(w->left)->color == BLACK &&
                            P64(w->right)->color == BLACK) {
                                w->color = RED;
                                x        = P64(x->parent);
                        } else {
                                if (P64(w->right)->color == BLACK) {
                                        P64(w->left)->color = BLACK;
                                        w->color            = RED;
                                        btrbc_right_rotate(ctx, w);
                                        w = P64(P64(x->parent)->right);
                                }
                                w->color              = P64(x->parent)->color;
                                P64(x->parent)->color = BLACK;
                                P64(w->right)->color  = BLACK;
                                btrbc_left_rotate(ctx, P64(x->parent));
                                x = *ctx->root;
                        }
                } else {
                        btrbc_node_t *w = P64(P64(x->parent)->left);
                        if (w->color == RED) {
                                w->color              = BLACK;
                                P64(x->parent)->color = RED;
                                btrbc_right_rotate(ctx, P64(x->parent));
                                w = P64(P64(x->parent)->left);
                        }
                        if (P64(w->right)->color == BLACK &&
                            P64(w->left)->color == BLACK) {
                                w->color = RED;
                                x        = P64(x->parent);
                        } else {
                                if (P64(w->left)->color == BLACK) {
                                        P64(w->right)->color = BLACK;
                                        w->color             = RED;
                                        btrbc_left_rotate(ctx, w);
                                        w = P64(P64(x->parent)->left);
                                }
                                w->color              = P64(x->parent)->color;
                                P64(x->parent)->color = BLACK;
                                P64(w->left)->color   = BLACK;
                                btrbc_right_rotate(ctx, P64(x->parent));
                                x = *ctx->root;
                        }
                }
        }
        x->color = BLACK;
}


static btrbc_node_t *tree_minimum(btrbc_ctx_t *ctx, btrbc_node_t *x)
{
        while (x->left != ctx->nil) {
                x = P64(x->left);
        }
        return x;
}


void btrbc_delete(btrbc_ctx_t *ctx, btrbc_node_t *z)
{
        btrbc_node_t *y = z;
        btrbc_node_t *x;
        int y_original_color = y->color;

        if (z->left == ctx->nil) {
                x = P64(z->right);
                btrbc_transplant(ctx, z, P64(z->right));
        } else if (z->right == ctx->nil) {
                x = P64(z->left);
                btrbc_transplant(ctx, z, P64(z->left));
        } else {
                y                = tree_minimum(ctx, P64(z->right));
                y_original_color = y->color;
                x                = P64(y->right);
                if (y->parent == P32(z)) {
                        x->parent = P32(y);
                } else {
                        btrbc_transplant(ctx, y, P64(y->right));
                        y->right              = z->right;
                        P64(y->right)->parent = P32(y);
                }
                btrbc_transplant(ctx, z, y);
                y->left              = z->left;
                P64(y->left)->parent = P32(y);
                y->color             = z->color;
        }
        if (y_original_color == BLACK) {
                btrbc_delete_fixup(ctx, x);
        }
}


btrbc_node_t *btrbc_search(btrbc_ctx_t *ctx, BT_RBC_VAL_TYPE n)
{
        btrbc_node_t *x = *ctx->root;

        while (!btrbc_is_nil(ctx, x) && x->val != n) {
                if (x->val < n) {
                        x = P64(x->right);
                }
                if (x->val > n) {
                        x = P64(x->left);
                }
        }
        if (btrbc_is_nil(ctx, x)) {
                return NULL;
        }
        return x;
}


void btrbc_insert_fixup(btrbc_ctx_t *ctx, btrbc_node_t *z)
{
        btrbc_node_t *y;
        while (P64(z->parent)->color == RED) {
                if (z->parent == P64(P64(z->parent)->parent)->left) {
                        y = P64(P64(P64(z->parent)->parent)->right);
                        if (y->color == RED) {
                                P64(z->parent)->color              = BLACK;
                                y->color                           = BLACK;
                                P64(P64(z->parent)->parent)->color = RED;
                                z = P64(P64(z->parent)->parent);
                        } else {
                                if (z == P64(P64(z->parent)->right)) {
                                        z = P64(z->parent);
                                        btrbc_left_rotate(ctx, z);
                                }
                                P64(z->parent)->color              = BLACK;
                                P64(P64(z->parent)->parent)->color = RED;
                                btrbc_right_rotate(ctx,
                                                   P64(P64(z->parent)->parent));
                        }
                } else {
                        y = P64(P64(P64(z->parent)->parent)->left);
                        if (y->color == RED) {
                                P64(z->parent)->color              = BLACK;
                                y->color                           = BLACK;
                                P64(P64(z->parent)->parent)->color = RED;
                                z = P64(P64(z->parent)->parent);
                        } else {
                                if (z == P64(P64(z->parent)->left)) {
                                        z = P64(z->parent);
                                        btrbc_right_rotate(ctx, z);
                                }
                                P64(z->parent)->color              = BLACK;
                                P64(P64(z->parent)->parent)->color = RED;
                                btrbc_left_rotate(ctx,
                                                  P64(P64(z->parent)->parent));
                        }
                }
        }
        (*ctx->root)->color = BLACK;
}


void btrbc_insert(btrbc_ctx_t *ctx, BT_RBC_VAL_TYPE n, void *user_data,
                  btrbc_node_t *z)
{
        if (*ctx->root == NULL) {
                *ctx->root = P64(ctx->nil);
        }

        btrbc_node_t *y = P64(ctx->nil);
        btrbc_node_t *x = *ctx->root;
        z->val          = n;
        while (!btrbc_is_nil(ctx, x)) {
                y = x;
                if (z->val < x->val) {
                        x = P64(x->left);
                } else {
                        x = P64(x->right);
                }
        }
        z->parent = P32(y);
        if (btrbc_is_nil(ctx, y)) {
                *ctx->root = z;
        } else if (z->val < y->val) {
                y->left = P32(z);
        } else {
                y->right = P32(z);
        }
        z->left      = ctx->nil;
        z->right     = ctx->nil;
        z->color     = RED;
        z->user_data = P32(user_data);
        btrbc_insert_fixup(ctx, z);
}


void btrbc_delete_by_val(btrbc_ctx_t *ctx, BT_RBC_VAL_TYPE n)
{
        btrbc_node_t *node = btrbc_search(ctx, n);

        if (!n) {
                return;
        }

        btrbc_delete(ctx, node);
}


/* the following in-order-traversal does not use a stack. */
/* It's called Morris Traversal */
void btrbc_iterate_in_order(btrbc_ctx_t *ctx, btrbc_iterator_cb cb)
{
        btrbc_node_t *current, *pre;

        if (*ctx->root == NULL || btrbc_is_nil(ctx, *ctx->root)) {
                return;
        }

        current = *ctx->root;
        while (!btrbc_is_nil(ctx, current)) {
                if (btrbc_is_nil(ctx, P64(current->left))) {
                        cb(current);
                        current = P64(current->right);
                } else {
                        pre = P64(current->left);
                        while (!btrbc_is_nil(ctx, P64(pre->right)) &&
                               pre->right != P32(current)) {
                                pre = P64(pre->right);
                        }

                        if (btrbc_is_nil(ctx, P64(pre->right))) {
                                pre->right = P32(current);
                                current    = P64(current->left);
                        } else {
                                pre->right = ctx->nil;
                                cb(current);
                                current = P64(current->right);
                        }
                }
        }
}


bool btrbc_is_nil(btrbc_ctx_t *ctx, btrbc_node_t *n)
{
        return n == P64(ctx->nil);
}


btrbc_node_t *btrbc_max(btrbc_ctx_t *ctx)
{
        btrbc_node_t *n = *ctx->root;
        if (!n || btrbc_is_nil(ctx, n)) {
                return n;
        }
        while (!btrbc_is_nil(ctx, P64(n->right))) {
                n = P64(n->right);
        }
        return n;
}


btrbc_node_t *btrbc_min(btrbc_ctx_t *ctx)
{
        btrbc_node_t *n = *ctx->root;

        if (!n || btrbc_is_nil(ctx, n)) {
                return n;
        }
        while (!btrbc_is_nil(ctx, P64(n->left))) {
                n = P64(n->left);
        }
        return n;
}


btrbc_node_t *btrbc_min_at_least(btrbc_ctx_t *ctx, BT_RBC_VAL_TYPE n)
{
        btrbc_node_t *node = btrbc_min(ctx);

        if (!node || btrbc_is_nil(ctx, node)) {
                return node;
        }
        while (node && node->val < n) {
                node = btrbc_next_larger(ctx, node);
        }

        if (!node) {
                return NULL;
        }

        return node;
}


btrbc_node_t *btrbc_max_at_most(btrbc_ctx_t *ctx, BT_RBC_VAL_TYPE n)
{
        btrbc_node_t *node = btrbc_max(ctx);

        if (!node || btrbc_is_nil(ctx, node)) {
                return node;
        }
        while (node && node->val > n) {
                node = btrbc_next_smaller(ctx, node);
        }

        if (!node) {
                return NULL;
        }

        return node;
}


btrbc_node_t *btrbc_next_smaller(btrbc_ctx_t *ctx, btrbc_node_t *node)
{
        btrbc_node_t *tmp = node;

        if (btrbc_is_nil(ctx, node)) {
                return NULL;
        }

        if (!btrbc_is_nil(ctx, P64(tmp->left))) {
                tmp = P64(tmp->left);
                while (!btrbc_is_nil(ctx, P64(tmp->left))) {
                        tmp = P64(tmp->right);
                }
                return tmp;
        }

        while (!btrbc_is_nil(ctx, P64(tmp->parent))) {
                if (P64(tmp->parent)->right == P32(tmp)) {
                        return P64(tmp->parent);
                }
                tmp = P64(tmp->parent);
        }

        return NULL;
}


btrbc_node_t *btrbc_next_larger(btrbc_ctx_t *ctx, btrbc_node_t *node)
{
        btrbc_node_t *tmp = node;

        if (btrbc_is_nil(ctx, node)) {
                return NULL;
        }

        if (!btrbc_is_nil(ctx, P64(tmp->right))) {
                tmp = P64(tmp->right);
                while (!btrbc_is_nil(ctx, P64(tmp->left))) {
                        tmp = P64(tmp->left);
                }
                return tmp;
        }

        while (!btrbc_is_nil(ctx, P64(tmp->parent))) {
                if (P64(tmp->parent)->left == P32(tmp)) {
                        return P64(tmp->parent);
                }
                tmp = P64(tmp->parent);
        }

        return NULL;
}
