#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "btrb.h"

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

static btrb_node_t nil_node = {
    .parent = NULL, .left = NULL, .right = NULL, .val = 0, .color = BLACK};


btrb_node_t *btrb_nil(void)
{
        return &nil_node;
}


void btrb_left_rotate(btrb_node_t **root, btrb_node_t *x)
{
        btrb_node_t *y = x->right;
        x->right       = y->left;
        if (y->left != &nil_node) {
                y->left->parent = x;
        }
        y->parent = x->parent;
        if (x->parent == &nil_node) {
                *root = y;
        } else if (x == x->parent->left) {
                x->parent->left = y;
        } else {
                x->parent->right = y;
        }
        y->left   = x;
        x->parent = y;
}


void btrb_right_rotate(btrb_node_t **root, btrb_node_t *y)
{
        btrb_node_t *x = y->left;
        y->left        = x->right;
        if (x->right != &nil_node) {
                x->right->parent = y;
        }
        x->parent = y->parent;
        if (y->parent == &nil_node) {
                *root = x;
        } else if (y == y->parent->right) {
                y->parent->right = x;
        } else {
                y->parent->left = x;
        }
        x->right  = y;
        y->parent = x;
}


void btrb_transplant(btrb_node_t **root, btrb_node_t *u, btrb_node_t *v)
{
        if (u->parent == &nil_node) {
                *root = v;
        } else if (u == u->parent->left) {
                u->parent->left = v;
        } else {
                u->parent->right = v;
        }
        v->parent = u->parent;
}


void btrb_delete_fixup(btrb_node_t **root, btrb_node_t *x)
{
        if (*root == &nil_node) {
                return;
        }
        while (x != *root && x->color == BLACK) {
                if (x == x->parent->left) {
                        btrb_node_t *w = x->parent->right;
                        if (w->color == RED) {
                                w->color         = BLACK;
                                x->parent->color = RED;
                                btrb_left_rotate(root, x->parent);
                                w = x->parent->right;
                        }
                        if (w->left->color == BLACK &&
                            w->right->color == BLACK) {
                                w->color = RED;
                                x        = x->parent;
                        } else {
                                if (w->right->color == BLACK) {
                                        w->left->color = BLACK;
                                        w->color       = RED;
                                        btrb_right_rotate(root, w);
                                        w = x->parent->right;
                                }
                                w->color         = x->parent->color;
                                x->parent->color = BLACK;
                                w->right->color  = BLACK;
                                btrb_left_rotate(root, x->parent);
                                x = *root;
                        }
                } else {
                        btrb_node_t *w = x->parent->left;
                        if (w->color == RED) {
                                w->color         = BLACK;
                                x->parent->color = RED;
                                btrb_right_rotate(root, x->parent);
                                w = x->parent->left;
                        }
                        if (w->right->color == BLACK &&
                            w->left->color == BLACK) {
                                w->color = RED;
                                x        = x->parent;
                        } else {
                                if (w->left->color == BLACK) {
                                        w->right->color = BLACK;
                                        w->color        = RED;
                                        btrb_left_rotate(root, w);
                                        w = x->parent->left;
                                }
                                w->color         = x->parent->color;
                                x->parent->color = BLACK;
                                w->left->color   = BLACK;
                                btrb_right_rotate(root, x->parent);
                                x = *root;
                        }
                }
        }
        x->color = BLACK;
}


static btrb_node_t *tree_minimum(btrb_node_t *x)
{
        while (x->left != &nil_node) {
                x = x->left;
        }
        return x;
}


void btrb_delete(btrb_node_t **root, btrb_node_t *z)
{
        btrb_node_t *y = z;
        btrb_node_t *x;
        bt_color_t y_original_color = y->color;

        if (z->left == &nil_node) {
                x = z->right;
                btrb_transplant(root, z, z->right);
        } else if (z->right == &nil_node) {
                x = z->left;
                btrb_transplant(root, z, z->left);
        } else {
                y                = tree_minimum(z->right);
                y_original_color = y->color;
                x                = y->right;
                if (y->parent == z) {
                        x->parent = y;
                } else {
                        btrb_transplant(root, y, y->right);
                        y->right         = z->right;
                        y->right->parent = y;
                }
                btrb_transplant(root, z, y);
                y->left         = z->left;
                y->left->parent = y;
                y->color        = z->color;
        }
        if (y_original_color == BLACK) {
                btrb_delete_fixup(root, x);
        }
}


btrb_node_t *btrb_search(btrb_node_t **root, BT_RB_VAL_TYPE n)
{
        btrb_node_t *x = *root;

        while (x != &nil_node && x->val != n) {
                if (x->val < n) {
                        x = x->right;
                }
                if (x->val > n) {
                        x = x->left;
                }
        }
        if (x == &nil_node) {
                return NULL;
        }
        return x;
}


