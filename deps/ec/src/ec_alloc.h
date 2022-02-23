/* Enhanced C : Allocator
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_ALLOC_H
#define EC_ALLOC_H

#include "ec_dt.h"

typedef void* (*ec_malloc_cb_t)(ec_size_t size);
typedef void* (*ec_calloc_cb_t)(ec_size_t nmemb, ec_size_t size);
typedef void (*ec_free_cb_t)(void* ptr);
typedef void* (*ec_memcpy_cb_t)(void* dest, const void* src, ec_size_t n);
typedef void* (*ec_memset_cb_t)(void* s, int c, ec_size_t n);

void ec_allocator_set_malloc(ec_malloc_cb_t cb);
void ec_allocator_set_calloc(ec_calloc_cb_t cb);
void ec_allocator_set_free(ec_free_cb_t cb);
void ec_allocator_set_memcpy(ec_memcpy_cb_t cb);
void ec_allocator_set_memset(ec_memset_cb_t cb);

void* ec_malloc(ec_size_t size);
void* ec_calloc(ec_size_t nmemb, ec_size_t size);
void ec_free(void* ptr);
void* ec_memcpy(void* dest, const void* src, ec_size_t n);
void* ec_memset(void* s, int c, ec_size_t n);

typedef void (*ec_ctor_t)(void* ptr);
typedef void (*ec_dtor_t)(void* ptr);

void* __ec_new_raw(ec_size_t size, ec_size_t count, ec_ctor_t ctor,
                   ec_dtor_t dtor);

/* new : 1 with nothing */
#define ec_new(_type) ((_type*)__ec_new_raw(sizeof(_type), 1, NULL, NULL))

/* new : 1 with ctor */
#define ec_newc(_type, _ctor)                                                  \
    ((_type*)__ec_new_raw(sizeof(_type), 1, _ctor, NULL))

/* new : 1 with dtor */
#define ec_newd(_type, _dtor)                                                  \
    ((_type*)__ec_new_raw(sizeof(_type), 1, NULL, _dtor))

/* new : 1 with ctor and dtor */
#define ec_newcd(_type, _ctor, _dtor)                                          \
    ((_type*)__ec_new_raw(sizeof(_type), 1, (ec_ctor_t)_ctor, (ec_dtor_t)_dtor))

/* new : n without ctor and dtor */
#define ec_newn(_type, _count)                                                 \
    ((_type*)__ec_new_raw(sizeof(_type), (_count), NULL, NULL))

/* new : n with ctor */
#define ec_newnc(_type, _count, _ctor)                                         \
    ((_type*)__ec_new_raw(sizeof(_type), (_count), _ctor, NULL))

/* new : n with dtor */
#define ec_newnd(_type, _count, _ctor)                                         \
    ((_type*)__ec_new_raw(sizeof(_type), (_count), NULL, _dtor))

/* new : n with ctor and dtor */
#define ec_newncd(_type, _count, _ctor)                                        \
    ((_type*)__ec_new_raw(sizeof(_type), (_count), _ctor, _dtor))

/* delete */
void ec_delete(void* ptr);

#endif
