#ifndef XJS_PRIVATE_H
#define XJS_PRIVATE_H

#include <ec_string.h>
#include "xjs_dt.h"
#include "xjs_types.h"

typedef void *(*xjs_malloc_cb_t)(xjs_size_t size);
typedef void *(*xjs_calloc_cb_t)(xjs_size_t nmemb, xjs_size_t size);
typedef void (*xjs_free_cb_t)(void *ptr);
typedef void *(*xjs_memcpy_cb_t)(void *dest, const void *src, xjs_size_t n);
typedef void *(*xjs_memset_cb_t)(void *s, int c, xjs_size_t n);

#endif

