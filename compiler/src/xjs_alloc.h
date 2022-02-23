/* XenonJS : Allocator
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_ALLOC_H
#define XJS_ALLOC_H

#include "xjs_dt.h"

typedef void *(*xjs_malloc_cb_t)(xjs_size_t size);
typedef void *(*xjs_calloc_cb_t)(xjs_size_t nmemb, xjs_size_t size);
typedef void (*xjs_free_cb_t)(void *ptr);
typedef void *(*xjs_memcpy_cb_t)(void *dest, const void *src, xjs_size_t n);
typedef void *(*xjs_memset_cb_t)(void *s, int c, xjs_size_t n);

void xjs_allocator_set_malloc(xjs_malloc_cb_t cb);
void xjs_allocator_set_calloc(xjs_calloc_cb_t cb);
void xjs_allocator_set_free(xjs_free_cb_t cb);
void xjs_allocator_set_memcpy(xjs_memcpy_cb_t cb);
void xjs_allocator_set_memset(xjs_memset_cb_t cb);

void *xjs_malloc(xjs_size_t size);
void *xjs_calloc(xjs_size_t nmemb, xjs_size_t size);
void xjs_free(void *ptr);
void *xjs_memcpy(void *dest, const void *src, xjs_size_t n);
void *xjs_memset(void *s, int c, xjs_size_t n);

typedef void (*xjs_ctor_t)(void *ptr);
typedef void (*xjs_dtor_t)(void *ptr);

void *__xjs_new_raw( \
        xjs_size_t size, \
        xjs_size_t count, \
        xjs_ctor_t ctor, \
        xjs_dtor_t dtor);

/* new : 1 with nothing */
#define xjs_new(_type) \
    ((_type *)__xjs_new_raw(sizeof(_type), 1, NULL, NULL))

/* new : 1 with ctor */
#define xjs_newc(_type, _ctor) \
    ((_type *)__xjs_new_raw(sizeof(_type), 1, _ctor, NULL))

/* new : 1 with dtor */
#define xjs_newd(_type, _dtor) \
    ((_type *)__xjs_new_raw(sizeof(_type), 1, NULL, _dtor))

/* new : 1 with ctor and dtor */
#define xjs_newcd(_type, _ctor, _dtor) \
    ((_type *)__xjs_new_raw(sizeof(_type), 1, (xjs_ctor_t)_ctor, (xjs_dtor_t)_dtor))


/* new : n without ctor and dtor */
#define xjs_newn(_type, _count) \
    ((_type *)__xjs_new_raw(sizeof(_type), (_count), NULL, NULL))

/* new : n with ctor */
#define xjs_newnc(_type, _count, _ctor) \
    ((_type *)__xjs_new_raw(sizeof(_type), (_count), _ctor, NULL))

/* new : n with dtor */
#define xjs_newnd(_type, _count, _ctor) \
    ((_type *)__xjs_new_raw(sizeof(_type), (_count), NULL, _dtor))

/* new : n with ctor and dtor */
#define xjs_newncd(_type, _count, _ctor) \
    ((_type *)__xjs_new_raw(sizeof(_type), (_count), _ctor, _dtor))


/* delete */
void xjs_delete(void *ptr);

#endif

