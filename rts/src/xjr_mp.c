/* XenonJS : Runtime Time System : Memory Pool
 * Copyright(c) 2017 y2c2 */

#include "xjr_dt.h"
#include "xjr_libc.h"
#include "xjr_mp.h"

/* Layer
 * --------------
 *
 * Pool -> Sub Pool -> Block 
 *
 */


/* Structure of each item (Virtually)
 * ---------------------------
 *
 * struct xjr_pool_item
 * {
 *    xjr_bit_t marked;            -- (1 bit)
 *    xjr_urid_t urid;             -- (15 bits)
 *
 *    xjr_pool_size_t size;        -- (2 Bytes, only contains the content, no header)
 *    xjr_pool_size_t offset_next; -- (2 Bytes)
 *    xjr_u8 [size];               -- (size Bytes)
 * };
 *
 * There is a solid block exists in every pool at the tail
 * position to indicate the final block
 *
 */

#define XJR_MP_BLOCK_HEADER_SIZE (6)
#define XJR_MP_BLOCK_HEADER_URID_OFFSET (0)
#define XJR_MP_BLOCK_HEADER_SIZE_OFFSET (XJR_MP_WIDTH)
#define XJR_MP_BLOCK_HEADER_OFFSET_NEXT_OFFSET ((XJR_MP_WIDTH)+(XJR_MP_WIDTH))
#define XJR_MP_BLOCK_HEADER_DATA_OFFSET ((XJR_MP_WIDTH)+(XJR_MP_WIDTH)+(XJR_MP_WIDTH))


/* Refined part */

#define XJR_MP_URID_REFINE(ruid) \
    ((ruid) & XJR_URID_REFINE_MASK)


/* Marked */

#define XJR_MP_BLOCK_IS_MARKED(ruid) \
    (((ruid) & XJR_URID_MARKED_MASK) != 0)
#define XJR_MP_URID_SET_MARKED(ruid) \
    ((ruid) | XJR_URID_MARKED_MASK)
#define XJR_MP_URID_CLR_MARKED(ruid) \
    ((ruid) & XJR_URID_REFINE_MASK)


/* Managed */
#define XJR_MP_BLOCK_IS_MANAGED(ruid) \
    (((ruid) & XJR_URID_MARKED_MASK) != 0)
#define XJR_MP_URID_SET_MANAGED(ruid) \
    ((ruid) | XJR_URID_MARKED_MASK)
#define XJR_MP_URID_CLR_MANAGED(ruid) \
    ((ruid) & XJR_URID_REFINE_MASK)


/* Sub pools */

struct xjr_mp_sub
{
    /* Base pointer to the allocated block memory from heap */
    void *base;

    /* Status of the current sub pool */
    xjr_u32 size_free;
    xjr_u32 size_capacity;
    
    /* Offset to the first block */
    xjr_mp_size_t offset_begin;

    xjr_mp_t *mp;
};
typedef struct xjr_mp_sub xjr_mp_sub_t;

/* Pool */
struct xjr_mp
{
    /* Sub pools */
    xjr_mp_sub_t **pools;

    xjr_size_t pools_num_allocated;
    xjr_size_t pools_num_capacity;

    /* Current */
    xjr_size_t size_usage;
    xjr_size_t size_capacity;
    xjr_size_t num_allocated_blocks;
    /* Peek */
    xjr_size_t size_usage_peak;
    xjr_size_t size_capacity_peak;
    xjr_size_t num_allocated_blocks_peak;

    /* Cache */
#ifdef XJR_URID_CACHE
    xjr_urid_t *urid_cache;
    xjr_size_t urid_cache_p;
    xjr_size_t urid_cache_size;
#endif

    /* Heap */
    xjr_heap_malloc_callback cb_malloc;
    xjr_heap_free_callback cb_free;
    void *heap_data;

    xjr_heap_malloc_record_callback cb_malloc_record;
    xjr_heap_free_record_callback cb_free_record;

    xjr_u32 opts;
};


/* Declarations */
void xjr_mp_destroy(xjr_mp_t *pool);
static xjr_bool xjr_sub_pool_free(xjr_mp_sub_t *sub_pool, xjr_urid_t urid);

static void xjr_sub_pool_write_pool_size(xjr_mp_sub_t *sub_pool, \
        xjr_mp_size_t offset, \
        xjr_mp_size_t value)
{
    xjr_mp_size_t *offset_dst = (xjr_mp_size_t *)(((xjr_u8 *)sub_pool->base) + offset);
    *offset_dst = value;
}

static void xjr_sub_pool_write_block(xjr_mp_sub_t *sub_pool, \
        xjr_mp_size_t offset, \
        xjr_bool managed, \
        xjr_urid_t urid, \
        xjr_mp_size_t size, \
        xjr_mp_size_t offset_next)
{
    if (managed != xjr_false)
    {
        xjr_sub_pool_write_pool_size(sub_pool, offset, XJR_MP_URID_SET_MANAGED(urid));
    }
    else
    {
        xjr_sub_pool_write_pool_size(sub_pool, offset, urid);
    }

    offset += XJR_MP_WIDTH;
    xjr_sub_pool_write_pool_size(sub_pool, offset, size);
    offset += XJR_MP_WIDTH;
    xjr_sub_pool_write_pool_size(sub_pool, offset, offset_next);
}

static void xjr_sub_pool_write_block_final(xjr_mp_sub_t *sub_pool)
{
    xjr_sub_pool_write_block(sub_pool, \
            (xjr_mp_size_t)(sub_pool->size_capacity - XJR_MP_BLOCK_HEADER_SIZE), \
            xjr_false, \
            XJR_URID_FINAL, \
            0, \
            XJR_URID_INVALID);
}

static void xjr_sub_pool_update_block_urid(xjr_mp_sub_t *sub_pool, \
        xjr_mp_size_t offset, \
        xjr_urid_t urid)
{
    xjr_sub_pool_write_pool_size(sub_pool, offset, urid);
}

static void xjr_sub_pool_update_block_offset_next(xjr_mp_sub_t *sub_pool, \
        xjr_mp_size_t offset, \
        xjr_mp_size_t offset_next)
{
    offset += XJR_MP_BLOCK_HEADER_OFFSET_NEXT_OFFSET;
    xjr_sub_pool_write_pool_size(sub_pool, offset, offset_next);
}

