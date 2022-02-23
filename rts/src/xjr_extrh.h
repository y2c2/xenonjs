/* XenonJS : Runtime Time System : External Resource
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_EXTRH_H
#define XJR_EXTRH_H

#include "xjr_dt.h"

/* External Resource Holder */

typedef struct
{
    xjr_bool marked;
} extrh_t;

typedef struct
{
    extrh_t *body;
    xjr_size_t size;
    void *heap_data;
    xjr_heap_malloc_callback cb_malloc;
    xjr_heap_free_callback cb_free;
} extrh_vec_t;

void extrh_vec_init(extrh_vec_t *vec, \
        void *heap_data, \
        xjr_heap_malloc_callback cb_malloc, \
        xjr_heap_free_callback cb_free);
void extrh_vec_uninit(extrh_vec_t *vec);


#endif

