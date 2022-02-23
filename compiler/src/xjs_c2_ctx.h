/* XenonJS : C2 : Context
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_C2_CTX_H
#define XJS_C2_CTX_H

#include <ec_set.h>
#include <ec_list.h>
#include <ec_stack.h>
#include <ec_string.h>
#include "xjs_dt.h"
#include "xjs_types.h"
#include "xjs_error.h"
#include "xjs_irbuilder.h"

typedef struct
{
    xjs_error_ref err;
    xjs_irbuilder_ref irb;
    ec_string *filename;
} xjs_c2_ctx;

void xjs_c2_ctx_init( \
        xjs_c2_ctx *ctx, \
        xjs_error_ref err, \
        xjs_irbuilder_ref irb);

void xjs_c2_ctx_error_raw(xjs_c2_ctx *ctx, int err_no, \
        const char *__file__, const xjs_size_t __line__);

#define xjs_c2_ctx_error(_ctx, _err_no) \
    xjs_c2_ctx_error_raw(_ctx, _err_no, __FILE__, __LINE__)

#endif