static xjr_bool xjr_sub_pool_read_block_managed(xjr_mp_sub_t *sub_pool, \
        xjr_mp_size_t offset)
{
    xjr_mp_size_t *offset_src = (xjr_mp_size_t *)(((xjr_u8 *)sub_pool->base) + offset);
    return XJR_MP_BLOCK_IS_MANAGED(*((xjr_urid_t *)offset_src));
}

static xjr_bool xjr_sub_pool_read_block_marked(xjr_mp_sub_t *sub_pool, \
        xjr_mp_size_t offset)
{
    xjr_mp_size_t *offset_src = (xjr_mp_size_t *)(((xjr_u8 *)sub_pool->base) + offset);
    return XJR_MP_BLOCK_IS_MARKED(*((xjr_urid_t *)offset_src));
}

static xjr_urid_t xjr_sub_pool_read_block_urid(xjr_mp_sub_t *sub_pool, \
        xjr_mp_size_t offset)
{
    xjr_mp_size_t *offset_src = (xjr_mp_size_t *)(((xjr_u8 *)sub_pool->base) + offset);
    return (xjr_urid_t)(XJR_MP_URID_REFINE(*((xjr_urid_t *)offset_src)));
}

static void xjr_sub_pool_clear_block_marked(xjr_mp_sub_t *sub_pool, \
        xjr_mp_size_t offset)
{
    xjr_urid_t raw_urid;
    raw_urid = xjr_sub_pool_read_block_urid(sub_pool, offset);
    xjr_sub_pool_update_block_urid(sub_pool, offset, XJR_MP_URID_CLR_MARKED(raw_urid));
}

static void xjr_sub_pool_set_block_marked(xjr_mp_sub_t *sub_pool, \
        xjr_mp_size_t offset)
{
    xjr_urid_t raw_urid;
    raw_urid = xjr_sub_pool_read_block_urid(sub_pool, offset);
    xjr_sub_pool_update_block_urid(sub_pool, offset, XJR_MP_URID_SET_MARKED(raw_urid));
}

static xjr_mp_size_t xjr_sub_pool_read_block_size(xjr_mp_sub_t *sub_pool, \
        xjr_mp_size_t offset)
{
    xjr_mp_size_t *offset_src = (xjr_mp_size_t *)(((xjr_u8 *)sub_pool->base) + offset + XJR_MP_BLOCK_HEADER_SIZE_OFFSET);
    return (xjr_mp_size_t)(*((xjr_mp_size_t*)offset_src));
}

static void *xjr_sub_pool_read_ptr(xjr_mp_sub_t *sub_pool, \
        xjr_mp_size_t offset)
{
    return ((xjr_u8 *)sub_pool->base) + offset + XJR_MP_BLOCK_HEADER_DATA_OFFSET;
}

static xjr_mp_size_t xjr_sub_pool_read_block_offset_next(xjr_mp_sub_t *sub_pool, \
        xjr_mp_size_t offset)
{
    xjr_mp_size_t *offset_src = (xjr_mp_size_t *)(((xjr_u8 *)sub_pool->base) + offset + XJR_MP_BLOCK_HEADER_OFFSET_NEXT_OFFSET);
    return (xjr_mp_size_t)(*((xjr_mp_size_t*)offset_src));
}

static xjr_bool xjr_sub_pool_is_final_block(xjr_mp_sub_t *sub_pool, \
        xjr_mp_size_t offset)
{
    if (xjr_sub_pool_read_block_urid(sub_pool, offset) != XJR_URID_FINAL)
    { return xjr_false; }
    if (xjr_sub_pool_read_block_size(sub_pool, offset) != 0)
    { return xjr_false; }
    if (xjr_sub_pool_read_block_offset_next(sub_pool, offset) != XJR_URID_INVALID)
    { return xjr_false; }

    return xjr_true;
}

static xjr_bool xjr_sub_pool_is_empty(xjr_mp_sub_t *sub_pool)
{
    return xjr_sub_pool_is_final_block(sub_pool, sub_pool->offset_begin);
}


/* Create/Destroy pool */

static xjr_mp_sub_t *xjr_mp_sub_new(xjr_mp_t *mp)
{
    xjr_mp_sub_t *new_sub_pool = xjr_nullptr;

    if ((new_sub_pool = (xjr_mp_sub_t *)mp->cb_malloc( \
                    mp->heap_data, \
                    sizeof(xjr_mp_sub_t))) == xjr_nullptr)
    { return xjr_nullptr; }
    new_sub_pool->mp = mp;
    /* Allocate the base */
    if ((new_sub_pool->base = mp->cb_malloc( \
                    mp->heap_data, \
                    XJR_SUB_MP_SIZE)) == xjr_nullptr)
    { goto fail; }
    /* Clean the pool */
    xjr_memset(new_sub_pool->base, 0, XJR_SUB_MP_SIZE);
    /* Free size and capacity size */
    new_sub_pool->size_free = XJR_SUB_MP_SIZE - XJR_MP_BLOCK_HEADER_SIZE;
    new_sub_pool->size_capacity = XJR_SUB_MP_SIZE;
    /* The beginning block is the final block */
    new_sub_pool->offset_begin = XJR_SUB_MP_SIZE - XJR_MP_BLOCK_HEADER_SIZE;
    /* Write the final block */
    xjr_sub_pool_write_block_final(new_sub_pool);

    goto done;
fail:
    if (new_sub_pool != xjr_nullptr)
    {
        if (new_sub_pool->base != xjr_nullptr) { mp->cb_free(mp->heap_data, new_sub_pool->base); }
        mp->cb_free(mp->heap_data, new_sub_pool);
        new_sub_pool = xjr_nullptr;
    }
done:
    return new_sub_pool;
}

