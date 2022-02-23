/* XenonJS : Allocator
 * Copyright(c) 2017 y2c2 */

#ifdef __clang__
#pragma clang diagnostic ignored "-Wcast-align"
#endif

#include "xjs_dt.h"
#include "xjs_alloc.h"

/* Global stuff */
static xjs_malloc_cb_t g_xjs_malloc = NULL;
static xjs_calloc_cb_t g_xjs_calloc = NULL;
static xjs_free_cb_t g_xjs_free = NULL;
static xjs_memcpy_cb_t g_xjs_memcpy = NULL;
static xjs_memset_cb_t g_xjs_memset = NULL;

void xjs_allocator_set_malloc(xjs_malloc_cb_t cb);
void xjs_allocator_set_calloc(xjs_calloc_cb_t cb);
void xjs_allocator_set_free(xjs_free_cb_t cb);
void xjs_allocator_set_memcpy(xjs_memcpy_cb_t cb);
void xjs_allocator_set_memset(xjs_memset_cb_t cb);

void xjs_allocator_set_malloc(xjs_malloc_cb_t cb) { g_xjs_malloc = cb; }
void xjs_allocator_set_calloc(xjs_calloc_cb_t cb) { g_xjs_calloc = cb; }
void xjs_allocator_set_free(xjs_free_cb_t cb) { g_xjs_free = cb; }
void xjs_allocator_set_memcpy(xjs_memcpy_cb_t cb) { g_xjs_memcpy = cb; }
void xjs_allocator_set_memset(xjs_memset_cb_t cb) { g_xjs_memset = cb; }

void *xjs_malloc(xjs_size_t size) { return g_xjs_malloc(size); }
void *xjs_calloc(xjs_size_t nmemb, xjs_size_t size) { return g_xjs_calloc(nmemb, size); }
void xjs_free(void *ptr) { g_xjs_free(ptr); }
void *xjs_memcpy(void *dest, const void *src, xjs_size_t n) { return g_xjs_memcpy(dest, src, n); }
void *xjs_memset(void *s, int c, xjs_size_t n) { return g_xjs_memset(s, c, n); }

/* <dtor> <item_size> <item_count> <body> */

#define PTR_SIZE (sizeof(void *))
#define ITEM_SIZE_SIZE (sizeof(xjs_size_t))
#define COUNT_SIZE (sizeof(xjs_size_t))
typedef void (*__xjs_dtor)(void *ptr);

void *__xjs_new_raw( \
        xjs_size_t size, \
        xjs_size_t count, \
        void ctor(void *ptr), \
        void dtor(void *ptr))
{
    __xjs_dtor *_dtor;
    xjs_size_t *_item_size;
    xjs_size_t *_item_count;
    xjs_size_t idx;
    char *ptr_with_dtor = NULL;
    char *ptr, *ptr2;

    ptr_with_dtor = xjs_malloc( \
            (PTR_SIZE + ITEM_SIZE_SIZE + COUNT_SIZE) + size * count);
    if (ptr_with_dtor == NULL) return NULL;

    _dtor = (__xjs_dtor *)ptr_with_dtor;
    _item_size = (xjs_size_t *)(ptr_with_dtor + PTR_SIZE);
    _item_count = (xjs_size_t *)(ptr_with_dtor + PTR_SIZE + ITEM_SIZE_SIZE);
    ptr = (char *)(ptr_with_dtor + PTR_SIZE + ITEM_SIZE_SIZE + COUNT_SIZE);

    /* Set block information */
    *_item_size = size;
    *_item_count = count;
    *_dtor = dtor;

    /* Initialize */
    if (ctor != NULL)
    {
        ptr2 = ptr;
        for (idx = 0; idx != count; idx++)
        {
            ctor(ptr2);
            ptr2 += size;
        }
    }

    return ptr;
}

void xjs_delete(void *ptr)
{
    __xjs_dtor _dtor;
    xjs_size_t *_item_size;
    xjs_size_t *_item_count;
    xjs_size_t idx;
    char *ptr_with_dtor;
    xjs_size_t size;
    xjs_size_t count;
    char *ptrb;

    if (ptr == NULL) return;

    ptr_with_dtor = ((char *)ptr) - (PTR_SIZE + ITEM_SIZE_SIZE + COUNT_SIZE);
    ptrb = ptr;
    _dtor = *((__xjs_dtor *)ptr_with_dtor);
    if (_dtor != NULL)
    {
        _item_size = (xjs_size_t *)(ptr_with_dtor + PTR_SIZE);
        _item_count = (xjs_size_t *)(ptr_with_dtor + PTR_SIZE + ITEM_SIZE_SIZE);
        size = *_item_size;
        count = *_item_count;

        for (idx = 0; idx != count; idx++)
        {
            (*_dtor)(ptrb);
            ptrb += size;
        }
    }
    xjs_free(ptr_with_dtor);
}

