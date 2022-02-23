/* XenonJS : Runtime Time System : Memory Pool
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_MP_H
#define XJR_MP_H

#include "xjr_dt.h"

#define XJR_MP_OPTS_NOALIGN (1 << 0)

#ifndef XJR_KB
#define XJR_KB (1024)
#endif
#ifndef XJR_MB
#define XJR_MB ((1024) * XJR_KB)
#endif

/* Sub Pool Size (Customizable) */
#ifndef XJR_SUB_MP_SIZE
# define XJR_SUB_MP_SIZE (4 * XJR_KB)
#endif

/* Sub Pool Maximum Number (Customizable) */
#ifndef XJR_SUB_MP_MAX_NUM
# define XJR_SUB_MP_MAX_NUM (20)
#endif


/* Unified Resource Identifier Width (Customizable)
 *
 * Affects the total number of block that can
 * be managed by the pool.
 * 
 * The default width is 2 which means 65535 blocks
 * can be managed by the pool and the URID type is
 * a 16-bit unsigned number */

#ifndef XJR_DATATYPE_URID_WIDTH
# define XJR_DATATYPE_URID_WIDTH 2
#endif


/* Width of pointers pool */
#ifndef XJR_MP_WIDTH 
#if (XJR_SUB_MP_SIZE <= (64 * XJR_KB))
# define XJR_MP_WIDTH 2
#else 
# define XJR_MP_WIDTH 4
#endif
#endif

#ifndef XJR_DATATYPE_MP_SIZE
 # define XJR_DATATYPE_MP_SIZE
# if XJR_MP_WIDTH == 2
   typedef xjr_u16 xjr_mp_size_t;
# elif XJR_MP_WIDTH == 4
   typedef xjr_u32 xjr_mp_size_t;
# else
#  error "Unsupported pool width"
# endif
#endif


/* Unified Resource Identifier */
#ifndef XJR_DATATYPE_URID
# define XJR_DATATYPE_URID

# if XJR_DATATYPE_URID_WIDTH == 2
   typedef xjr_u16 xjr_urid_t;
#  define XJR_URID_REFINE_MASK (0x3FFF)
# elif XJR_DATATYPE_URID_WIDTH == 4
   typedef xjr_u32 xjr_urid_t;
#  define XJR_URID_REFINE_MASK (0x3FFFFFFF)
# else
#  error "Unsupported URID width"
# endif

/* Managed | Marked  | URID
 * 1 bit   | 1 bit   | 14 bits */

#define XJR_URID_INVALID (XJR_URID_REFINE_MASK)
#define XJR_URID_UNINITIALIZED (XJR_URID_INVALID)
#define XJR_URID_FINAL (XJR_URID_INVALID - 1)

#define XJR_URID_MARKED_MASK (XJR_URID_INVALID + 1)
#define XJR_URID_MANAGED_MASK ((XJR_URID_INVALID + 1) << 1)

#define XJR_URID_UPBOUND_LIMIT ((1 << 14) - 2)

#endif


struct xjr_mp;

#ifndef XJR_DATATYPE_POOL
#define XJR_DATATYPE_POOL
typedef struct xjr_mp xjr_mp_t;
#endif

typedef void (*xjr_heap_malloc_record_callback)(xjr_urid_t urid);
typedef void (*xjr_heap_free_record_callback)(xjr_urid_t urid);

/* Create/Destroy pool */
xjr_mp_t *xjr_mp_new_opt( \
        void *heap_data, \
        xjr_heap_malloc_callback cb_malloc, \
        xjr_heap_free_callback cb_free, \
        xjr_size_t urid_cache_size, \
        xjr_u32 opts);
xjr_mp_t *xjr_mp_new( \
        void *heap_data, \
        xjr_heap_malloc_callback cb_malloc, \
        xjr_heap_free_callback cb_free);
void xjr_mp_destroy(xjr_mp_t *pool);

/* Set record callbacks (For debugging purpose) */
void xjr_mp_set_malloc_callback(xjr_mp_t *pool, xjr_heap_malloc_record_callback cb);
void xjr_mp_set_free_callback(xjr_mp_t *pool, xjr_heap_free_record_callback cb);

/* Status */
xjr_size_t xjr_mp_size_free(xjr_mp_t *pool);
xjr_size_t xjr_mp_size_capacity(xjr_mp_t *pool);
xjr_size_t xjr_mp_size_usage_peak(xjr_mp_t *pool);
xjr_size_t xjr_mp_size_capacity_peak(xjr_mp_t *pool);
xjr_size_t xjr_mp_number_allocated_block_peak(xjr_mp_t *pool);

/* Debug */
xjr_size_t xjr_mp_number_sub_pools(xjr_mp_t *pool);
xjr_size_t xjr_mp_number_block(xjr_mp_t *pool);
xjr_urid_t xjr_mp_block_urid(xjr_mp_t *pool, xjr_size_t n);
xjr_mp_size_t xjr_mp_block_size(xjr_mp_t *pool, xjr_size_t n);
xjr_mp_size_t xjr_mp_block_offset_next(xjr_mp_t *pool, xjr_size_t n);

/* Allocate/Release */
xjr_urid_t xjr_mp_malloc(xjr_mp_t *pool, xjr_size_t size);
void xjr_mp_free(xjr_mp_t *pool, xjr_urid_t urid);

/* Get pointer with URID */
void *xjr_mp_get_ptr(xjr_mp_t *pool, xjr_urid_t urid);
/* Get URID with pointer */
xjr_urid_t xjr_mp_get_urid(xjr_mp_t *pool, void *ptr);

/* Auto Management */
xjr_urid_t xjr_mp_malloc_managed(xjr_mp_t *pool, xjr_size_t size);
void xjr_mp_clear(xjr_mp_t *pool);
/* Return true when marked */
xjr_bool xjr_mp_mark(xjr_mp_t *pool, xjr_urid_t urid);
void xjr_mp_sweep(xjr_mp_t *pool);

#endif

