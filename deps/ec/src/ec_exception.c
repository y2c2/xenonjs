/* Enhanced C : Exception
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_alloc.h"
#include "ec_dt.h"
#include "ec_exception_dt.h"

/* Exception System needs its own memory allocator and pre-allocation */

/* Global stuff */
static ec_malloc_cb_t g_ec_malloc = NULL;
static ec_calloc_cb_t g_ec_calloc = NULL;
static ec_free_cb_t g_ec_free = NULL;
static ec_memcpy_cb_t g_ec_memcpy = NULL;
static ec_memset_cb_t g_ec_memset = NULL;

void ec_exception_allocator_set_malloc(ec_malloc_cb_t cb);
void ec_exception_allocator_set_calloc(ec_calloc_cb_t cb);
void ec_exception_allocator_set_free(ec_free_cb_t cb);
void ec_exception_allocator_set_memcpy(ec_memcpy_cb_t cb);
void ec_exception_allocator_set_memset(ec_memset_cb_t cb);

void ec_exception_allocator_set_malloc(ec_malloc_cb_t cb) { g_ec_malloc = cb; }
void ec_exception_allocator_set_calloc(ec_calloc_cb_t cb) { g_ec_calloc = cb; }
void ec_exception_allocator_set_free(ec_free_cb_t cb) { g_ec_free = cb; }
void ec_exception_allocator_set_memcpy(ec_memcpy_cb_t cb) { g_ec_memcpy = cb; }
void ec_exception_allocator_set_memset(ec_memset_cb_t cb) { g_ec_memset = cb; }

static void* ec_exception_malloc(ec_size_t size) { return g_ec_malloc(size); }
static void ec_exception_free(void* ptr) { g_ec_free(ptr); }

int ec_exception_init(ec_exception_t* expt, ec_size_t pool_capacity);
int ec_exception_uninit(ec_exception_t* expt);
int ec_exception_define_root_item(ec_exception_t* expt, const char* name,
                                  int exception_root);
int ec_exception_define_item(ec_exception_t* expt, const char* name,
                             int exception_item, int exception_base);
int ec_throw_raw(ec_exception_t* expt, ec_exception_no_t exception_no);
int ec_catch_raw(ec_exception_t* expt, ec_exception_no_t exception_no);

/* Init Exception */
int ec_exception_init(ec_exception_t* expt, ec_size_t pool_capacity)
{
    expt->_root = ec_exception_no_null;
    expt->items_pool_size = 0;
    expt->items_pool_capacity = pool_capacity;
    if ((expt->items_pool = (ec_exception_item_t*)ec_exception_malloc(
             sizeof(ec_exception_item_t) * pool_capacity)) == NULL)
    {
        return -1;
    }
    expt->raised = ec_false;
    expt->raised_exception_no = 0;

    return 0;
}

int ec_exception_uninit(ec_exception_t* expt)
{
    if (expt->items_pool != NULL)
    {
        ec_exception_free(expt->items_pool);
    }
    return 0;
}

/* Define exception items */
int ec_exception_define_root_item(ec_exception_t* expt, const char* name,
                                  int exception_root)
{
    (void)name;
    expt->items_pool[exception_root]._base = ec_exception_no_null;
    expt->_root = exception_root;
    expt->items_pool_size++;
    return 0;
}

int ec_exception_define_item(ec_exception_t* expt, const char* name,
                             int exception_item, int exception_base)
{
    (void)name;
    expt->items_pool[exception_item]._base = exception_base;
    expt->items_pool_size++;
    return 0;
}

/* Throw an exception */
int ec_throw_raw(ec_exception_t* expt, ec_exception_no_t exception_no)
{
    expt->raised = ec_true;
    expt->raised_exception_no = exception_no;
    return 0;
}

/* Catch an exception */
int ec_catch_raw(ec_exception_t* expt, ec_exception_no_t exception_no)
{
    int raised_exception_no_cur = expt->raised_exception_no;
    /* No exception throwed */
    if (expt->raised == ec_false)
        return 0;
    do
    {
        if (exception_no == raised_exception_no_cur)
        {
            expt->raised = ec_false;
            return 1;
        }
        /* Go base */
        raised_exception_no_cur =
            expt->items_pool[raised_exception_no_cur]._base;
    } while (raised_exception_no_cur != ec_exception_no_null);
    return 0;
}
