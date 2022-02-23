/* Red-Black Tree
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_rbt.h"
#include "ec_alloc.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

static int ec_rbt_search_node(ec_rbt_t* ec_rbt, ec_rbt_node_t** node_out,
                              void* key);

/* Node */

static ec_rbt_node_t* ec_rbt_node_new(ec_rbt_t* ec_rbt, void* key, void* value)
{
    ec_rbt_node_t* new_node =
        (ec_rbt_node_t*)ec_rbt->cb_malloc(sizeof(ec_rbt_node_t));
    if (new_node == NULL)
        return NULL;

    new_node->key = key;
    new_node->value = value;

    new_node->left = new_node->right = new_node->parent = NULL;
    new_node->color = EC_RBT_NODE_COLOR_BLACK;
    return new_node;
}

static void ec_rbt_node_destroy(ec_rbt_t* ec_rbt, ec_rbt_node_t* node)
{
    if (node->key != NULL)
    {
        if (ec_rbt->cb_key_dtor != NULL)
        {
            ec_rbt->cb_key_dtor(node->key);
        }
        ec_rbt->cb_free(node->key);
    }
    if (node->value != NULL)
    {
        if (ec_rbt->cb_value_dtor != NULL)
        {
            ec_rbt->cb_value_dtor(node->value);
        }
        ec_rbt->cb_free(node->value);
    }
    ec_rbt->cb_free(node);
}

/* Get number of nodes in tree */
int ec_rbt_size(ec_rbt_t* ec_rbt)
{
    int count = 0;
    ec_rbt_iterator_t iter;

    /* TODO: To be optimized */

    for (ec_rbt_iterator_init(ec_rbt, &iter); !ec_rbt_iterator_isend(&iter);
         ec_rbt_iterator_next(&iter))
    {
        count++;
    }

    return count;
}

static void ec_rbt_left_rotate(ec_rbt_t* ec_rbt, ec_rbt_node_t* x)
{
    ec_rbt_node_t* y = x->right;
    x->right = y->left;
    if (y->left != ec_rbt->nil)
        y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == ec_rbt->nil)
    {
        ec_rbt->root = y;
    }
    else
    {
        if (x == x->parent->left)
        {
            x->parent->left = y;
        }
        else
        {
            x->parent->right = y;
        }
    }
    y->left = x;
    x->parent = y;
}

static void ec_rbt_right_rotate(ec_rbt_t* ec_rbt, ec_rbt_node_t* x)
{
    ec_rbt_node_t* y = x->left;
    x->left = y->right;
    if (y->right != ec_rbt->nil)
        y->right->parent = x;
    y->parent = x->parent;
    if (x->parent == ec_rbt->nil)
    {
        ec_rbt->root = y;
    }
    else
    {
        if (x == x->parent->right)
        {
            x->parent->right = y;
        }
        else
        {
            x->parent->left = y;
        }
    }
    y->right = x;
    x->parent = y;
}

static void ec_rbt_insert_fixup(ec_rbt_t* ec_rbt, ec_rbt_node_t* z)
{
    ec_rbt_node_t* y = NULL;
    while (z->parent->color == EC_RBT_NODE_COLOR_RED)
    {
        if (z->parent == z->parent->parent->left)
        {
            y = z->parent->parent->right;
            if (y->color == EC_RBT_NODE_COLOR_RED)
            {
                z->parent->color = EC_RBT_NODE_COLOR_BLACK;
                y->color = EC_RBT_NODE_COLOR_BLACK;
                z->parent->parent->color = EC_RBT_NODE_COLOR_RED;
                z = z->parent->parent;
            }
            else
            {
                if (z == z->parent->right)
                {
                    z = z->parent;
                    ec_rbt_left_rotate(ec_rbt, z);
                }
                z->parent->color = EC_RBT_NODE_COLOR_BLACK;
                z->parent->parent->color = EC_RBT_NODE_COLOR_RED;
                ec_rbt_right_rotate(ec_rbt, z->parent->parent);
            }
        }
        else if (z->parent == z->parent->parent->right)
        {
            y = z->parent->parent->left;
            if (y->color == EC_RBT_NODE_COLOR_RED)
            {
                z->parent->color = EC_RBT_NODE_COLOR_BLACK;
                y->color = EC_RBT_NODE_COLOR_BLACK;
                z->parent->parent->color = EC_RBT_NODE_COLOR_RED;
                z = z->parent->parent;
            }
            else
            {
                if (z == z->parent->left)
                {
                    z = z->parent;
                    ec_rbt_right_rotate(ec_rbt, z);
                }
                z->parent->color = EC_RBT_NODE_COLOR_BLACK;
                z->parent->parent->color = EC_RBT_NODE_COLOR_RED;
                ec_rbt_left_rotate(ec_rbt, z->parent->parent);
            }
        }
    }
    ec_rbt->root->color = EC_RBT_NODE_COLOR_BLACK;
}

