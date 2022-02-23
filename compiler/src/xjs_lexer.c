/* XenonJS : Lexer
 * Copyright(c) 2017 y2c2 */

#include <ec_libc.h>
#include <ec_algorithm.h>
#include <ec_string.h>
#include "xjs_types.h"
#include "xjs_error.h"
#include "xjs_token.h"
#include "xjs_helper.h"
#include "xjs_aux.h"
#include "xjs_lexer_helper.h"
#include "xjs_lexer_ctx.h"
#include "xjs_lexer.h"

#define LEX_ERROR_PRINTF(_fmt, ...) \
    do { \
        xjs_error_update(ctx.err, __FILE__, __LINE__, XJS_ERRNO_LEX); \
        xjs_error_update_desc_printf(ctx.err, _fmt, __VA_ARGS__); \
        xjs_lexer_ctx_error(&ctx, XJS_ERRNO_LEX); \
        goto fail; \
    } while (0)


/* https://www.ecma-international.org/ecma-262/8.0/index.html#sec-keywords */
static xjs_bool ec_string_match_with_keyword(ec_string *s)
{
    const char *keywords[] = {
        "await", "as",
        "break",
        "case", "catch", "class", "const", "continue",
        "debugger", "default", "delete", "do",
        "else", "export", "extends",
        "finally", "for", "from", "function",
        "if", "import", "in", "instanceof",
        "let",
        "new",
        "return",
        "static", "super", "switch",
        "this", "throw", "try", "typeof",
        "var", "void",
        "while", "with",
        "yield",
        "inspect", /* for testing */
    };
    xjs_size_t idx;

    for (idx = 0; idx != sizeof(keywords) / sizeof(const char *); idx++)
    {
        if (xjs_aux_string_match_with(s, keywords[idx]))
        { return xjs_true; }
    }

    return xjs_false;
}

static xjs_token_ref
xjs_lexer_get_token_identifier(xjs_lexer_ctx *ctx)
{
    xjs_token_ref new_token = NULL;
    ec_string *value = NULL;
    ec_char_t ch;

    XJS_VNZ_ERROR(value = ec_string_new(), ctx->err, XJS_ERRNO_MEM);
    ec_string_push_back(value, xjs_lexer_ctx_getch(ctx));

    while (!xjs_lexer_ctx_eof(ctx))
    {
        ch = xjs_lexer_ctx_peekch(ctx);
        if (!XJS_CHAR_IS_UNICODE_ID_CONTINUE(ch)) break;

        ec_string_push_back(value, ch);
        xjs_lexer_ctx_next(ctx);
    }

    XJS_VNZ_ERROR(new_token = xjs_token_new(), ctx->err, XJS_ERRNO_MEM);
    if (xjs_aux_string_match_with(value, "null"))
    { xjs_token_type_set(new_token, XJS_TOKEN_TYPE_NULL_LITERAL); }
    else if (xjs_aux_string_match_with(value, "false") || xjs_aux_string_match_with(value, "true"))
    { xjs_token_type_set(new_token, XJS_TOKEN_TYPE_BOOLEAN_LITERAL); }
    else if (ec_string_match_with_keyword(value))
    { xjs_token_type_set(new_token, XJS_TOKEN_TYPE_KEYWORD); }
    else
    { xjs_token_type_set(new_token, XJS_TOKEN_TYPE_IDENTIFIER); }
    xjs_token_value_set(new_token, value); value = NULL;

fail:
    ec_delete(value);
    return new_token;
}

