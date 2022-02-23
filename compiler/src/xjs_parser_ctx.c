/* XenonJS : Parser : Context
 * Copyright(c) 2017 y2c2 */

#include "xjs_error.h"
#include "xjs_parser_ctx.h"

#define _basename(_fullname) \
    (strrchr(_fullname, '/') ? strrchr(_fullname, '/') + 1 : \
     (strrchr(_fullname, '\\') ? strrchr(_fullname, '\\') + 1 : _fullname))

void xjs_parser_ctx_init( \
        xjs_parser_ctx *ctx, \
        xjs_error_ref err, \
        xjs_token_list_ref tokens, \
        token_iterator_stack *snapshot)
{
    ctx->err = err;
    ctx->tokens = tokens;
    xjs_token_list_iterator_init_begin(&ctx->it, tokens);
    xjs_token_list_iterator_init_end(&ctx->it_end, tokens);
    ctx->source_filename = NULL;
    ctx->snapshot = snapshot;
}

xjs_bool xjs_parser_ctx_eof(xjs_parser_ctx *ctx)
{
    return (xjs_bool)xjs_token_list_iterator_eq(&ctx->it, &ctx->it_end);
}

void xjs_parser_ctx_next(xjs_parser_ctx *ctx)
{
    xjs_token_list_iterator_next(&ctx->it);
}

xjs_token_ref xjs_parser_ctx_peek_token(xjs_parser_ctx *ctx)
{
    return ect_deref(xjs_token_ref, ctx->it);
}

xjs_token_ref xjs_parser_ctx_peek_prev_token(xjs_parser_ctx *ctx)
{
    xjs_token_ref token;

    /* TODO: does previous item exist? */
    {
        xjs_token_list_iterator it_begin;
        xjs_token_list_iterator_init_begin(&it_begin, ctx->tokens);
        if (xjs_token_list_iterator_eq(&it_begin, &ctx->it) == ec_true)
        {
            return ect_deref(xjs_token_ref, it_begin);
        }
    }

    xjs_token_list_iterator_prev(&ctx->it);
    token = ect_deref(xjs_token_ref, ctx->it);
    xjs_token_list_iterator_next(&ctx->it);
    return token;
}

void xjs_parser_ctx_error_raw(xjs_parser_ctx *ctx, int err_no, \
        const char *__file__, const xjs_size_t __line__)
{
    xjs_error_update(ctx->err, __file__, __line__, err_no);
}

void xjs_parser_ctx_set_source_filename( \
        xjs_parser_ctx *ctx, const char *source_filename)
{
    ctx->source_filename = _basename(source_filename);
}

static void ctor1(ect_iterator(xjs_token_list) *key)
{
    ec_free(key);
}

ect_list_define_declared(token_iterator_list, ect_iterator(xjs_token_list) *, ctor1);
ect_stack_define(token_iterator_stack, ect_iterator(xjs_token_list) *, token_iterator_list);

int xjs_parser_snapshot_save( \
        xjs_parser_ctx *ctx)
{
    ect_iterator(xjs_token_list) *new_it = ec_malloc(sizeof(ect_iterator(xjs_token_list)));
    if (new_it == NULL) return -1;
    memcpy(new_it, &ctx->it, sizeof(ect_iterator(xjs_token_list)));
    ect_stack_push(token_iterator_stack, ctx->snapshot, new_it);
    return 0;
}

int xjs_parser_snapshot_restore( \
        xjs_parser_ctx *ctx)
{
    if (ect_stack_size(token_iterator_stack, ctx->snapshot) == 0) return -1;
    memcpy(&ctx->it, ect_stack_top(token_iterator_stack, ctx->snapshot), sizeof(ect_iterator(xjs_token_list)));
    ect_stack_pop(token_iterator_stack, ctx->snapshot);
    return 0;
}