/*
static int ec_rbt_cmp_default(ec_rbt_node_data_t *key1, ec_rbt_node_data_t
*key2)
{
    if ((key1->ptr)==(key2->ptr)) return 0;
    return ((key1->ptr)<(key2->ptr)) ? -1 : 1;
}
*/

/* Tree */

ec_rbt_t* ec_rbt_new(ec_rbt_malloc_cb_t cb_malloc, ec_rbt_free_cb_t cb_free,
                     ec_size_t key_size, ec_rbt_node_ctor_cb_t cb_key_ctor,
                     ec_rbt_node_dtor_cb_t cb_key_dtor, ec_size_t value_size,
                     ec_rbt_node_ctor_cb_t cb_value_ctor,
                     ec_rbt_node_dtor_cb_t cb_value_dtor,
                     ec_rbt_cmp_cb_t cb_cmp)
{
    ec_rbt_t* new_ec_rbt = (ec_rbt_t*)cb_malloc(sizeof(ec_rbt_t));
    if (new_ec_rbt == NULL)
        return NULL;

    /* Allocators and other callbacks */
    new_ec_rbt->cb_malloc = cb_malloc;
    new_ec_rbt->cb_free = cb_free;
    new_ec_rbt->key_size = key_size;
    new_ec_rbt->cb_key_ctor = cb_key_ctor;
    new_ec_rbt->cb_key_dtor = cb_key_dtor;
    new_ec_rbt->value_size = value_size;
    new_ec_rbt->cb_value_ctor = cb_value_ctor;
    new_ec_rbt->cb_value_dtor = cb_value_dtor;
    new_ec_rbt->cb_cmp = cb_cmp;

    new_ec_rbt->nil = ec_rbt_node_new(new_ec_rbt, NULL, NULL);
    if (new_ec_rbt->nil == NULL)
    {
        cb_free(new_ec_rbt);
        return NULL;
    }
    new_ec_rbt->nil->left = new_ec_rbt->nil->right = new_ec_rbt->nil->parent =
        new_ec_rbt->nil;
    new_ec_rbt->root = new_ec_rbt->nil;

    return new_ec_rbt;
}

static void ec_rbt_destroy_in(ec_rbt_t* ec_rbt, ec_rbt_node_t* node)
{
    if (node->left != ec_rbt->nil)
    {
        ec_rbt_destroy_in(ec_rbt, node->left);
    }
    if (node->right != ec_rbt->nil)
    {
        ec_rbt_destroy_in(ec_rbt, node->right);
    }
    ec_rbt_node_destroy(ec_rbt, node);
}

void ec_rbt_destroy(ec_rbt_t* ec_rbt)
{
    if (ec_rbt->root != ec_rbt->nil)
    {
        ec_rbt_destroy_in(ec_rbt, ec_rbt->root);
    }
    ec_rbt_node_destroy(ec_rbt, ec_rbt->nil);
    ec_rbt->cb_free(ec_rbt);
}

void ec_rbt_clear(ec_rbt_t* ec_rbt)
{
    if (ec_rbt->root != ec_rbt->nil)
    {
        ec_rbt_destroy_in(ec_rbt, ec_rbt->root);
    }
    ec_rbt->root = ec_rbt->nil;
}