static xjs_token_ref
xjs_lexer_get_token_string_literal(xjs_lexer_ctx *ctx)
{
    xjs_token_ref new_token = NULL;
    ec_string *value = NULL;
    ec_char_t ch;
    ec_char_t ch_start;

    XJS_VNZ_ERROR(value = ec_string_new(), ctx->err, XJS_ERRNO_MEM);

    /* Starts with ' or " */
    ch_start = xjs_lexer_ctx_getch(ctx);
    ec_string_push_back(value, ch_start);

    while (!xjs_lexer_ctx_eof(ctx))
    {
        ch = xjs_lexer_ctx_getch(ctx);
        ec_string_push_back(value, ch);

        if (ch == ch_start) { break; }
        else if (ch == '\\')
        {
            if (xjs_lexer_ctx_eof(ctx))
            { xjs_lexer_ctx_error(ctx, XJS_ERRNO_LEX); goto fail; }

            ch = xjs_lexer_ctx_getch(ctx);
            ec_string_push_back(value, ch);
        }
    }

    XJS_VNZ_ERROR(new_token = xjs_token_new(), ctx->err, XJS_ERRNO_MEM);
    xjs_token_type_set(new_token, XJS_TOKEN_TYPE_STRING_LITERAL);
    xjs_token_value_set(new_token, value); value = NULL;

fail:
    ec_delete(value);
    return new_token;
}

/* https://www.ecma-international.org/ecma-262/8.0/index.html#sec-literals-numeric-literals */

typedef enum
{
    XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_STATE_INIT,
    /* accepts: [0-9], '.' */

    XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_STATE_0,
    /* accepts:
     *   [0-7]
     *   '.' (note: 010 == 08 == 8)
     *   b, B
     *   x, X
     */

    XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_STATE_0B,
    /* accepts:
     *   [0-1] */

    XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_STATE_0X,
    /* accepts:
     *   [0-9a-fA-F] */

    XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_STATE_DECINT,
    /* accepts: [0-9], '.' */

    XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_STATE_OCT,
    /* accepts: [0-9], '.' */

    XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_STATE_DECDOT,
    /* accepts: [0-9] */

} xjs_lexer_get_token_numeric_literal_state;

