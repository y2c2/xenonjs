/* Enhanced C : Exception
 * Copyright(c) 2017-2020 y2c2 */

/* Exception:
 *
 * Sample:
 * ----------------------
 * #include <ec_exception.h>
 * ec_exception_t expt;
 *
 * ec_exception_init(&expt, ExceptionCount);
 * ec_exception_define_root_item(&expt, "Exception", Exception);
 * ec_exception_define_item(&expt, "BooleanException", BooleanException,
 *                          Exception);
 * ec_exception_define_item(&expt, "TrueException", TrueException,
 *                          BooleanException);
 * ec_exception_define_item(&expt, "FalseException", FalseException,
 *                          BooleanException);
 * ec_try { } ec_catch(expt, Exception) { } ec_exception_uninit(&expt);
 * ----------------------
 */

#ifndef EC_EXCEPTION_H
#define EC_EXCEPTION_H

#include "ec_alloc.h"
#include "ec_dt.h"
#include "ec_exception_dt.h"

void ec_exception_allocator_set_malloc(ec_malloc_cb_t cb);
void ec_exception_allocator_set_calloc(ec_calloc_cb_t cb);
void ec_exception_allocator_set_free(ec_free_cb_t cb);
void ec_exception_allocator_set_memcpy(ec_memcpy_cb_t cb);
void ec_exception_allocator_set_memset(ec_memset_cb_t cb);

struct ec_exception;
typedef struct ec_exception ec_exception_t;

/* Init Exception */
int ec_exception_init(ec_exception_t* expt, ec_size_t pool_capacity);
int ec_exception_uninit(ec_exception_t* expt);

/* Define exception items */
int ec_exception_define_root_item(ec_exception_t* expt, const char* name,
                                  int exception_root);
int ec_exception_define_item(ec_exception_t* expt, const char* name,
                             int exception_item, int exception_base);

/* Throw an exception */
int ec_throw_raw(ec_exception_t* expt, ec_exception_no_t exception_no);

/* Catch an exception */
int ec_catch_raw(ec_exception_t* expt, ec_exception_no_t exception_no);

#define ec_try                                                                 \
    for (ec_bool looped_once = ec_false; looped_once == ec_false;              \
         looped_once = ec_true)

#define ec_throw(_expt, _exception_no)                                         \
    ec_throw_raw(_expt, _exception_no);                                        \
    break

#define ec_catch(_expt, _exception_no) if (ec_catch_raw(_expt, _exception_no))

#endif