/* Operations */

int ec_rbt_insert(ec_rbt_t* ec_rbt, void* key, void* value)
{
    ec_rbt_node_t *x = ec_rbt->root, *y = ec_rbt->nil;
    ec_rbt_node_t* z = NULL;
    int cmp_result;
    /* Prevent key collision */
    {
        ec_rbt_node_t* node_target = NULL;
        ec_rbt_search_node(ec_rbt, &node_target, key);
        if (node_target != NULL)
            return 0;
    }
    /* Clone the key and value */
    {
        void *clone_key = NULL, *clone_value = NULL;

        if ((clone_key = ec_rbt->cb_malloc(ec_rbt->key_size)) == NULL)
            return -1;
        if (ec_rbt->cb_key_ctor(clone_key, key) != 0)
        {
            ec_rbt->cb_free(clone_key);
            return -1;
        }
        if (ec_rbt->cb_value_ctor != NULL)
        {
            if ((clone_value = ec_rbt->cb_malloc(ec_rbt->value_size)) == NULL)
            {
                if (ec_rbt->cb_key_dtor != NULL)
                {
                    ec_rbt->cb_key_dtor(clone_key);
                }
                ec_rbt->cb_free(clone_key);
                return -1;
            }
            if (ec_rbt->cb_value_ctor(clone_value, value) != 0)
            {
                if (ec_rbt->cb_key_dtor != NULL)
                {
                    ec_rbt->cb_key_dtor(clone_key);
                }
                ec_rbt->cb_free(clone_key);
                ec_rbt->cb_free(clone_value);
                return -1;
            }
        }
        if ((z = ec_rbt_node_new(ec_rbt, clone_key, clone_value)) == NULL)
        {
            if (ec_rbt->cb_key_dtor != NULL)
            {
                ec_rbt->cb_key_dtor(clone_key);
            }
            ec_rbt->cb_free(clone_key);
            if (ec_rbt->cb_value_dtor != NULL)
            {
                ec_rbt->cb_value_dtor(clone_value);
            }
            ec_rbt->cb_free(clone_value);
            return -1;
        }
    }
    z->parent = z->left = z->right = ec_rbt->nil;
    z->color = EC_RBT_NODE_COLOR_RED;
    while (x != ec_rbt->nil)
    {
        y = x;
        cmp_result = ec_rbt->cb_cmp(z->key, x->key);
        if (cmp_result == 0)
            return 0;
        x = (cmp_result < 0) ? x->left : x->right;
    }
    z->parent = y;
    if (y == ec_rbt->nil)
    {
        ec_rbt->root = z;
    }
    else
    {
        if (ec_rbt->cb_cmp(z->key, y->key) < 0)
            y->left = z;
        else
            y->right = z;
    }
    ec_rbt_insert_fixup(ec_rbt, z);
    return 0;
}

static int ec_rbt_search_node(ec_rbt_t* ec_rbt, ec_rbt_node_t** node_out,
                              void* key)
{
    ec_rbt_node_t* node_cur = ec_rbt->root;
    int result;

    while (node_cur != ec_rbt->nil)
    {
        result = ec_rbt->cb_cmp(key, node_cur->key);
        switch (result)
        {
        case 0:
            *node_out = node_cur;
            return 0;
        case -1:
            node_cur = node_cur->left;
            break;
        case 1:
            node_cur = node_cur->right;
            break;
        }
    }

    return -1;
}

int ec_rbt_search_key(ec_rbt_t* ec_rbt, void** key_out, void* key)
{
    int ret = 0;
    ec_rbt_node_t* node_target = NULL;
    ret = ec_rbt_search_node(ec_rbt, &node_target, key);
    if (node_target != NULL)
        *key_out = &node_target->key;
    return ret;
}

int ec_rbt_search(ec_rbt_t* ec_rbt, void** value_out, void* key)
{
    int ret = 0;
    ec_rbt_node_t* node_target = NULL;
    ret = ec_rbt_search_node(ec_rbt, &node_target, key);
    if (node_target != NULL)
        *value_out = &node_target->value;
    return ret;
}