static void xjr_sub_pool_destroy(xjr_mp_sub_t *smp)
{
    if (smp->base != xjr_nullptr) smp->mp->cb_free(smp->mp->heap_data, smp->base);
    smp->mp->cb_free(smp->mp->heap_data, smp);
}

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifdef XJR_URID_CACHE
static int xjr_mp_urid_cache_init(xjr_mp_t *mp, xjr_size_t urid_cache_size)
{
    xjr_size_t idx;

    urid_cache_size = MIN(urid_cache_size, XJR_URID_UPBOUND_LIMIT);

    if ((mp->urid_cache = (xjr_urid_t *)xjr_malloc( \
                    sizeof(xjr_urid_t) * urid_cache_size)) == xjr_nullptr)
    { return -1; }

    for (idx = 0; idx != urid_cache_size; idx++)
    { mp->urid_cache[idx] = (xjr_urid_t)idx; }

    mp->urid_cache_p = urid_cache_size;
    mp->urid_cache_size = urid_cache_size;

    return 0;
}

static xjr_bool xjr_mp_urid_cache_empty(xjr_mp_t *pool)
{
    return pool->urid_cache_p == 0 ? xjr_true : xjr_false;
}

static xjr_bool xjr_mp_urid_cache_full(xjr_mp_t *pool)
{
    return pool->urid_cache_p == pool->urid_cache_size ? xjr_true : xjr_false;
}

static int xjr_mp_urid_cache_allocate(xjr_mp_t *pool, xjr_urid_t *urid_out)
{
    if (xjr_mp_urid_cache_empty(pool) == xjr_true) return -1;
    
    pool->urid_cache_p--;
    *urid_out = pool->urid_cache[pool->urid_cache_p];

    return 0;
}

static void xjr_mp_urid_cache_release(xjr_mp_t *pool, xjr_urid_t urid)
{
    if (xjr_mp_urid_cache_full(pool) == xjr_true) return;
    pool->urid_cache[pool->urid_cache_p] = urid;
    pool->urid_cache_p++;
}

#endif

xjr_mp_t *xjr_mp_new_opt( \
        void *heap_data, \
        xjr_heap_malloc_callback cb_malloc, \
        xjr_heap_free_callback cb_free, \
        xjr_size_t urid_cache_size, \
        xjr_u32 opts)
{
    xjr_mp_t *new_pool = xjr_nullptr;
    xjr_size_t idx;

    /* Allocate the pool */
    if ((new_pool = (xjr_mp_t *)cb_malloc( \
                    heap_data, \
                    sizeof(xjr_mp_t))) == xjr_nullptr)
    { return xjr_nullptr; }

    new_pool->opts = opts;

    new_pool->heap_data = heap_data;
    new_pool->cb_malloc = cb_malloc;
    new_pool->cb_free = cb_free;
    new_pool->cb_malloc_record = xjr_nullptr;
    new_pool->cb_free_record = xjr_nullptr;

    new_pool->pools = xjr_nullptr;
    new_pool->pools_num_allocated = 0;
    new_pool->pools_num_capacity = 0;

    /* Allocate slots of sub pools */
    if ((new_pool->pools = (xjr_mp_sub_t **)cb_malloc( \
                    heap_data, \
                    sizeof(xjr_mp_sub_t *) * XJR_SUB_MP_MAX_NUM)) == xjr_nullptr)
    { goto fail; }
    for (idx = 0; idx != XJR_SUB_MP_MAX_NUM; idx++)
    { new_pool->pools[idx] = xjr_nullptr; }
    new_pool->pools_num_capacity = XJR_SUB_MP_MAX_NUM;

    /* Allocate the first sub pool */
    if ((new_pool->pools[0] = xjr_mp_sub_new(new_pool)) == xjr_nullptr)
    { goto fail; }
    new_pool->pools_num_allocated = 1;

    new_pool->size_capacity = xjr_mp_size_capacity(new_pool);
    new_pool->size_usage = xjr_mp_size_capacity(new_pool) - xjr_mp_size_free(new_pool);
    new_pool->size_capacity_peak = new_pool->size_capacity;
    new_pool->size_usage_peak = new_pool->size_usage;
    new_pool->num_allocated_blocks = 0;
    new_pool->num_allocated_blocks_peak = 0;
#ifdef XJR_URID_CACHE
    new_pool->urid_cache = xjr_nullptr;
#endif

#ifdef XJR_URID_CACHE
    if (xjr_mp_urid_cache_init(new_pool, urid_cache_size) != 0)
    { goto fail; }
#else
    (void)urid_cache_size;
#endif

    goto done;
fail:
    if (new_pool != xjr_nullptr)
    {
        xjr_mp_destroy(new_pool);
        new_pool = xjr_nullptr;
    }
done:
    return new_pool;
}

xjr_mp_t *xjr_mp_new( \
        void *heap_data, \
        xjr_heap_malloc_callback cb_malloc, \
        xjr_heap_free_callback cb_free)
{
#ifdef XJR_URID_CACHE_SIZE_DEFAULT
    return xjr_mp_new_opt(heap_data, cb_malloc, cb_free, XJR_URID_CACHE_SIZE_DEFAULT, 0);
#else
    return xjr_mp_new_opt(heap_data, cb_malloc, cb_free, 0, 0);
#endif
}

void xjr_mp_destroy(xjr_mp_t *pool)
{
    xjr_size_t idx;

    if (pool->pools != xjr_nullptr)
    {
        for (idx = 0; idx != pool->pools_num_capacity; idx++)
        {
            if (pool->pools[idx] != xjr_nullptr)
            {
                xjr_sub_pool_destroy(pool->pools[idx]);
            }
        }
        pool->cb_free(pool->heap_data, pool->pools);
    }
#ifdef XJR_URID_CACHE
    if (pool->urid_cache != xjr_nullptr) { xjr_free(pool->urid_cache); }
#endif
    pool->cb_free(pool->heap_data, pool);
}

void xjr_mp_set_malloc_callback(xjr_mp_t *pool, xjr_heap_malloc_record_callback cb)
{
    pool->cb_malloc_record = cb;
}

void xjr_mp_set_free_callback(xjr_mp_t *pool, xjr_heap_free_record_callback cb)
{
    pool->cb_free_record = cb;
}


/* Status */

xjr_size_t xjr_mp_size_free(xjr_mp_t *pool)
{
    xjr_size_t idx;
    xjr_size_t size_free = 0;

    for (idx = 0; idx != pool->pools_num_allocated; idx++)
    {
        if (pool->pools[idx] == xjr_nullptr) continue;
        size_free += pool->pools[idx]->size_free;
    }

    return size_free;
}

