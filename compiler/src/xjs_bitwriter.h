/* XenonJS : Bit Writer
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_BITWRITER_H
#define XJS_BITWRITER_H

#include "xjs_types.h"

typedef void *(*xjs_bitwriter_malloc_cb_t)(xjs_size_t size);
typedef void (*xjs_bitwriter_free_cb_t)(void *ptr);
typedef void *(*xjs_bitwriter_memcpy_cb_t)(void *dest, const void *src, xjs_size_t n);

struct xjs_bitwriter
{
    char *body;
    xjs_size_t size;
    xjs_size_t capacity;

    xjs_size_t init_size;
    xjs_size_t inc_size;
    xjs_bitwriter_malloc_cb_t malloc_cb;
    xjs_bitwriter_free_cb_t free_cb;
    xjs_bitwriter_memcpy_cb_t memcpy_cb;
};

typedef struct xjs_bitwriter xjs_bitwriter;

int xjs_bitwriter_init_conf( \
        xjs_bitwriter *xjs_bitwriter, \
        xjs_size_t init_size, xjs_size_t inc_size, \
        xjs_bitwriter_malloc_cb_t malloc_cb, \
        xjs_bitwriter_free_cb_t free_cb, \
        xjs_bitwriter_memcpy_cb_t memcpy_cb);
int xjs_bitwriter_init( \
        xjs_bitwriter *xjs_bitwriter);
void xjs_bitwriter_uninit(xjs_bitwriter *xjs_bitwriter);

int xjs_bitwriter_append(xjs_bitwriter *xjs_bitwriter, const char *s, const xjs_size_t len);
int xjs_bitwriter_append_c_str(xjs_bitwriter *xjs_bitwriter, const char *s);
int xjs_bitwriter_write_u32(xjs_bitwriter *xjs_bitwriter, const xjs_u32 v);
int xjs_bitwriter_write_u16(xjs_bitwriter *xjs_bitwriter, const xjs_u16 v);
int xjs_bitwriter_write_u8(xjs_bitwriter *xjs_bitwriter, const xjs_u8 v);
int xjs_bitwriter_write_s32(xjs_bitwriter *xjs_bitwriter, const xjs_s32 v);
int xjs_bitwriter_write_s16(xjs_bitwriter *xjs_bitwriter, const xjs_s16 v);
int xjs_bitwriter_write_s8(xjs_bitwriter *xjs_bitwriter, const xjs_s8 v);
int xjs_bitwriter_write_f64(xjs_bitwriter *xjs_bitwriter, const xjs_f64 v);

int xjs_bitwriter_shift(xjs_bitwriter *xjs_bitwriter, const xjs_size_t len);
int xjs_bitwriter_clear(xjs_bitwriter *xjs_bitwriter);

char *xjs_bitwriter_body(xjs_bitwriter *xjs_bitwriter);
xjs_size_t xjs_bitwriter_size(xjs_bitwriter *xjs_bitwriter);

#endif