void btrb_insert_fixup(btrb_node_t **root, btrb_node_t *z)
{
        btrb_node_t *y;
        while (z->parent->color == RED) {
                if (z->parent == z->parent->parent->left) {
                        y = z->parent->parent->right;
                        if (y->color == RED) {
                                z->parent->color         = BLACK;
                                y->color                 = BLACK;
                                z->parent->parent->color = RED;
                                z                        = z->parent->parent;
                        } else {
                                if (z == z->parent->right) {
                                        z = z->parent;
                                        btrb_left_rotate(root, z);
                                }
                                z->parent->color         = BLACK;
                                z->parent->parent->color = RED;
                                btrb_right_rotate(root, z->parent->parent);
                        }
                } else {
                        y = z->parent->parent->left;
                        if (y->color == RED) {
                                z->parent->color         = BLACK;
                                y->color                 = BLACK;
                                z->parent->parent->color = RED;
                                z                        = z->parent->parent;
                        } else {
                                if (z == z->parent->left) {
                                        z = z->parent;
                                        btrb_right_rotate(root, z);
                                }
                                z->parent->color         = BLACK;
                                z->parent->parent->color = RED;
                                btrb_left_rotate(root, z->parent->parent);
                        }
                }
        }
        (*root)->color = BLACK;
}


void btrb_insert(btrb_node_t **root, BT_RB_VAL_TYPE n, void *user_data,
                 btrb_node_t *z)
{
        if (*root == NULL) {
                *root = &nil_node;
        }

        btrb_node_t *y = &nil_node;
        btrb_node_t *x = *root;
        z->val         = n;
        while (x != &nil_node) {
                y = x;
                if (z->val < x->val) {
                        x = x->left;
                } else {
                        x = x->right;
                }
        }
        z->parent = y;
        if (y == &nil_node) {
                *root = z;
        } else if (z->val < y->val) {
                y->left = z;
        } else {
                y->right = z;
        }
        z->left      = &nil_node;
        z->right     = &nil_node;
        z->color     = RED;
        z->user_data = user_data;
        btrb_insert_fixup(root, z);
}


void btrb_delete_by_val(btrb_node_t **root, BT_RB_VAL_TYPE n)
{
        btrb_node_t *node = btrb_search(root, n);

        if (!n) {
                return;
        }

        btrb_delete(root, node);
}


/* the following in-order-traversal does not use a stack. */
/* It's called Morris Traversal */
void btrb_iterate_in_order(btrb_node_t *root, btrb_iterator_cb cb)
{
        btrb_node_t *current, *pre;

        if (root == NULL || root == &nil_node) {
                return;
        }

        current = root;
        while (current != &nil_node) {
                if (current->left == &nil_node) {
                        cb(current);
                        current = current->right;
                } else {
                        pre = current->left;
                        while (pre->right != &nil_node &&
                               pre->right != current) {
                                pre = pre->right;
                        }

                        if (pre->right == &nil_node) {
                                pre->right = current;
                                current    = current->left;
                        } else {
                                pre->right = &nil_node;
                                cb(current);
                                current = current->right;
                        }
                }
        }
}


bool btrb_is_nil(btrb_node_t *n)
{
        return n == &nil_node;
}


btrb_node_t *btrb_max(btrb_node_t **root)
{
        btrb_node_t *n = *root;
        if (!n || btrb_is_nil(n)) {
                return n;
        }
        while (!btrb_is_nil(n->right)) {
                n = n->right;
        }
        return n;
}


btrb_node_t *btrb_min(btrb_node_t **root)
{
        btrb_node_t *n = *root;

        if (!n || btrb_is_nil(n)) {
                return n;
        }
        while (!btrb_is_nil(n->left)) {
                n = n->left;
        }
        return n;
}


btrb_node_t *btrb_min_at_least(btrb_node_t **root, BT_RB_VAL_TYPE n)
{
        btrb_node_t *node = btrb_min(root);

        if (!node || btrb_is_nil(node)) {
                return node;
        }
        while (node && node->val < n) {
                node = btrb_next_larger(node);
        }

        if (!node) {
                return NULL;
        }

        return node;
}


btrb_node_t *btrb_max_at_most(btrb_node_t **root, BT_RB_VAL_TYPE n)
{
        btrb_node_t *node = btrb_max(root);

        if (!node || btrb_is_nil(node)) {
                return node;
        }
        while (node && node->val > n) {
                node = btrb_next_smaller(node);
        }

        if (!node) {
                return NULL;
        }

        return node;
}


btrb_node_t *btrb_next_smaller(btrb_node_t *node)
{
        btrb_node_t *tmp = node;

        if (btrb_is_nil(node)) {
                return NULL;
        }

        if (!btrb_is_nil(tmp->left)) {
                tmp = tmp->left;
                while (!btrb_is_nil(tmp->left)) {
                        tmp = tmp->right;
                }
                return tmp;
        }

        while (!btrb_is_nil(tmp->parent)) {
                if (tmp->parent->right == tmp) {
                        return tmp->parent;
                }
                tmp = tmp->parent;
        }

        return NULL;
}


btrb_node_t *btrb_next_larger(btrb_node_t *node)
{
        btrb_node_t *tmp = node;

        if (btrb_is_nil(node)) {
                return NULL;
        }

        if (!btrb_is_nil(tmp->right)) {
                tmp = tmp->right;
                while (!btrb_is_nil(tmp->left)) {
                        tmp = tmp->left;
                }
                return tmp;
        }

        while (!btrb_is_nil(tmp->parent)) {
                if (tmp->parent->left == tmp) {
                        return tmp->parent;
                }
                tmp = tmp->parent;
        }

        return NULL;
}