xjr_size_t xjr_mp_size_capacity(xjr_mp_t *pool)
{
    xjr_size_t idx;
    xjr_size_t size_capacity = 0;

    for (idx = 0; idx != pool->pools_num_allocated; idx++)
    {
        if (pool->pools[idx] == xjr_nullptr) continue;
        size_capacity += pool->pools[idx]->size_capacity;
    }

    return size_capacity;
}

xjr_size_t xjr_mp_size_usage_peak(xjr_mp_t *pool)
{
    return pool->size_usage_peak;
}

xjr_size_t xjr_mp_size_capacity_peak(xjr_mp_t *pool)
{
    return pool->size_capacity_peak;
}


/* Debug */

xjr_size_t xjr_mp_number_sub_pools(xjr_mp_t *pool)
{
    return pool->pools_num_allocated;
}

static xjr_size_t xjr_sub_pool_number_block(xjr_mp_sub_t *sub_pool)
{
    xjr_size_t count = 0;
    xjr_mp_size_t offset = sub_pool->offset_begin;

    for (;;)
    {
        if (xjr_sub_pool_is_final_block(sub_pool, offset) == xjr_true) break;

        count++;
        offset = xjr_sub_pool_read_block_offset_next(sub_pool, offset);
    }

    return count + 1;
}

xjr_size_t xjr_mp_number_block(xjr_mp_t *pool)
{
    xjr_size_t idx;
    xjr_size_t count = 0;

    for (idx = 0; idx != pool->pools_num_allocated; idx++)
    {
        count += xjr_sub_pool_number_block(pool->pools[idx]);
    }

    return count;
}

xjr_size_t xjr_mp_number_allocated_block_peak(xjr_mp_t *pool)
{
    return pool->num_allocated_blocks_peak;
}

static xjr_bool xjr_sub_pool_block_get_by_urid( \
        xjr_mp_size_t *offset_out, \
        xjr_mp_sub_t *sub_pool, \
        xjr_urid_t urid)
{
    xjr_mp_size_t offset;

    offset = sub_pool->offset_begin;

    while (xjr_sub_pool_is_final_block(sub_pool, offset) == xjr_false)
    {
        if (xjr_sub_pool_read_block_urid(sub_pool, offset) == urid)
        {
            /* Matched */
            *offset_out = offset;
            return xjr_true;
        }

        /* Next */
        offset = xjr_sub_pool_read_block_offset_next(sub_pool, offset);
    }

    return xjr_false;
}

typedef enum
{
    XJR_MP_BLOCK_GET_BY_IDX_URID = 0,
    XJR_MP_BLOCK_GET_BY_IDX_SIZE = 1,
    XJR_MP_BLOCK_GET_BY_IDX_OFFSET_NEXT = 2,
} xjr_mp_block_get_by_idx_type_t;

static xjr_size_t xjr_mp_block_get_by_idx(xjr_mp_t *pool, xjr_size_t n, \
        xjr_mp_block_get_by_idx_type_t type)
{
    xjr_size_t idx;
    xjr_mp_sub_t *sub_pool;
    xjr_mp_size_t offset;

    for (idx = 0; idx != pool->pools_num_allocated; idx++)
    {
        sub_pool = pool->pools[idx];
        offset = sub_pool->offset_begin;

        for (;;)
        {
            if (xjr_sub_pool_is_final_block(sub_pool, offset))
            { break; }
            if (n == 0) 
            {
                switch (type)
                {
                    case XJR_MP_BLOCK_GET_BY_IDX_URID:
                        return (xjr_size_t)xjr_sub_pool_read_block_urid(sub_pool, offset); 
                    case XJR_MP_BLOCK_GET_BY_IDX_SIZE:
                        return (xjr_size_t)xjr_sub_pool_read_block_size(sub_pool, offset); 
                    case XJR_MP_BLOCK_GET_BY_IDX_OFFSET_NEXT:
                        return (xjr_size_t)xjr_sub_pool_read_block_offset_next(sub_pool, offset); 
                }
            }

            offset = xjr_sub_pool_read_block_offset_next(sub_pool, offset); 
            n--;
        }
    }

    return (xjr_size_t)XJR_URID_INVALID;
}

xjr_urid_t xjr_mp_block_urid(xjr_mp_t *pool, xjr_size_t n)
{
    return (xjr_urid_t)xjr_mp_block_get_by_idx(pool, n, \
            XJR_MP_BLOCK_GET_BY_IDX_URID);
}

xjr_mp_size_t xjr_mp_block_size(xjr_mp_t *pool, xjr_size_t n)
{
    return (xjr_mp_size_t)xjr_mp_block_get_by_idx(pool, n, \
            XJR_MP_BLOCK_GET_BY_IDX_SIZE);
}

xjr_mp_size_t xjr_mp_block_offset_next(xjr_mp_t *pool, xjr_size_t n)
{
    return (xjr_mp_size_t)xjr_mp_block_get_by_idx(pool, n, \
            XJR_MP_BLOCK_GET_BY_IDX_OFFSET_NEXT);
}


/* Allocate/Release */

static xjr_bool xjr_sub_pool_allocate_urid_try(xjr_mp_sub_t *sub_pool, xjr_urid_t urid)
{
    xjr_mp_size_t offset = sub_pool->offset_begin;

    while (xjr_sub_pool_is_final_block(sub_pool, offset) == xjr_false)
    {
        if (xjr_sub_pool_read_block_urid(sub_pool, offset) == urid)
        { return xjr_false; }
        offset = xjr_sub_pool_read_block_offset_next(sub_pool, offset);
    }

    return xjr_true;
}

static xjr_urid_t xjr_sub_pool_block_get_by_size_thereshold(xjr_mp_sub_t *sub_pool, \
        xjr_size_t size_threshold)
{
    xjr_mp_size_t offset;

    offset = sub_pool->offset_begin;

    for (;;)
    {
        if (xjr_sub_pool_is_final_block(sub_pool, offset))
        { break; }

        if (xjr_sub_pool_read_block_size(sub_pool, offset) <= size_threshold)
        {
            return xjr_sub_pool_read_block_urid(sub_pool, offset); 
        }

        offset = xjr_sub_pool_read_block_offset_next(sub_pool, offset); 
    }

    return XJR_URID_INVALID;
}


