/* Enhanced C : Allocator
 * Copyright(c) 2017-2020 y2c2 */

#ifdef __clang__
#pragma clang diagnostic ignored "-Wcast-align"
#endif

#include "ec_alloc.h"
#include "ec_dt.h"

/* Global stuff */
static ec_malloc_cb_t g_ec_malloc = NULL;
static ec_calloc_cb_t g_ec_calloc = NULL;
static ec_free_cb_t g_ec_free = NULL;
static ec_memcpy_cb_t g_ec_memcpy = NULL;
static ec_memset_cb_t g_ec_memset = NULL;

void ec_allocator_set_malloc(ec_malloc_cb_t cb);
void ec_allocator_set_calloc(ec_calloc_cb_t cb);
void ec_allocator_set_free(ec_free_cb_t cb);
void ec_allocator_set_memcpy(ec_memcpy_cb_t cb);
void ec_allocator_set_memset(ec_memset_cb_t cb);

void ec_allocator_set_malloc(ec_malloc_cb_t cb) { g_ec_malloc = cb; }
void ec_allocator_set_calloc(ec_calloc_cb_t cb) { g_ec_calloc = cb; }
void ec_allocator_set_free(ec_free_cb_t cb) { g_ec_free = cb; }
void ec_allocator_set_memcpy(ec_memcpy_cb_t cb) { g_ec_memcpy = cb; }
void ec_allocator_set_memset(ec_memset_cb_t cb) { g_ec_memset = cb; }

void* ec_malloc(ec_size_t size) { return g_ec_malloc(size); }
void* ec_calloc(ec_size_t nmemb, ec_size_t size)
{
    return g_ec_calloc(nmemb, size);
}
void ec_free(void* ptr) { g_ec_free(ptr); }
void* ec_memcpy(void* dest, const void* src, ec_size_t n)
{
    return g_ec_memcpy(dest, src, n);
}
void* ec_memset(void* s, int c, ec_size_t n) { return g_ec_memset(s, c, n); }

/* <dtor> <item_size> <item_count> <body> */

#define PTR_SIZE (sizeof(void*))
#define ITEM_SIZE_SIZE (sizeof(ec_size_t))
#define COUNT_SIZE (sizeof(ec_size_t))
typedef void (*__ec_dtor)(void* ptr);

void* __ec_new_raw(ec_size_t size, ec_size_t count, void ctor(void* ptr),
                   void dtor(void* ptr))
{
    __ec_dtor* _dtor;
    ec_size_t* _item_size;
    ec_size_t* _item_count;
    ec_size_t idx;
    char* ptr_with_dtor = NULL;
    char *ptr, *ptr2;
    ptr_with_dtor =
        ec_malloc((PTR_SIZE + ITEM_SIZE_SIZE + COUNT_SIZE) + size * count);
    if (ptr_with_dtor == NULL)
        return NULL;
    _dtor = (__ec_dtor*)ptr_with_dtor;
    _item_size = (ec_size_t*)(ptr_with_dtor + PTR_SIZE);
    _item_count = (ec_size_t*)(ptr_with_dtor + PTR_SIZE + ITEM_SIZE_SIZE);
    ptr = (char*)(ptr_with_dtor + PTR_SIZE + ITEM_SIZE_SIZE + COUNT_SIZE);
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

void ec_delete(void* ptr)
{
    __ec_dtor _dtor;
    ec_size_t* _item_size;
    ec_size_t* _item_count;
    ec_size_t idx;
    char* ptr_with_dtor;
    ec_size_t size;
    ec_size_t count;
    char* ptrb;
    if (ptr == NULL)
        return;
    ptr_with_dtor = ((char*)ptr) - (PTR_SIZE + ITEM_SIZE_SIZE + COUNT_SIZE);
    ptrb = ptr;
    _dtor = *((__ec_dtor*)ptr_with_dtor);
    if (_dtor != NULL)
    {
        _item_size = (ec_size_t*)(ptr_with_dtor + PTR_SIZE);
        _item_count = (ec_size_t*)(ptr_with_dtor + PTR_SIZE + ITEM_SIZE_SIZE);
        size = *_item_size;
        count = *_item_count;
        for (idx = 0; idx != count; idx++)
        {
            (*_dtor)(ptrb);
            ptrb += size;
        }
    }
    ec_free(ptr_with_dtor);
}
