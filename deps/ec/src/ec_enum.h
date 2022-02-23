/* Enhanced C : Container : Enum
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_ENUM_H
#define EC_ENUM_H

#include "ec_alloc.h"
#include "ec_dt.h"
#include "ec_interface.h"
#include <stdarg.h>

#define ect_enum_declare(_typename, ...)                                       \
    typedef enum                                                               \
    {                                                                          \
        __VA_ARGS__                                                            \
    } _typename##_tag;                                                         \
    struct _typename;                                                          \
    typedef struct _typename _typename

#define ect_enum_declare_begin(_typename)                                      \
    struct _typename;                                                          \
    typedef struct _typename _typename;                                        \
    struct _typename                                                           \
    {                                                                          \
        _typename##_tag tag;                                                   \
        union                                                                  \
        {

#define ect_enum_declare_end()                                                 \
    }                                                                          \
    u;                                                                         \
    }

#endif