static xjr_urid_t xjr_mp_allocate_urid(xjr_mp_t *pool)
{
    xjr_urid_t new_urid = 0;
    xjr_size_t idx;

#ifdef XJR_URID_CACHE
    if (xjr_mp_urid_cache_allocate(pool, &new_urid) == 0)
    { return new_urid; }
#endif

    /* Naive implementation 
     * Iterate ranged from 0 to unlimited */

    for (; new_urid != XJR_URID_INVALID; new_urid++)
    {
        for (idx = 0; idx != pool->pools_num_allocated; idx++)
        {
            if (xjr_sub_pool_allocate_urid_try(pool->pools[idx], new_urid) == xjr_false)
            { goto next_urid; }
        }
        return new_urid;
next_urid:;
    }

    return XJR_URID_INVALID;
}

static void xjr_mp_move_data(char *dst, char *src, xjr_size_t n)
{
    while (n-- != 0) { *dst++ = *src++; }
}

/* Compact blocks in a single sub pool, align all blocks in the head of the pool */
static void xjr_sub_pool_compact(xjr_mp_sub_t *sub_pool)
{
    xjr_mp_size_t offset_src_cur = sub_pool->offset_begin;
    xjr_mp_size_t offset_src_next;

    xjr_mp_size_t offset_dst_cur = 0;
    xjr_mp_size_t offset_dst_prev = 0; xjr_bool offset_dst_prev_set = xjr_false;

    xjr_mp_size_t block_size;

    while (xjr_sub_pool_is_final_block(sub_pool, offset_src_cur) == xjr_false)
    {
        offset_src_next = xjr_sub_pool_read_block_offset_next(sub_pool, offset_src_cur);

        if (offset_dst_cur == offset_src_cur)
        {
            /* Turn to the next block */

            offset_dst_prev = offset_dst_cur; offset_dst_prev_set = xjr_true;
            offset_dst_cur = offset_dst_cur + XJR_MP_BLOCK_HEADER_SIZE + \
                             xjr_sub_pool_read_block_size(sub_pool, offset_dst_cur);

            offset_src_cur = offset_src_next;
        }
        else
        {
            /* Move the block (including the header and data) */
            block_size = xjr_sub_pool_read_block_size(sub_pool, offset_src_cur);
            xjr_mp_move_data( \
                    (char *)sub_pool->base + offset_dst_cur, \
                    (char *)sub_pool->base + offset_src_cur, \
                    XJR_MP_BLOCK_HEADER_SIZE + block_size);

            /* Update src cur */
            offset_src_cur = offset_src_next; 

            /* 
             * |dst_cur               src_cur
             * |========              ========
             * +----------------------------------*/

            /* Rechain dst_prev -> dst_cur */ 
            if (offset_dst_prev_set == xjr_false)
            {
                /* First */
                sub_pool->offset_begin = offset_dst_cur;
            }
            else
            {
                xjr_sub_pool_update_block_offset_next(sub_pool, \
                        offset_dst_prev, \
                        offset_dst_cur);
            }

            /* Rechain dst_cur -> src_cur */
            xjr_sub_pool_update_block_offset_next(sub_pool, \
                    offset_dst_cur, \
                    offset_src_cur);

            /* Update dst prev & cur */
            offset_dst_prev = offset_dst_cur; offset_dst_prev_set = xjr_true;
            offset_dst_cur = offset_dst_cur + XJR_MP_BLOCK_HEADER_SIZE + \
                             xjr_sub_pool_read_block_size(sub_pool, offset_dst_cur);
        }
    }
}

/*
static void xjr_mp_compact(xjr_mp_t *pool)
{
    xjr_size_t idx;

    for (idx = 0; idx != pool->pools_num_allocated; idx++)
    {
        xjr_sub_pool_compact(pool->pools[idx]);
    }
}
*/

/* Output the offset of the block which followed by enough spare space,
 * return true if found spare space 
 * FIXME: improve the interface */
static xjr_bool xjr_sub_pool_allocate_space( \
        xjr_mp_size_t *offset_out,\
        xjr_mp_sub_t *sub_pool, \
        xjr_size_t size, \
        xjr_u32 opts)
{
    xjr_mp_size_t offset = sub_pool->offset_begin;
    xjr_mp_size_t offset_end_of_cur_data;
    xjr_mp_size_t offset_next_block;
    xjr_mp_size_t size_hole;

retry:

    while (xjr_sub_pool_is_final_block(sub_pool, offset) == xjr_false)
    {
        offset_end_of_cur_data = offset + XJR_MP_BLOCK_HEADER_SIZE + \
                                 xjr_sub_pool_read_block_size(sub_pool, offset);
        offset_next_block = xjr_sub_pool_read_block_offset_next(sub_pool, offset);
        size_hole = offset_next_block - offset_end_of_cur_data;

        if (size_hole >= size + XJR_MP_BLOCK_HEADER_SIZE)
        {
            *offset_out = offset;
            return xjr_true;
        }

        /* Next */
        offset = offset_next_block;
    }

    if ((opts & XJR_MP_OPTS_NOALIGN) == 0)
    {
        /* Compact then retry */
        xjr_sub_pool_compact(sub_pool);
        offset = sub_pool->offset_begin;
        goto retry;
    }

    return xjr_false;
}

