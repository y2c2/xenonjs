/* XenonJS : Parser : Context
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_PARSER_CTX_H
#define XJS_PARSER_CTX_H

#include <ec_algorithm.h>
#include <ec_list.h>
#include <ec_stack.h>
#include "xjs_dt.h"
#include "xjs_types.h"

ect_list_declare(token_iterator_list, ect_iterator(xjs_token_list) *);
ect_stack_declare(token_iterator_stack, ect_iterator(xjs_token_list) *, token_iterator_list);

typedef struct
{
    xjs_error_ref err;
    xjs_token_list_ref tokens;
    ect_iterator(xjs_token_list) it;
    ect_iterator(xjs_token_list) it_end;
    const char *source_filename;

    token_iterator_stack *snapshot;
} xjs_parser_ctx;

void xjs_parser_ctx_init( \
        xjs_parser_ctx *ctx, \
        xjs_error_ref err, \
        xjs_token_list_ref tokens, \
        token_iterator_stack *snapshot);

xjs_bool xjs_parser_ctx_eof(xjs_parser_ctx *ctx);

void xjs_parser_ctx_next(xjs_parser_ctx *ctx);

xjs_token_ref xjs_parser_ctx_peek_token(xjs_parser_ctx *ctx);

xjs_token_ref xjs_parser_ctx_peek_prev_token(xjs_parser_ctx *ctx);

void xjs_parser_ctx_error_raw(xjs_parser_ctx *ctx, int err_no, \
        const char *__file__, const xjs_size_t __line__);

void xjs_parser_ctx_set_source_filename( \
        xjs_parser_ctx *ctx, const char *source_filename);

int xjs_parser_snapshot_save( \
        xjs_parser_ctx *ctx);

int xjs_parser_snapshot_restore( \
        xjs_parser_ctx *ctx);

#define xjs_parser_ctx_error(_ctx, _err_no) \
    xjs_parser_ctx_error_raw(_ctx, _err_no, __FILE__, __LINE__)

#endif

