/* XenonJS : Runtime Time System : Mutable Buffer
 * Copyright(c) 2018 y2c2 */

#ifndef XJS_MBUF_H
#define XJS_MBUF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xjr_dt.h"

typedef xjr_size_t xjr_mbuf_size_t;
typedef void *(*xjr_mbuf_malloc_cb_t)(void *data, xjr_mbuf_size_t size);
typedef void (*xjr_mbuf_free_cb_t)(void *data, void *ptr);
typedef void *(*xjr_mbuf_memcpy_cb_t)(void *dest, const void *src, xjr_mbuf_size_t n);

struct xjr_mbuf
{
    char *body;
    xjr_mbuf_size_t size;
    xjr_mbuf_size_t capacity;

    xjr_mbuf_size_t init_size;
    xjr_mbuf_size_t inc_size;

    void *data;
    xjr_mbuf_malloc_cb_t malloc_cb;
    xjr_mbuf_free_cb_t free_cb;

    xjr_mbuf_memcpy_cb_t memcpy_cb;
};

typedef struct xjr_mbuf xjr_mbuf_t;

int xjr_mbuf_init_conf( \
        xjr_mbuf_t *mbuf, \
        xjr_mbuf_size_t init_size, xjr_mbuf_size_t inc_size, \
        void *data, \
        xjr_mbuf_malloc_cb_t malloc_cb, \
        xjr_mbuf_free_cb_t free_cb, \
        xjr_mbuf_memcpy_cb_t memcpy_cb);
int xjr_mbuf_init( \
        xjr_mbuf_t *mbuf, \
        void *data, \
        xjr_mbuf_malloc_cb_t malloc_cb, \
        xjr_mbuf_free_cb_t free_cb, \
        xjr_mbuf_memcpy_cb_t memcpy_cb);
void xjr_mbuf_uninit(xjr_mbuf_t *mbuf);

int xjr_mbuf_append(xjr_mbuf_t *mbuf, const char *s, const xjr_mbuf_size_t len);
int xjr_mbuf_append_c_str(xjr_mbuf_t *mbuf, const char *s);
int xjr_mbuf_shift(xjr_mbuf_t *mbuf, const xjr_mbuf_size_t len);
int xjr_mbuf_clear(xjr_mbuf_t *mbuf);

char *xjr_mbuf_body(xjr_mbuf_t *mbuf);
xjr_mbuf_size_t xjr_mbuf_size(xjr_mbuf_t *mbuf);

#ifdef __cplusplus
}
#endif

#endif