static xjr_urid_t xjr_sub_pool_malloc_with_urid( \
        xjr_urid_t new_urid, \
        xjr_mp_sub_t *sub_pool, \
        xjr_size_t size, \
        xjr_bool managed, \
        xjr_u32 opts)
{
    xjr_mp_size_t offset = 0;
    xjr_mp_size_t offset_end_of_cur_data;
    xjr_mp_size_t offset_next_block;

    if ((xjr_sub_pool_is_final_block(sub_pool, sub_pool->offset_begin) == xjr_true) || 
            (sub_pool->offset_begin >= size + XJR_MP_BLOCK_HEADER_SIZE))
    {
        offset = 0;
        xjr_sub_pool_write_block(sub_pool, 0, \
                managed, \
                new_urid, \
                (xjr_mp_size_t)size, \
                sub_pool->offset_begin);

        /* Update the offset of the first block */
        sub_pool->offset_begin = offset;

        /* Update the free space */
        sub_pool->size_free -= (size + XJR_MP_BLOCK_HEADER_SIZE);
    }
    else
    {
        /* Find a spare space between blocks */
        if (xjr_sub_pool_allocate_space(&offset, sub_pool, size, opts) == xjr_false)
        {
            /* There must be some INTERNAL ERRORS */
            return XJR_URID_INVALID;
        }
        offset_end_of_cur_data = offset + \
                                 XJR_MP_BLOCK_HEADER_SIZE + \
                                 xjr_sub_pool_read_block_size(sub_pool, offset);
        offset_next_block = xjr_sub_pool_read_block_offset_next(sub_pool, offset);

        /* Write the new block */
        xjr_sub_pool_write_block(sub_pool, offset_end_of_cur_data, \
                managed, \
                new_urid, \
                (xjr_mp_size_t)size, \
                offset_next_block);

        /* Chain the new block with exist ones */
        xjr_sub_pool_update_block_offset_next(sub_pool, offset, offset_end_of_cur_data);

        /* Update the free space */
        sub_pool->size_free -= (size + XJR_MP_BLOCK_HEADER_SIZE);
    }

    return new_urid;
}

static xjr_urid_t xjr_sub_pool_malloc( \
        xjr_mp_t *pool, \
        xjr_mp_sub_t *sub_pool, \
        xjr_size_t size, \
        xjr_bool managed, \
        xjr_u32 opts)
{
    xjr_urid_t new_urid = XJR_URID_UNINITIALIZED;

    /* Get a usable URID */
    if ((new_urid = xjr_mp_allocate_urid(pool)) == XJR_URID_INVALID)
    { return XJR_URID_INVALID; }

    return xjr_sub_pool_malloc_with_urid(new_urid, sub_pool, size, managed, opts);
}

static xjr_urid_t xjr_mp_malloc_in_single_sub_pool( \
        xjr_mp_t *pool, \
        xjr_size_t size, \
        xjr_bool managed)
{
    xjr_urid_t new_urid;
    xjr_size_t idx;
    xjr_mp_sub_t *sub_pool;

    /* Try to allocate in a single sub pool */ 
    for (idx = 0; idx != pool->pools_num_allocated; idx++)
    {
        sub_pool = pool->pools[idx];

        /* Is there enough space in this sub pool? */
        if (XJR_MP_BLOCK_HEADER_SIZE + size > sub_pool->size_free)
        { continue; }

        new_urid = xjr_sub_pool_malloc(pool, sub_pool, size, managed, pool->opts);
        if (new_urid != XJR_URID_INVALID)
        { return new_urid; }
    }

    return XJR_URID_INVALID;
}

static void xjr_mp_align_blocks(xjr_mp_t *pool)
{
    xjr_size_t idx;
    xjr_size_t size_free;
    xjr_size_t pool_idx_dst, pool_idx_src;
    xjr_mp_sub_t *pool_dst, *pool_src;
    xjr_urid_t urid_fitable;
    xjr_size_t size_fitable;
    xjr_mp_size_t offset_src, offset_dst;
    void *ptr_src, *ptr_dst;
    xjr_bool managed;

    /* Compact all sub pools */
    for (idx = 0; idx != pool->pools_num_allocated; idx++)
    { xjr_sub_pool_compact(pool->pools[idx]); }

    /* Merge blocks to the head. */

    /* Get start from moving to the early sub pools
     * from the following sub pools */
    for (pool_idx_dst = 0; \
            pool_idx_dst != pool->pools_num_allocated - 1; \
            pool_idx_dst++)
    {
        pool_dst = pool->pools[pool_idx_dst];
        pool_idx_src = pool_idx_dst + 1;

continue_trying:
        size_free = pool_dst->size_free;

        for (; pool_idx_src != pool->pools_num_allocated; \
                pool_idx_src++)
        {
            pool_src = pool->pools[pool_idx_src];
            if ((size_free > XJR_MP_BLOCK_HEADER_SIZE) &&
                    ((urid_fitable = xjr_sub_pool_block_get_by_size_thereshold(pool_src, \
                    size_free - XJR_MP_BLOCK_HEADER_SIZE)) != XJR_URID_INVALID))
            {
                /* Found a block that should be move with URID provided
                 * Note: MUST KEEP THE ORIGINAL URID */

                if (xjr_sub_pool_block_get_by_urid( \
                            &offset_src, pool_src, urid_fitable) == xjr_false)
                { return; }
                ptr_src = xjr_sub_pool_read_ptr(pool_src, offset_src);
                managed = xjr_sub_pool_read_block_managed(pool_src, offset_src);

                /* Create the same block in the destination sub pool */
                size_fitable = xjr_sub_pool_read_block_size(pool_src, offset_src);
                if (xjr_sub_pool_malloc_with_urid( \
                            urid_fitable, pool_dst, size_fitable, managed, pool->opts) == XJR_URID_INVALID)
                { return; }

                if (xjr_sub_pool_block_get_by_urid( \
                            &offset_dst, pool_dst, urid_fitable) == xjr_false)
                { return; }
                ptr_dst = xjr_sub_pool_read_ptr(pool_dst, offset_dst);

                /* Move the data */
                xjr_mp_move_data(ptr_dst, ptr_src, size_fitable);

                /* Remove the block in the src sub pool */
                xjr_sub_pool_free(pool_src, urid_fitable);

                goto continue_trying;
            }
        }
    }
}

