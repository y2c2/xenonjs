/* XenonJS : Lexer : Context
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_LEXER_CTX_H
#define XJS_LEXER_CTX_H

#include <ec_algorithm.h>
#include <ec_string.h>
#include "xjs_dt.h"
#include "xjs_types.h"

typedef struct
{
    xjs_error_ref err;
    ect_iterator(ec_string) it;
    ect_iterator(ec_string) it_end;
    xjs_size_t ln, col;
    xjs_size_t idx;
    const char *source_filename;
} xjs_lexer_ctx;

void xjs_lexer_ctx_init( \
        xjs_lexer_ctx *ctx, \
        xjs_error_ref err, \
        ec_string *src_code);

xjs_bool xjs_lexer_ctx_eof(xjs_lexer_ctx *ctx);

void xjs_lexer_ctx_next(xjs_lexer_ctx *ctx);

xjs_char_t xjs_lexer_ctx_peekch(xjs_lexer_ctx *ctx);
xjs_char_t xjs_lexer_ctx_getch(xjs_lexer_ctx *ctx);
xjs_bool xjs_lexer_ctx_remain_at_least(xjs_lexer_ctx *ctx, xjs_size_t n);
xjs_size_t xjs_lexer_ctx_loc_ln_get(xjs_lexer_ctx *ctx);
xjs_size_t xjs_lexer_ctx_loc_col_get(xjs_lexer_ctx *ctx);
xjs_size_t xjs_lexer_ctx_idx_get(xjs_lexer_ctx *ctx);

void xjs_lexer_ctx_error_raw(xjs_lexer_ctx *ctx, int err_no, \
        const char *__file__, const xjs_size_t __line__);

void xjs_lexer_ctx_set_source_filename( \
        xjs_lexer_ctx *ctx, const char *source_filename);

#define xjs_lexer_ctx_error(_ctx, _err_no) \
    xjs_lexer_ctx_error_raw(_ctx, _err_no, __FILE__, __LINE__)


#endif