ec_rbt_node_t* ec_rbt_minimum(ec_rbt_t* ec_rbt, ec_rbt_node_t* x)
{
    while (x->left != ec_rbt->nil)
    {
        x = x->left;
    }
    return x;
}

ec_rbt_node_t* ec_rbt_maximum(ec_rbt_t* ec_rbt, ec_rbt_node_t* x)
{
    while (x->right != ec_rbt->nil)
    {
        x = x->right;
    }
    return x;
}

ec_rbt_node_t* ec_rbt_successor(ec_rbt_t* ec_rbt, ec_rbt_node_t* x)
{
    ec_rbt_node_t* y;
    if (x->right != ec_rbt->nil)
        return ec_rbt_minimum(ec_rbt, x->right);
    y = x->parent;
    while ((y != ec_rbt->nil) && (x == y->right))
    {
        x = y;
        y = y->parent;
    }
    return y;
}

ec_rbt_node_t* ec_rbt_predecessor(ec_rbt_t* ec_rbt, ec_rbt_node_t* x)
{
    ec_rbt_node_t* y;
    if (x->left != ec_rbt->nil)
        return ec_rbt_maximum(ec_rbt, x->left);
    y = x->parent;
    while ((y != ec_rbt->nil) && (x == y->left))
    {
        x = y;
        y = y->parent;
    }
    return y;
}