/* Release empty pools */
static int xjr_mp_trim_empty_sub_pools(xjr_mp_t *pool)
{
    xjr_size_t pool_idx, pool_idx_rest;
    xjr_mp_sub_t *sub_pool;

    if (pool->pools_num_allocated == 1)
    {
        /* No extra sub pools to be trimed */
        return 0;
    }

    /* Merge non empty to the beginning positions */ 
    for (pool_idx = 0; \
            pool_idx < pool->pools_num_allocated;)
    {
        sub_pool = pool->pools[pool_idx];
        if (xjr_sub_pool_is_empty(sub_pool) != xjr_false)
        {
            xjr_sub_pool_destroy(sub_pool);
            for (pool_idx_rest = pool_idx; \
                    pool_idx_rest < pool->pools_num_allocated - 1; \
                    pool_idx_rest++)
            {
                pool->pools[pool_idx_rest] = pool->pools[pool_idx_rest + 1];
                pool->pools[pool_idx_rest + 1] = xjr_nullptr;
            }
            pool->pools[pool->pools_num_allocated - 1] = xjr_nullptr;
            pool->pools_num_allocated--;
        }
        else
        {
            pool_idx++;
        }
    }

    /* Cleared all pools, allocate at least one */
    if (pool->pools_num_allocated == 0)
    {
        /* Allocate the first sub pool */
        if ((pool->pools[0] = xjr_mp_sub_new(pool)) == xjr_nullptr)
        { return -1; }
        pool->pools_num_allocated = 1;
    }

    return 0;
}

/* Return true if success */
static xjr_bool xjr_mp_extend(xjr_mp_t *pool)
{
    /* Check if met the limit */
    if (pool->pools_num_allocated >= pool->pools_num_capacity)
    { return xjr_false; }

    /* Allocate a new sub pool */
    if ((pool->pools[pool->pools_num_allocated] = xjr_mp_sub_new(pool)) == xjr_nullptr)
    { goto fail; }
    pool->pools_num_allocated++;

    return xjr_true;
fail:
    return xjr_false;
}

static xjr_urid_t xjr_mp_malloc_opt(xjr_mp_t *pool, xjr_size_t size, \
        xjr_bool managed)
{
    xjr_urid_t new_urid;

    /* Is it possible to fit the whole block into an empty sub pool */
    if (size + XJR_MP_BLOCK_HEADER_SIZE > XJR_SUB_MP_SIZE)
    { return XJR_URID_INVALID; }

    /* Try to allocate in a single sub pool */
    new_urid = xjr_mp_malloc_in_single_sub_pool(pool, size, managed);
    if (new_urid != XJR_URID_INVALID) { goto allocated; }

    /* Failed to allocate in a single sub pool.
     * Judge if enough space available after aligning blocks to the 'early' 
     * sub pools WITHOUT extending */
    if ((pool->opts & XJR_MP_OPTS_NOALIGN) == 0)
    {
        if ((pool->pools_num_allocated > 1) && \
                (xjr_mp_size_free(pool) >= size + XJR_MP_BLOCK_HEADER_SIZE))
        {
            /* 'Potentially' possible, align blocks */
            xjr_mp_align_blocks(pool);

            /* Try again */
            new_urid = xjr_mp_malloc_in_single_sub_pool(pool, size, managed);
            if (new_urid != XJR_URID_INVALID) { goto allocated; }
        }
    }

    /* Extend */
    if (xjr_mp_extend(pool) != xjr_false)
    {
        /* Try again */
        new_urid = xjr_mp_malloc_in_single_sub_pool(pool, size, managed);
        if (new_urid != XJR_URID_INVALID) { goto allocated; }
    }

    /* Failed to extend, no more space */
    return XJR_URID_INVALID;

allocated:

    /* Update info */
    pool->size_capacity = xjr_mp_size_capacity(pool);
    pool->size_usage = pool->size_capacity - xjr_mp_size_free(pool);
    pool->num_allocated_blocks++;
    if (pool->size_capacity > pool->size_capacity_peak) pool->size_capacity_peak = pool->size_capacity;
    if (pool->size_usage > pool->size_usage_peak) pool->size_usage_peak = pool->size_usage;
    if (pool->num_allocated_blocks > pool->num_allocated_blocks_peak) pool->num_allocated_blocks_peak = pool->num_allocated_blocks;

    if (managed == xjr_true)
    {
        if (pool->cb_malloc_record != xjr_nullptr)
        {
            pool->cb_malloc_record(new_urid);
        }
    }

    return new_urid;
}

xjr_urid_t xjr_mp_malloc(xjr_mp_t *pool, xjr_size_t size)
{
    return xjr_mp_malloc_opt(pool, size, xjr_false);
}

xjr_urid_t xjr_mp_malloc_managed(xjr_mp_t *pool, xjr_size_t size)
{
    return xjr_mp_malloc_opt(pool, size, xjr_true);
}


/* Return true if successfuly freed */
static xjr_bool xjr_sub_pool_free(xjr_mp_sub_t *sub_pool, xjr_urid_t urid)
{
    xjr_mp_size_t offset_prev = 0;
    xjr_bool offset_prev_set = xjr_false;

    xjr_mp_size_t offset = sub_pool->offset_begin; 
    xjr_mp_size_t offset_next_block;
    xjr_mp_size_t size;

    /* Naive searching by iterating */
    while (xjr_sub_pool_is_final_block(sub_pool, offset) == xjr_false)
    {
        offset_next_block = xjr_sub_pool_read_block_offset_next(sub_pool, offset);

        if (xjr_sub_pool_read_block_urid(sub_pool, offset) == urid)
        {
            /* Matched */
            size = xjr_sub_pool_read_block_size(sub_pool, offset);

            /* Chain prev and next */
            if (offset_prev_set == xjr_false)
            {
                sub_pool->offset_begin = offset_next_block;
            }
            else
            {
                xjr_sub_pool_update_block_offset_next(sub_pool, offset_prev, offset_next_block);
            }

            /* Update the free space */
            sub_pool->size_free += (size + XJR_MP_BLOCK_HEADER_SIZE);

            return xjr_true;
        }

        /* Next */
        offset_prev_set = xjr_true;
        offset_prev = offset;
        offset = offset_next_block;
    }

    return xjr_false;
}

void xjr_mp_free(xjr_mp_t *pool, xjr_urid_t urid)
{
    xjr_size_t idx;
    xjr_mp_sub_t *sub_pool;

    for (idx = 0; idx != pool->pools_num_allocated; idx++)
    {
        sub_pool = pool->pools[idx];

        if (xjr_sub_pool_free(sub_pool, urid) != xjr_false)
        {
            pool->size_capacity = xjr_mp_size_capacity(pool);
            pool->size_usage = pool->size_capacity - xjr_mp_size_free(pool);
            pool->num_allocated_blocks--;
            return; 
        }
    }
}