static xjs_token_ref
xjs_lexer_get_token_numeric_literal(xjs_lexer_ctx *ctx)
{
    xjs_lexer_get_token_numeric_literal_state state;
    xjs_token_ref new_token = NULL;
    ec_string *value = NULL;
    ec_char_t ch;

    XJS_VNZ_ERROR(value = ec_string_new(), ctx->err, XJS_ERRNO_MEM);

    /* Initialize the state */
    state = XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_STATE_INIT;

#define MOV() \
    do { ec_string_push_back(value, ch); xjs_lexer_ctx_next(ctx); } while (0)

#define JMP(_state) \
    do { state = XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_##_state; } while (0)

#define MOVJMP(_state) \
    do { MOV(); JMP(_state); } while (0)

    /*
#define FAIL() \
    do { xjs_lexer_ctx_error(ctx, XJS_ERRNO_LEX); goto fail; } while (0)
    */

#define FINISH() \
    do { goto finish; } while (0)

    while (!xjs_lexer_ctx_eof(ctx))
    {
        ch = xjs_lexer_ctx_peekch(ctx);

        switch (state)
        {
            case XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_STATE_INIT:
                if (ch == '0') { MOVJMP(STATE_0); }
                else if (ch == '.') { MOVJMP(STATE_DECDOT); }
                else if (('1' <= ch) && (ch <= '9')) { MOVJMP(STATE_DECINT); }
                else { FINISH(); }
                break;

            case XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_STATE_0:
                if ((ch == 'b') || (ch == 'B')) { MOVJMP(STATE_0B); }
                else if ((ch == 'x') || (ch == 'X')) { MOVJMP(STATE_0X); }
                else if (ch == '.') { MOVJMP(STATE_DECDOT); }
                else if (('0' <= ch) && (ch <= '7')) { MOVJMP(STATE_OCT); }
                else { FINISH(); }
                break;

            case XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_STATE_0B:
                if ((ch == '0') || (ch == '1')) { MOV(); }
                else { FINISH(); }
                break;

            case XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_STATE_0X:
                if (XJS_CHAR_IS_DIGIT_HEX(ch)) { MOV(); }
                else { FINISH(); }
                break;

            case XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_STATE_DECINT:
                if (XJS_CHAR_IS_DIGIT_DEC(ch)) { MOV(); }
                else if (ch == '.') { MOVJMP(STATE_DECDOT); }
                else { FINISH(); }
                break;

            case XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_STATE_DECDOT:
                if (XJS_CHAR_IS_DIGIT_DEC(ch)) { MOV(); }
                else { FINISH(); }
                break;

            case XJS_LEXER_GET_TOKEN_NUMERIC_LITERAL_STATE_OCT:
                if (XJS_CHAR_IS_DIGIT_OCT(ch)) { MOV(); }
                else { FINISH(); }
                break;
        }
    }

finish:
#undef FINISH
#undef MOV
#undef JMP
#undef MOVJMP
    /*
#undef FAIL
*/

    XJS_VNZ_ERROR(new_token = xjs_token_new(), ctx->err, XJS_ERRNO_MEM);
    xjs_token_type_set(new_token, XJS_TOKEN_TYPE_NUMERIC_LITERAL);
    xjs_token_value_set(new_token, value); value = NULL;

fail:
    ec_delete(value);
    return new_token;
}

static xjs_token_ref
xjs_lexer_get_token_dot(xjs_lexer_ctx *ctx)
{
    xjs_token_ref new_token = NULL;
    ec_string *value = NULL;
    xjs_token_type type = XJS_TOKEN_TYPE_UNKNOWN;
    ec_char_t ch;

    XJS_VNZ_ERROR(value = ec_string_new(), ctx->err, XJS_ERRNO_MEM);
    ch = xjs_lexer_ctx_getch(ctx);
    ec_string_push_back(value, ch);

    if (!xjs_lexer_ctx_eof(ctx) && \
            XJS_CHAR_IS_DIGIT_DEC(xjs_lexer_ctx_peekch(ctx)))
    {
        do
        {
            ch = xjs_lexer_ctx_getch(ctx);
            ec_string_push_back(value, ch);
        } while (!xjs_lexer_ctx_eof(ctx) && XJS_CHAR_IS_DIGIT_DEC(ch));
        type = XJS_TOKEN_TYPE_NUMERIC_LITERAL;
    }
    else
    {
        type = XJS_TOKEN_TYPE_PUNCTUATOR;
    }

    XJS_VNZ_ERROR(new_token = xjs_token_new(), ctx->err, XJS_ERRNO_MEM);
    xjs_token_type_set(new_token, type);
    xjs_token_value_set(new_token, value); value = NULL;

fail:
    ec_delete(value);
    return new_token;
}

/* https://www.ecma-international.org/ecma-262/8.0/index.html#prod-Punctuator */
static xjs_token_ref
xjs_lexer_get_token_punctuator(xjs_lexer_ctx *ctx)
{
    xjs_token_ref new_token = NULL;
    ec_string *value = NULL;
    ec_char_t ch;

    XJS_VNZ_ERROR(value = ec_string_new(), ctx->err, XJS_ERRNO_MEM);

#define MOV() \
    do { ec_string_push_back(value, ch); xjs_lexer_ctx_next(ctx); } while (0)

#define TRY_MORE() \
    if (!xjs_lexer_ctx_eof(ctx) && ((ch = xjs_lexer_ctx_peekch(ctx)) || 1))

    ch = xjs_lexer_ctx_peekch(ctx);
    switch (ch)
    {
        case '{': case '}': case '(': case ')':
        case '[': case ']': case ';':
        case ',': case '~': case '?':
        case ':':
            MOV();
            break;

        case '<':
            MOV(); /* < */
            TRY_MORE()
            {
                if (ch == '=') { MOV(); } /* <= */
                else if (ch == '<') /* << */
                {
                    MOV();
                    TRY_MORE()
                    {
                        if (ch == '=') { MOV(); } /* <<= */
                    }
                }
            }
            break;

        case '>':
            MOV(); /* > */
            TRY_MORE()
            {
                if (ch == '=') { MOV(); } /* >= */
                else if (ch == '>') /* >> */
                {
                    MOV();
                    TRY_MORE()
                    {
                        if (ch == '=') { MOV(); } /* >>= */
                        else if (ch == '>') /* >>> */
                        {
                            MOV();
                            TRY_MORE()
                            {
                                if (ch == '=') { MOV(); } /* >>>= */
                            }
                        }
                    }
                }
            }
            break;

        case '=':
            MOV(); /* = */
            TRY_MORE()
            {
                if (ch == '=') /* == */
                {
                    MOV();
                    TRY_MORE()
                    {
                        if (ch == '=') { MOV(); }/* === */
                    }
                }
                else if (ch == '>') { MOV(); } /* => */
            }
            break;

        case '!':
            MOV(); /* ! */
            TRY_MORE()
            {
                if (ch == '=') /* != */
                {
                    MOV();
                    TRY_MORE()
                    {
                        if (ch == '=') { MOV(); }/* !== */
                    }
                }
            }
            break;

        case '+':
            MOV(); /* + */
            TRY_MORE()
            {
                if (ch == '=') { MOV(); } /* += */
                else if (ch == '+') { MOV(); } /* ++ */
            }
            break;

        case '-':
            MOV(); /* - */
            TRY_MORE()
            {
                if (ch == '=') { MOV(); } /* -= */
                else if (ch == '-') { MOV(); } /* -- */
            }
            break;

        case '*':
            MOV(); /* * */
            TRY_MORE()
            {
                if (ch == '=') { MOV(); } /* *= */
                else if (ch == '*') /* ** */
                {
                    MOV();
                    TRY_MORE()
                    {
                        if (ch == '=') { MOV(); } /* **= */
                    }
                }
            }
            break;

        case '%':
            MOV(); /* % */
            TRY_MORE()
            {
                if (ch == '=') { MOV(); } /* %= */
            }
            break;

        case '&':
            MOV(); /* & */
            TRY_MORE()
            {
                if (ch == '=') { MOV(); } /* &= */
                else if (ch == '&') { MOV(); } /* && */
            }
            break;

        case '|':
            MOV(); /* | */
            TRY_MORE()
            {
                if (ch == '=') { MOV(); } /* |= */
                else if (ch == '|') { MOV(); } /* || */
            }
            break;

        case '^':
            MOV(); /* ^ */
            TRY_MORE()
            {
                if (ch == '=') { MOV(); } /* ^= */
            }
            break;

        case '/':
            MOV(); /* / */
            TRY_MORE()
            {
                if (ch == '=') { MOV(); } /* /= */
            }
            break;

        default:
            xjs_lexer_ctx_error(ctx, XJS_ERRNO_LEX);
            goto fail;
    }

#undef TRY_MORE
#undef MOV

    XJS_VNZ_ERROR(new_token = xjs_token_new(), ctx->err, XJS_ERRNO_MEM);
    xjs_token_type_set(new_token, XJS_TOKEN_TYPE_PUNCTUATOR);
    xjs_token_value_set(new_token, value); value = NULL;

fail:
    ec_delete(value);
    return new_token;
}

typedef enum
{
    XJS_LEXER_GET_COMMENT_STATE_INIT,
} xjs_lexer_get_comment_state;

static xjs_token_ref
xjs_lexer_get_comment(xjs_lexer_ctx *ctx)
{
    ec_string *value = NULL;
    xjs_token_ref new_token = NULL;
    ec_char_t ch = xjs_lexer_ctx_peekch(ctx);
    xjs_size_t start_ln, start_col, start_idx;

    if (ch == '/')
    {
        start_ln = xjs_lexer_ctx_loc_ln_get(ctx);
        start_col = xjs_lexer_ctx_loc_col_get(ctx);
        start_idx = xjs_lexer_ctx_idx_get(ctx);

        xjs_lexer_ctx_next(ctx);
        ch = xjs_lexer_ctx_peekch(ctx);
        if (xjs_lexer_ctx_eof(ctx))
        {
            XJS_VNZ_ERROR(new_token = xjs_token_new(), ctx->err, XJS_ERRNO_MEM);
            xjs_token_type_set(new_token, XJS_TOKEN_TYPE_PUNCTUATOR);
            {
                value = ec_string_new_assign_c_str("/"); 
                XJS_VNZ_ERROR_MEM(value, ctx->err);
                xjs_token_value_set(new_token, value);
                value = NULL;
            }
            xjs_token_loc_start_set(new_token, start_ln, start_col);
            xjs_token_loc_end_set(new_token, \
                    xjs_lexer_ctx_loc_ln_get(ctx), xjs_lexer_ctx_loc_col_get(ctx));
            xjs_token_range_start_set(new_token, start_idx);
            xjs_token_range_end_set(new_token, xjs_lexer_ctx_idx_get(ctx));
        }
        else if (ch == '/')
        {
            /* to end of line comment */
            value = ec_string_new_assign_c_str("//"); 
            XJS_VNZ_ERROR_MEM(value, ctx->err);

            while (!xjs_lexer_ctx_eof(ctx) && \
                    (xjs_lexer_ctx_peekch(ctx) != '\n') && \
                    (xjs_lexer_ctx_peekch(ctx) != '\r'))
            {
                ec_string_push_back(value, xjs_lexer_ctx_peekch(ctx));
                xjs_lexer_ctx_next(ctx);
            }

            XJS_VNZ_ERROR(new_token = xjs_token_new(), ctx->err, XJS_ERRNO_MEM);
            xjs_token_type_set(new_token, XJS_TOKEN_TYPE_COMMENT);
            xjs_token_value_set(new_token, value);
            xjs_token_loc_start_set(new_token, start_ln, start_col);
            xjs_token_loc_end_set(new_token, \
                    xjs_lexer_ctx_loc_ln_get(ctx), xjs_lexer_ctx_loc_col_get(ctx));
            xjs_token_range_start_set(new_token, start_idx);
            xjs_token_range_end_set(new_token, xjs_lexer_ctx_idx_get(ctx));
            value = NULL;
        }
        else if (ch == '=')
        {
            /* Skip '=' */
            xjs_lexer_ctx_next(ctx);
            XJS_VNZ_ERROR(new_token = xjs_token_new(), ctx->err, XJS_ERRNO_MEM);
            xjs_token_type_set(new_token, XJS_TOKEN_TYPE_PUNCTUATOR);
            {
                value = ec_string_new_assign_c_str("/="); 
                XJS_VNZ_ERROR_MEM(value, ctx->err);
                xjs_token_value_set(new_token, value);
                value = NULL;
            }
            xjs_token_loc_start_set(new_token, start_ln, start_col);
            xjs_token_loc_end_set(new_token, \
                    xjs_lexer_ctx_loc_ln_get(ctx), xjs_lexer_ctx_loc_col_get(ctx));
            xjs_token_range_start_set(new_token, start_idx);
            xjs_token_range_end_set(new_token, xjs_lexer_ctx_idx_get(ctx));
        }
        else if (ch == '*')
        {
            ec_bool at_star = ec_false, at_finish = ec_false;

            /* block comment */
            xjs_lexer_ctx_next(ctx);
            value = ec_string_new_assign_c_str("/*"); 
            XJS_VNZ_ERROR_MEM(value, ctx->err);

            while (!xjs_lexer_ctx_eof(ctx))
            {
                ch = xjs_lexer_ctx_peekch(ctx);
                xjs_lexer_ctx_next(ctx);
                ec_string_push_back(value, xjs_lexer_ctx_peekch(ctx));

                if (at_star == ec_false)
                {
                    if (ch == '*') { at_star = ec_true; }
                }
                else /* if (at_star == ec_true) */
                {
                    if (ch == '/') { at_finish = ec_true; break; }
                    if (ch == '*') { }
                    else { at_star = ec_false; }
                }
            }

            if (at_finish == ec_true)
            {
                XJS_VNZ_ERROR(new_token = xjs_token_new(), ctx->err, XJS_ERRNO_MEM);
                xjs_token_type_set(new_token, XJS_TOKEN_TYPE_COMMENT);
                xjs_token_value_set(new_token, value);
                xjs_token_loc_start_set(new_token, start_ln, start_col);
                xjs_token_loc_end_set(new_token, \
                    xjs_lexer_ctx_loc_ln_get(ctx), xjs_lexer_ctx_loc_col_get(ctx));
                xjs_token_range_start_set(new_token, start_idx);
                xjs_token_range_end_set(new_token, xjs_lexer_ctx_idx_get(ctx));
                value = NULL;
            }
            else
            {
                ec_delete(value);
                value = NULL;
            }
        }
        else
        {
            /* Just a simple punctuator '/' */
            value = ec_string_new_assign_c_str("/"); 
            XJS_VNZ_ERROR_MEM(value, ctx->err);
            XJS_VNZ_ERROR(new_token = xjs_token_new(), ctx->err, XJS_ERRNO_MEM);
            xjs_token_type_set(new_token, XJS_TOKEN_TYPE_PUNCTUATOR);
            xjs_token_value_set(new_token, value); value = NULL;
            xjs_token_loc_start_set(new_token, start_ln, start_col);
            xjs_token_loc_end_set(new_token, \
                    xjs_lexer_ctx_loc_ln_get(ctx), xjs_lexer_ctx_loc_col_get(ctx));
            xjs_token_range_start_set(new_token, start_idx);
            xjs_token_range_end_set(new_token, xjs_lexer_ctx_idx_get(ctx));
        }
    }
    else if ((ch == '#') && \
            (xjs_lexer_ctx_loc_ln_get(ctx) == 1) && (xjs_lexer_ctx_loc_col_get(ctx) == 0))
    {
        /* Shebang? (#!) */
        start_ln = xjs_lexer_ctx_loc_ln_get(ctx);
        start_col = xjs_lexer_ctx_loc_col_get(ctx);
        start_idx = xjs_lexer_ctx_idx_get(ctx);

        xjs_lexer_ctx_next(ctx);
        ch = xjs_lexer_ctx_peekch(ctx);
        if (xjs_lexer_ctx_eof(ctx)) { goto fail; }
        if (ch == '!')
        {
            /* to end of line comment */
            value = ec_string_new_assign_c_str("#!"); 
            XJS_VNZ_ERROR_MEM(value, ctx->err);

            while (!xjs_lexer_ctx_eof(ctx) && \
                    (xjs_lexer_ctx_peekch(ctx) != '\n') && \
                    (xjs_lexer_ctx_peekch(ctx) != '\r'))
            {
                ec_string_push_back(value, xjs_lexer_ctx_peekch(ctx));
                xjs_lexer_ctx_next(ctx);
            }

            XJS_VNZ_ERROR(new_token = xjs_token_new(), ctx->err, XJS_ERRNO_MEM);
            xjs_token_type_set(new_token, XJS_TOKEN_TYPE_COMMENT);
            xjs_token_value_set(new_token, value);
            xjs_token_loc_start_set(new_token, start_ln, start_col);
            xjs_token_loc_end_set(new_token, \
                    xjs_lexer_ctx_loc_ln_get(ctx), xjs_lexer_ctx_loc_col_get(ctx));
            xjs_token_range_start_set(new_token, start_idx);
            xjs_token_range_end_set(new_token, xjs_lexer_ctx_idx_get(ctx));
            value = NULL;
        }
    }

    /* Not a comment */
    goto done;
fail:
    if (new_token != NULL)
    {
        ec_delete(new_token);
        new_token = NULL;
    }
done:
    ec_delete(value);
    return new_token;
}

static xjs_token_ref
xjs_lexer_get_token(xjs_lexer_ctx *ctx)
{
    ec_char_t ch = xjs_lexer_ctx_peekch(ctx);

    if (XJS_CHAR_IS_UNICODE_ID_START(ch))
    { return xjs_lexer_get_token_identifier(ctx); }
    else if ((ch == '\'') || (ch == '\"'))
    { return xjs_lexer_get_token_string_literal(ctx); }
    else if (XJS_CHAR_IS_DIGIT_DEC(ch))
    { return xjs_lexer_get_token_numeric_literal(ctx); }
    else if (ch == '.')
    {
        /* Float-point number or just a '.' */
        return xjs_lexer_get_token_dot(ctx);
    }
    else
    {
        return xjs_lexer_get_token_punctuator(ctx);
    }
}

xjs_token_list_ref
xjs_lexer_start_ex( \
        xjs_error_ref err, \
        ec_string *src_code, \
        const char *filename)
{
    xjs_lexer_ctx ctx;
    xjs_token_list_ref new_list = NULL;
    xjs_token_ref new_token = NULL, last_token = NULL;
    xjs_size_t start_ln, start_col, start_idx;

    XJS_VNZ_ERROR_MEM(new_list = xjs_token_list_new(), err);

    xjs_lexer_ctx_init(&ctx, err, src_code);
    xjs_lexer_ctx_set_source_filename(&ctx, filename);

    while (!xjs_lexer_ctx_eof(&ctx))
    {
        /* Skip Whitespace */
        while (!xjs_lexer_ctx_eof(&ctx) && \
                XJS_CHAR_IS_WS(xjs_lexer_ctx_peekch(&ctx)))
        { xjs_lexer_ctx_next(&ctx); }

        /* EOF */
        if (xjs_lexer_ctx_eof(&ctx)) break;
        start_ln = xjs_lexer_ctx_loc_ln_get(&ctx);
        start_col = xjs_lexer_ctx_loc_col_get(&ctx);
        start_idx = xjs_lexer_ctx_idx_get(&ctx);

        if ((new_token = xjs_lexer_get_comment(&ctx)) != NULL)
        {
            /* Comment */
            if (xjs_token_type_get(new_token) == XJS_TOKEN_TYPE_COMMENT)
            {
                /* Ignore the comment */
                ec_delete(new_token); new_token = NULL;
                continue;
            }
            else
            {
                last_token = new_token;
                xjs_token_list_push_back(new_list, new_token); 
            }
        }
        else if ((new_token = xjs_lexer_get_token(&ctx)) != NULL)
        {
            /* Normal Token */
            xjs_token_loc_start_set(new_token, start_ln, start_col);
            xjs_token_loc_end_set(new_token, \
                    xjs_lexer_ctx_loc_ln_get(&ctx), \
                    xjs_lexer_ctx_loc_col_get(&ctx));
            xjs_token_range_start_set(new_token, start_idx);
            xjs_token_range_end_set(new_token, xjs_lexer_ctx_idx_get(&ctx));
            last_token = new_token;
            xjs_token_list_push_back(new_list, new_token); 
        }
        else
        {
            xjs_lexer_ctx_error(&ctx, XJS_ERRNO_LEX);
            LEX_ERROR_PRINTF("{c_str}:{size_t}:{size_t} :" \
                    "error: invalid or unexpected token", \
                    ctx.source_filename, ctx.ln, ctx.col + 1);
            goto fail;
        }
    }

    /* EOF */
    {
        new_token = xjs_token_new();
        xjs_token_type_set(new_token, XJS_TOKEN_TYPE_EOF);
        xjs_token_value_set(new_token, ec_string_new());
        if (last_token != NULL)
        {
            xjs_token_loc_start_set(new_token, xjs_token_loc_end_ln_get(last_token), xjs_token_loc_end_col_get(last_token));
            xjs_token_loc_end_set(new_token, xjs_token_loc_end_ln_get(last_token), xjs_token_loc_end_col_get(last_token) + 1);
            xjs_token_range_start_set(new_token, xjs_token_range_end_get(last_token));
            xjs_token_range_end_set(new_token, xjs_token_range_end_get(last_token) + 1);
        }
        else
        {
            xjs_token_loc_start_set(new_token, xjs_lexer_ctx_loc_ln_get(&ctx), xjs_lexer_ctx_loc_col_get(&ctx));
            xjs_token_loc_end_set(new_token, xjs_lexer_ctx_loc_ln_get(&ctx), xjs_lexer_ctx_loc_col_get(&ctx));
            xjs_token_range_start_set(new_token, xjs_lexer_ctx_idx_get(&ctx));
            xjs_token_range_end_set(new_token, xjs_lexer_ctx_idx_get(&ctx));
        }
        xjs_token_list_push_back(new_list, new_token); 
    }

    goto done;
fail:
    if (new_list != NULL)
    { ec_delete(new_list); new_list = NULL; }
done:
    return new_list;
}

xjs_token_list_ref
xjs_lexer_start( \
        xjs_error_ref err, \
        ec_string *src_code)
{
    static const char *untitled = "untitled";
    return xjs_lexer_start_ex( \
            err, \
            src_code, \
            untitled);
}