static void ec_rbt_remove_node_fixup(ec_rbt_t* ec_rbt, ec_rbt_node_t* x)
{
    ec_rbt_node_t* w;
    while ((x != ec_rbt->root) && (x->color == EC_RBT_NODE_COLOR_BLACK))
    {
        if (x == x->parent->left)
        {
            w = x->parent->right;
            if (w->color == EC_RBT_NODE_COLOR_RED)
            {
                w->color = EC_RBT_NODE_COLOR_BLACK;
                x->parent->color = EC_RBT_NODE_COLOR_RED;
                ec_rbt_left_rotate(ec_rbt, x->parent);
                w = x->parent->right;
            }
            if ((w->left->color == EC_RBT_NODE_COLOR_BLACK) &&
                (w->right->color == EC_RBT_NODE_COLOR_BLACK))
            {
                w->color = EC_RBT_NODE_COLOR_RED;
                x = x->parent;
            }
            else
            {
                if (w->right->color == EC_RBT_NODE_COLOR_BLACK)
                {
                    w->left->color = EC_RBT_NODE_COLOR_BLACK;
                    w->color = EC_RBT_NODE_COLOR_RED;
                    ec_rbt_right_rotate(ec_rbt, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = EC_RBT_NODE_COLOR_BLACK;
                w->right->color = EC_RBT_NODE_COLOR_BLACK;
                ec_rbt_left_rotate(ec_rbt, x->parent);
                x = ec_rbt->root;
            }
        }
        else
        {
            w = x->parent->left;
            if (w->color == EC_RBT_NODE_COLOR_RED)
            {
                w->color = EC_RBT_NODE_COLOR_BLACK;
                x->parent->color = EC_RBT_NODE_COLOR_RED;
                ec_rbt_right_rotate(ec_rbt, x->parent);
                w = x->parent->left;
            }
            if ((w->right->color == EC_RBT_NODE_COLOR_BLACK) &&
                (w->left->color == EC_RBT_NODE_COLOR_BLACK))
            {
                w->color = EC_RBT_NODE_COLOR_RED;
                x = x->parent;
            }
            else
            {
                if (w->left->color == EC_RBT_NODE_COLOR_BLACK)
                {
                    w->right->color = EC_RBT_NODE_COLOR_BLACK;
                    w->color = EC_RBT_NODE_COLOR_RED;
                    ec_rbt_left_rotate(ec_rbt, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = EC_RBT_NODE_COLOR_BLACK;
                w->left->color = EC_RBT_NODE_COLOR_BLACK;
                ec_rbt_right_rotate(ec_rbt, x->parent);
                x = ec_rbt->root;
            }
        }
    }
    x->color = EC_RBT_NODE_COLOR_BLACK;
}

static void ec_rbt_transplant(ec_rbt_t* ec_rbt, ec_rbt_node_t* u,
                              ec_rbt_node_t* v)
{
    if (u->parent == ec_rbt->nil)
    {
        ec_rbt->root = v;
    }
    else if (u == u->parent->left)
    {
        u->parent->left = v;
    }
    else
    {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

static ec_rbt_node_t* ec_rbt_remove_node(ec_rbt_t* ec_rbt, ec_rbt_node_t* z)
{
    ec_rbt_node_t *x, *y;
    ec_rbt_node_color_t y_original_color;

    y = z;
    y_original_color = y->color;

    if (z->left == ec_rbt->nil)
    {
        x = z->right;
        ec_rbt_transplant(ec_rbt, z, z->right);
    }
    else if (z->right == ec_rbt->nil)
    {
        x = z->left;
        ec_rbt_transplant(ec_rbt, z, z->left);
    }
    else
    {
        y = ec_rbt_minimum(ec_rbt, z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z)
        {
            x->parent = y;
        }
        else
        {
            ec_rbt_transplant(ec_rbt, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        ec_rbt_transplant(ec_rbt, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }
    if (y_original_color == EC_RBT_NODE_COLOR_BLACK)
    {
        ec_rbt_remove_node_fixup(ec_rbt, x);
    }

    if (ec_rbt->cb_key_dtor != NULL)
    {
        ec_rbt->cb_key_dtor(z->key);
        ec_rbt->cb_free(z->key);
    }
    if (ec_rbt->cb_value_dtor != NULL)
    {
        ec_rbt->cb_value_dtor(z->value);
        ec_rbt->cb_free(z->value);
    }
    ec_rbt->cb_free(z);
    return y;
}

ec_rbt_node_t* ec_rbt_remove(ec_rbt_t* ec_rbt, void* key)
{
    ec_rbt_node_t* node_target = NULL;

    /* Search the node */
    ec_rbt_search_node(ec_rbt, &node_target, key);
    if (node_target == NULL)
        return NULL;

    /* Remove the node */
    return ec_rbt_remove_node(ec_rbt, node_target);
}

/* Iterator */

int ec_rbt_iterator_init(ec_rbt_t* ec_rbt, ec_rbt_iterator_t* iterator)
{
    iterator->node_cur = ec_rbt_minimum(ec_rbt, ec_rbt->root);
    iterator->ec_rbt = ec_rbt;
    return 0;
}

void ec_rbt_iterator_next(ec_rbt_iterator_t* iterator)
{
    iterator->node_cur = ec_rbt_successor(iterator->ec_rbt, iterator->node_cur);
}

ec_rbt_node_t* ec_rbt_iterator_deref(ec_rbt_iterator_t* iterator)
{
    return iterator->node_cur->key;
}

void* ec_rbt_iterator_deref_key(ec_rbt_iterator_t* iterator)
{
    return iterator->node_cur->key;
}

void* ec_rbt_iterator_deref_value(ec_rbt_iterator_t* iterator)
{
    return iterator->node_cur->value;
}

int ec_rbt_iterator_isend(ec_rbt_iterator_t* iterator)
{
    return iterator->node_cur == iterator->ec_rbt->nil ? 1 : 0;
}

int ec_rbt_iterator_eq(ec_rbt_iterator_t* iter1, ec_rbt_iterator_t* iter2)
{
    return ((iter1->ec_rbt == iter2->ec_rbt) &&
            (iter1->node_cur == iter2->node_cur))
               ? ec_true
               : ec_false;
}

int ec_rbt_iterate(ec_rbt_t* ec_rbt, ec_rbt_iterator_callback_t callback,
                   void* data)
{
    int ret = 0;
    ec_rbt_iterator_t iter;
    for (ec_rbt_iterator_init(ec_rbt, &iter); !ec_rbt_iterator_isend(&iter);
         ec_rbt_iterator_next(&iter))
    {
        if ((ret = callback(&iter.node_cur->key, &iter.node_cur->value,
                            data)) != 0)
        {
            return ret;
        }
    }
    return 0;
}