/* Get the pointer with URID */
static void *xjr_sub_pool_get_ptr(xjr_mp_sub_t *sub_pool, xjr_urid_t urid)
{
    xjr_mp_size_t offset = sub_pool->offset_begin;

    /* Naive searching by iterating */
    while (xjr_sub_pool_is_final_block(sub_pool, offset) == xjr_false)
    {
        if (xjr_sub_pool_read_block_urid(sub_pool, offset) == urid)
        {
            return ((xjr_u8 *)sub_pool->base) + offset + XJR_MP_BLOCK_HEADER_DATA_OFFSET;
        }

        /* Next */
        offset = xjr_sub_pool_read_block_offset_next(sub_pool, offset);
    }

    /* Not found in this sub pool */
    return xjr_nullptr;
}

static xjr_urid_t xjr_sub_pool_get_urid(xjr_mp_sub_t *sub_pool, void *ptr)
{
    xjr_mp_size_t offset = sub_pool->offset_begin;

    /* Naive searching by iterating */
    while (xjr_sub_pool_is_final_block(sub_pool, offset) == xjr_false)
    {
        void *cur_ptr = ((xjr_u8 *)sub_pool->base) + offset + XJR_MP_BLOCK_HEADER_DATA_OFFSET;
        if (cur_ptr == ptr) return xjr_sub_pool_read_block_urid(sub_pool, offset);
        /* Next */
        offset = xjr_sub_pool_read_block_offset_next(sub_pool, offset);
    }

    /* Not found in this sub pool */
    return XJR_URID_INVALID;
}

void *xjr_mp_get_ptr(xjr_mp_t *pool, xjr_urid_t urid)
{
    void *ptr;
    xjr_size_t idx;
    xjr_mp_sub_t *sub_pool;

    for (idx = 0; idx != pool->pools_num_allocated; idx++)
    {
        sub_pool = pool->pools[idx];
        ptr = xjr_sub_pool_get_ptr(sub_pool, urid);
        if (ptr != xjr_nullptr)
        { return ptr; }
    }

    /* There must be something wrong,
     * either internal error or an invalid URID is given */
    return xjr_nullptr;
}

/* Get URID with pointer */
xjr_urid_t xjr_mp_get_urid(xjr_mp_t *pool, void *ptr)
{
    xjr_urid_t urid;
    xjr_size_t idx;
    xjr_mp_sub_t *sub_pool;

    for (idx = 0; idx != pool->pools_num_allocated; idx++)
    {
        sub_pool = pool->pools[idx];
        urid = xjr_sub_pool_get_urid(sub_pool, ptr);
        if (urid != XJR_URID_INVALID) { return urid; }
    }

    return XJR_URID_INVALID;
}

/* Auto Management */

static void xjr_sub_pool_clear(xjr_mp_sub_t *sub_pool)
{
    xjr_mp_size_t offset = sub_pool->offset_begin;

    while (xjr_sub_pool_is_final_block(sub_pool, offset) == xjr_false)
    {
        xjr_sub_pool_clear_block_marked(sub_pool, offset);
        /* Next */
        offset = xjr_sub_pool_read_block_offset_next(sub_pool, offset);
    }
}

void xjr_mp_clear(xjr_mp_t *pool)
{
    xjr_size_t idx;

    for (idx = 0; idx != pool->pools_num_allocated; idx++)
    { xjr_sub_pool_clear(pool->pools[idx]); }
}

static void xjr_sub_pool_mark(xjr_mp_sub_t *sub_pool, \
        xjr_urid_t urid, \
        xjr_bool *exist, xjr_bool *marked)
{
    xjr_mp_size_t offset = sub_pool->offset_begin;

    /* Naive searching by iterating */
    while (xjr_sub_pool_is_final_block(sub_pool, offset) == xjr_false)
    {
        if (xjr_sub_pool_read_block_urid(sub_pool, offset) == urid)
        {
            *exist = xjr_true;
            *marked = xjr_sub_pool_read_block_marked(sub_pool, offset);
            xjr_sub_pool_set_block_marked(sub_pool, offset);
            return;
        }
        /* Next */
        offset = xjr_sub_pool_read_block_offset_next(sub_pool, offset);
    }

    *exist = xjr_false;
    *marked = xjr_false;
}

xjr_bool xjr_mp_mark(xjr_mp_t *pool, xjr_urid_t urid)
{
    xjr_size_t idx;
    xjr_bool exist, marked;

    for (idx = 0; idx != pool->pools_num_allocated; idx++)
    {
        xjr_sub_pool_mark(pool->pools[idx], urid, &exist, &marked);
        if (exist != xjr_false)
        { return marked; }
    }

    return xjr_false;
}

static void xjr_sub_pool_sweep(xjr_mp_t *pool, xjr_mp_sub_t *sub_pool)
{
    xjr_mp_size_t offset = sub_pool->offset_begin, offset_next;
    xjr_urid_t urid;

    /* Naive searching by iterating */
    while (xjr_sub_pool_is_final_block(sub_pool, offset) == xjr_false)
    {
        offset_next = xjr_sub_pool_read_block_offset_next(sub_pool, offset);

        if (xjr_sub_pool_read_block_marked(sub_pool, offset) == xjr_false)
        {
            /* Not marked, clean */
            urid = xjr_sub_pool_read_block_urid(sub_pool, offset);

            if (pool->cb_free_record != xjr_nullptr)
            {
                pool->cb_free_record(urid);
            }

#ifdef XJR_URID_CACHE
            xjr_mp_urid_cache_release(pool, urid);
#endif
            xjr_sub_pool_free(sub_pool, urid);
            pool->num_allocated_blocks--;
        }

        /* Next */
        offset = offset_next;
    }
}

void xjr_mp_sweep(xjr_mp_t *pool)
{
    xjr_size_t idx;

    for (idx = 0; idx != pool->pools_num_allocated; idx++)
    {
        xjr_sub_pool_sweep(pool, pool->pools[idx]); 
    }

    xjr_mp_trim_empty_sub_pools(pool);

    pool->size_capacity = xjr_mp_size_capacity(pool);
    pool->size_usage = pool->size_capacity - xjr_mp_size_free(pool);
}

