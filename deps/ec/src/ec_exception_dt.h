/* Enhanced C : Exception : Data Types
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_EXCEPTION_DT_H
#define EC_EXCEPTION_DT_H

#include "ec_dt.h"

struct ec_exception;
typedef struct ec_exception ec_exception_t;
struct ec_exception_item;
typedef struct ec_exception_item ec_exception_item_t;

typedef int ec_exception_no_t;
#define ec_exception_no_null (-1)

struct ec_exception_item
{
    ec_exception_no_t _base;
};

struct ec_exception
{
    /* Defined items */
    ec_exception_no_t _root;
    /* Items Pool */
    ec_exception_item_t* items_pool;
    ec_size_t items_pool_size;
    ec_size_t items_pool_capacity;
    /* Raised */
    ec_bool raised;
    ec_exception_no_t raised_exception_no;
};

#endif
