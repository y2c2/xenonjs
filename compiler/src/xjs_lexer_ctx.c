/* XenonJS : Lexer : Context
 * Copyright(c) 2017 y2c2 */

#include "xjs_error.h"
#include "xjs_lexer_ctx.h"

#define LN_START 1
#define COL_START 0
#define IDX_START 1

void xjs_lexer_ctx_init( \
        xjs_lexer_ctx *ctx, \
        xjs_error_ref err, \
        ec_string *src_code)
{
    ctx->err = err;
    ec_string_iterator_init_begin(&ctx->it, src_code);
    ec_string_iterator_init_end(&ctx->it_end, src_code);
    ctx->ln = LN_START;
    ctx->col = COL_START;
    ctx->idx = IDX_START;
}

xjs_bool xjs_lexer_ctx_eof(xjs_lexer_ctx *ctx)
{
    return (xjs_bool)ec_string_iterator_eq(&ctx->it, &ctx->it_end);
}

void xjs_lexer_ctx_next(xjs_lexer_ctx *ctx)
{
    xjs_char_t ch = ect_deref(ec_char_t , ctx->it);
    if (ch == (xjs_char_t)'\n')
    {
        ctx->ln++;
        ctx->col = COL_START;
    }
    else
    {
        ctx->col++;
    }
    ec_string_iterator_next(&ctx->it);
    ctx->idx++;
}

xjs_char_t xjs_lexer_ctx_peekch(xjs_lexer_ctx *ctx)
{
    return ect_deref(ec_char_t , ctx->it);
}

xjs_char_t xjs_lexer_ctx_getch(xjs_lexer_ctx *ctx)
{
    xjs_char_t ch = ect_deref(ec_char_t , ctx->it);
    xjs_lexer_ctx_next(ctx);
    return ch;
}

xjs_bool xjs_lexer_ctx_remain_at_least(xjs_lexer_ctx *ctx, xjs_size_t n)
{
    xjs_bool ret = xjs_false;
    xjs_size_t moved_step = 0;

    if (n == 0) return xjs_true;

    for (;;)
    {
        if (xjs_lexer_ctx_eof(ctx)) break;
        ec_string_iterator_next(&ctx->it);
        ctx->idx++;
        n--; moved_step++;
        if (n == 0) { ret = xjs_true; break; }
    }
    while (moved_step-- != 0)
    {
        ec_string_iterator_prev(&ctx->it);
        ctx->idx--;
    }

    return ret;
}

xjs_size_t xjs_lexer_ctx_loc_ln_get(xjs_lexer_ctx *ctx)
{
    return ctx->ln;
}

xjs_size_t xjs_lexer_ctx_loc_col_get(xjs_lexer_ctx *ctx)
{
    return ctx->col;
}

xjs_size_t xjs_lexer_ctx_idx_get(xjs_lexer_ctx *ctx)
{
    return ctx->idx;
}

void xjs_lexer_ctx_error_raw(xjs_lexer_ctx *ctx, int err_no, \
        const char *__file__, const xjs_size_t __line__)
{
    xjs_error_update(ctx->err, __file__, __line__, err_no);
}

void xjs_lexer_ctx_set_source_filename( \
        xjs_lexer_ctx *ctx, const char *source_filename)
{
    ctx->source_filename = source_filename;
}

