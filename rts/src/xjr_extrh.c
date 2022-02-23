/* XenonJS : Runtime Time System : External Resource
 * Copyright(c) 2017 y2c2 */

#include "xjr_libc.h"
#include "xjr_extrh.h"

void extrh_vec_init(extrh_vec_t *vec, \
        void *heap_data, \
        xjr_heap_malloc_callback cb_malloc, \
        xjr_heap_free_callback cb_free)
{
    vec->body = xjr_nullptr;
    vec->size = 0;
    vec->heap_data = heap_data;
    vec->cb_malloc = cb_malloc;
    vec->cb_free = cb_free;
}

void extrh_vec_uninit(extrh_vec_t *vec)
{
    if (vec->body != xjr_nullptr) vec->cb_free(vec->heap_data, vec->body);
}

