/* Red-Black Tree
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_RBT_H
#define EC_RBT_H

#include "ec_dt.h"
typedef ec_size_t ec_rbt_size_t;

/* Node */

typedef enum
{
    EC_RBT_NODE_COLOR_RED,
    EC_RBT_NODE_COLOR_BLACK,
} ec_rbt_node_color_t;

struct ec_rbt_node
{
    void *key, *value;
    struct ec_rbt_node *left, *right, *parent;
    ec_rbt_node_color_t color;
};
typedef struct ec_rbt_node ec_rbt_node_t;

/* Callbacks */
typedef void* (*ec_rbt_malloc_cb_t)(ec_rbt_size_t size);
typedef void (*ec_rbt_free_cb_t)(void* ptr);

typedef int (*ec_rbt_node_ctor_cb_t)(void* _this_key, void* key);
typedef void (*ec_rbt_node_dtor_cb_t)(void* _this_key);
typedef int (*ec_rbt_cmp_cb_t)(void* key1, void* key2);

/* Tree */

struct ec_rbt
{
    ec_rbt_node_t* root;
    ec_rbt_node_t* nil;

    ec_rbt_malloc_cb_t cb_malloc;
    ec_rbt_free_cb_t cb_free;

    ec_size_t key_size;
    ec_rbt_node_ctor_cb_t cb_key_ctor;
    ec_rbt_node_dtor_cb_t cb_key_dtor;

    ec_size_t value_size;
    ec_rbt_node_ctor_cb_t cb_value_ctor;
    ec_rbt_node_dtor_cb_t cb_value_dtor;

    ec_rbt_cmp_cb_t cb_cmp;
};
typedef struct ec_rbt ec_rbt_t;

/* Create and destroy */
ec_rbt_t* ec_rbt_new(ec_rbt_malloc_cb_t cb_malloc, ec_rbt_free_cb_t cb_free,
                     ec_size_t key_size, ec_rbt_node_ctor_cb_t cb_key_ctor,
                     ec_rbt_node_dtor_cb_t cb_key_dtor, ec_size_t value_size,
                     ec_rbt_node_ctor_cb_t cb_value_ctor,
                     ec_rbt_node_dtor_cb_t cb_value_dtor,
                     ec_rbt_cmp_cb_t cb_cmp);
void ec_rbt_destroy(ec_rbt_t* ec_rbt);

/* Clear */
void ec_rbt_clear(ec_rbt_t* ec_rbt);

/* Get number of nodes in tree */
int ec_rbt_size(ec_rbt_t* ec_rbt);

/* Operations */

int ec_rbt_insert(ec_rbt_t* ec_rbt, void* key, void* value);
int ec_rbt_search_key(ec_rbt_t* ec_rbt, void** key_out, void* key);
int ec_rbt_search(ec_rbt_t* ec_rbt, void** value_out, void* key);
ec_rbt_node_t* ec_rbt_minimum(ec_rbt_t* ec_rbt, ec_rbt_node_t* x);
ec_rbt_node_t* ec_rbt_maximum(ec_rbt_t* ec_rbt, ec_rbt_node_t* x);
ec_rbt_node_t* ec_rbt_successor(ec_rbt_t* ec_rbt, ec_rbt_node_t* x);
ec_rbt_node_t* ec_rbt_predecessor(ec_rbt_t* ec_rbt, ec_rbt_node_t* x);
ec_rbt_node_t* ec_rbt_remove(ec_rbt_t* ec_rbt, void* key);

/* Iterator */

typedef struct ec_rbt_iterator
{
    ec_rbt_node_t* node_cur;
    ec_rbt_t* ec_rbt;
} ec_rbt_iterator_t;

int ec_rbt_iterator_init(ec_rbt_t* ec_rbt, ec_rbt_iterator_t* iterator);
void ec_rbt_iterator_next(ec_rbt_iterator_t* iterator);
ec_rbt_node_t* ec_rbt_iterator_deref(ec_rbt_iterator_t* iterator);
void* ec_rbt_iterator_deref_key(ec_rbt_iterator_t* iterator);
void* ec_rbt_iterator_deref_value(ec_rbt_iterator_t* iterator);
int ec_rbt_iterator_isend(ec_rbt_iterator_t* iterator);
int ec_rbt_iterator_eq(ec_rbt_iterator_t* iter1, ec_rbt_iterator_t* iter2);

typedef int (*ec_rbt_iterator_callback_t)(void* key, void* value, void* data);

int ec_rbt_iterate(ec_rbt_t* ec_rbt, ec_rbt_iterator_callback_t callback,
                   void* data);

#endif
