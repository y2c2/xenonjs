/* XenonJS : C2 : Context
 * Copyright(c) 2017 y2c2 */

#include "xjs_c2_ctx.h"

void xjs_c2_ctx_init( \
        xjs_c2_ctx *ctx, \
        xjs_error_ref err, \
        xjs_irbuilder_ref irb)
{
    ctx->irb = irb;
    ctx->err = err;
    ctx->filename = NULL;
}

void xjs_c2_ctx_error_raw(xjs_c2_ctx *ctx, int err_no, \
        const char *__file__, const xjs_size_t __line__)
{
    xjs_error_update(ctx->err, __file__, __line__, err_no);
}

