/* XenonJS : Types : Error
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_TYPES_ERROR_H
#define XJS_TYPES_ERROR_H

#include <ec_string.h>
#include "xjs_dt.h"

enum
{
    XJS_ERRNO_INTERNAL = 1,
    XJS_ERRNO_MEM = 2,
    XJS_ERRNO_NOTIMP = 3,
    XJS_ERRNO_IO = 4,
    XJS_ERRNO_LEX = 5,
    XJS_ERRNO_PARSE = 6,
    XJS_ERRNO_COMPILE = 7,
    XJS_ERRNO_LINK = 8,
    XJS_ERRNO_RUNTIME = 9,
};

struct xjs_opaque_error
{
    xjs_bool occurred;
    int error_no;
    struct
    {
        const char *filename;
        xjs_size_t ln;
    } loc;
    ec_string *desc;
};
typedef struct xjs_opaque_error xjs_error;
typedef struct xjs_opaque_error *xjs_error_ref;

#endif

