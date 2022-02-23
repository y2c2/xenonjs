/* XenonJS : Parser
 * Copyright(c) 2017 y2c2 */

#include <ec_libc.h>
#include <ec_algorithm.h>
#include <ec_string.h>
#include "xjs_types.h"
#include "xjs_error.h"
#include "xjs_token.h"
#include "xjs_ast.h"
#include "xjs_helper.h"
#include "xjs_aux.h"
#include "xjs_parser_ctx.h"
#include "xjs_parser.h"

/* Macros to help parsing */

#define PARSE_NAME(_routine) \
    xjs_parse_##_routine

#define DEF_PARSE_ROUTINE(_routine, _ret_type) \
    static _ret_type xjs_parse_##_routine(xjs_parser_ctx *ctx)

#define DEF_PARSE_ROUTINE_ARG(_routine, _ret_type, _arg) \
    static _ret_type xjs_parse_##_routine(xjs_parser_ctx *ctx, _arg)

/*
#define DEF_PARSE_ROUTINE_EMPTY(_routine, _ret_type) \
    static _ret_type xjs_parse_##_routine(xjs_parser_ctx *ctx) { \
        xjs_parser_ctx_error(ctx, XJS_ERRNO_NOTIMP); \
        return NULL; }
        */

#define PARSE(_routine) ((PARSE_NAME(_routine))(ctx))

#define PARSEA(_routine, _arg) ((PARSE_NAME(_routine))(ctx, _arg))

#define PARSE_START(_routine, _ctx) ((PARSE_NAME(_routine))(_ctx))

#define PARSE_CTX (ctx)

#define PARSE_ERR (ctx->err)

#define PARSE_SRCLOC ctx->source_filename

#define PARSE_EOF() \
    (xjs_parser_ctx_eof(ctx) || \
     (xjs_token_type_get(xjs_parser_ctx_peek_token(ctx)) == XJS_TOKEN_TYPE_EOF))

#define PARSE_PEEK() (xjs_parser_ctx_peek_token(ctx))
#define PARSE_PEEK_PREV() (xjs_parser_ctx_peek_prev_token(ctx))

#define PARSE_SKIP() do { xjs_parser_ctx_next(ctx); } while (0)

/*
#define PARSE_ERROR_PUTS(_s) \
    do { \
        xjs_error_update(PARSE_CTX->err, __FILE__, __LINE__, XJS_ERRNO_PARSE); \
        xjs_error_update_desc_puts(PARSE_CTX->err, _s); \
        xjs_parser_ctx_error(PARSE_CTX, XJS_ERRNO_PARSE); \
        goto fail; \
    } while (0)
    */

#define PARSE_ERROR_PRINTF(_fmt, ...) \
    do { \
        xjs_error_update(PARSE_CTX->err, __FILE__, __LINE__, XJS_ERRNO_PARSE); \
        xjs_error_update_desc_printf(PARSE_CTX->err, _fmt, __VA_ARGS__); \
        goto fail; \
    } while (0)

#define PARSE_ERROR_PRINTF_UNEXPECTED_TOKEN(token) \
    do { \
        xjs_error_update(ctx->err, __FILE__, __LINE__, XJS_ERRNO_PARSE); \
        PARSE_ERROR_PRINTF("{c_str}:{size_t}:{size_t} :" \
                "error: unexpected token '{string}'", \
                PARSE_SRCLOC, xjs_token_loc_start_ln_get(token), xjs_token_loc_start_col_get(token) + 1, \
                xjs_token_value_get(token)); \
    } while (0)

static void parse_error_printf_expected_before( \
        xjs_parser_ctx *ctx, \
        const char *__file__, const xjs_size_t __line__, \
        const xjs_token_ref token, \
        const char *expected_thing)
{
    xjs_error_update(ctx->err, __file__, __line__, XJS_ERRNO_PARSE);
    if (xjs_token_type_get(token) == XJS_TOKEN_TYPE_EOF)
    {
        xjs_error_update_desc_printf(ctx->err, \
                "{c_str}:{size_t}:{size_t}: " \
                "error: expected {c_str} at end of input", \
                PARSE_SRCLOC, xjs_token_loc_start_ln_get(token), xjs_token_loc_start_col_get(token) + 1, \
                expected_thing);
    }
    else
    {
        xjs_error_update_desc_printf(ctx->err, \
                "{c_str}:{size_t}:{size_t}: " \
                "error: expected {c_str} before '{string}'", \
                PARSE_SRCLOC, xjs_token_loc_start_ln_get(token), xjs_token_loc_start_col_get(token) + 1, \
                expected_thing, \
                xjs_token_value_get(token));
    }
}

#define PARSE_ERROR_PRINTF_EXPECTED_BEFORE(_token, _expected) \
    do { \
        parse_error_printf_expected_before( \
                PARSE_CTX, __FILE__, __LINE__, \
                _token, _expected); \
        goto fail; \
    } while (0)

#define PARSE_LOC_CLONE_FROM_TOKEN_START(_dst, _token) \
    do { \
        (_dst)->loc.start.ln = (int)xjs_token_loc_start_ln_get(_token); \
        (_dst)->loc.start.col = (int)xjs_token_loc_start_col_get(_token); \
    } while (0)

#define PARSE_LOC_CLONE_FROM_TOKEN_END(_dst, _token) \
    do { \
        (_dst)->loc.end.ln = (int)xjs_token_loc_end_ln_get(_token); \
        (_dst)->loc.end.col = (int)xjs_token_loc_end_col_get(_token); \
    } while (0)

#define PARSE_RANGE_CLONE_FROM_TOKEN_START(_dst, _token) \
    do { \
        (_dst)->range.start = (int)xjs_token_range_start_get(_token); \
    } while (0)

#define PARSE_RANGE_CLONE_FROM_TOKEN_END(_dst, _token) \
    do { \
        (_dst)->range.end = (int)xjs_token_range_end_get(_token); \
    } while (0)

#define PARSE_RANGE_CLONE_FROM_TOKEN(_dst, _token) \
    do { \
        PARSE_RANGE_CLONE_FROM_TOKEN_START(_dst, _token); \
        PARSE_RANGE_CLONE_FROM_TOKEN_END(_dst, _token); \
    } while (0)

#define PARSE_LOC_CLONE_FROM_TOKEN(_dst, _token) \
    do { \
        PARSE_LOC_CLONE_FROM_TOKEN_START(_dst, _token); \
        PARSE_LOC_CLONE_FROM_TOKEN_END(_dst, _token); \
    } while (0)

#define PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(_dst, _token) \
    do { \
        PARSE_LOC_CLONE_FROM_TOKEN_START(_dst, _token); \
        PARSE_RANGE_CLONE_FROM_TOKEN_START(_dst, _token); \
    } while (0)

#define PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(_dst, _token) \
    do { \
        PARSE_LOC_CLONE_FROM_TOKEN_END(_dst, _token); \
        PARSE_RANGE_CLONE_FROM_TOKEN_END(_dst, _token); \
    } while (0)

#define PARSE_LOC_RANGE_CLONE_FROM_TOKEN(_dst, _token) \
    do { \
        PARSE_LOC_CLONE_FROM_TOKEN(_dst, _token); \
        PARSE_RANGE_CLONE_FROM_TOKEN(_dst, _token); \
    } while (0)


/* Keywords */

typedef enum
{
    xjs_parser_keyword_unknown,
    xjs_parser_keyword_await,
    xjs_parser_keyword_as,
    xjs_parser_keyword_break,
    xjs_parser_keyword_case,
    xjs_parser_keyword_catch,
    xjs_parser_keyword_class,
    xjs_parser_keyword_const,
    xjs_parser_keyword_continue,
    xjs_parser_keyword_debugger,
    xjs_parser_keyword_default,
    xjs_parser_keyword_delete,
    xjs_parser_keyword_do,
    xjs_parser_keyword_else,
    xjs_parser_keyword_export,
    xjs_parser_keyword_extends,
    xjs_parser_keyword_finally,
    xjs_parser_keyword_for,
    xjs_parser_keyword_from,
    xjs_parser_keyword_function,
    xjs_parser_keyword_if,
    xjs_parser_keyword_import,
    xjs_parser_keyword_in,
    xjs_parser_keyword_instanceof,
    xjs_parser_keyword_let,
    xjs_parser_keyword_new,
    xjs_parser_keyword_return,
    xjs_parser_keyword_static,
    xjs_parser_keyword_super,
    xjs_parser_keyword_switch,
    xjs_parser_keyword_this,
    xjs_parser_keyword_throw,
    xjs_parser_keyword_try,
    xjs_parser_keyword_typeof,
    xjs_parser_keyword_var,
    xjs_parser_keyword_void,
    xjs_parser_keyword_while,
    xjs_parser_keyword_with,
    xjs_parser_keyword_yield,

    /* For testing */
    xjs_parser_keyword_inspect,
} xjs_parser_keyword_t;

static xjs_parser_keyword_t
xjs_parser_keyword_type_resolve(xjs_token_ref token)
{
    ec_string *value = xjs_token_value_get(token);
    ec_size_t len = (ec_size_t)ec_string_length(value);

#define MATCH_KEYWORD(x) \
    do { \
        if (xjs_aux_string_match_with(value, XJS_ID_STRINGIFY(x))) \
            return xjs_parser_keyword_##x; \
    } while (0)

    switch (len)
    {
        case 2:
            MATCH_KEYWORD(as);
            MATCH_KEYWORD(do);
            MATCH_KEYWORD(if);
            MATCH_KEYWORD(in);
            break;

        case 3:
            MATCH_KEYWORD(for);
            MATCH_KEYWORD(let); /* strict mode */
            MATCH_KEYWORD(new);
            MATCH_KEYWORD(try);
            MATCH_KEYWORD(var);
            break;

        case 4:
            MATCH_KEYWORD(case);
            MATCH_KEYWORD(else);
            MATCH_KEYWORD(from);
            MATCH_KEYWORD(this);
            MATCH_KEYWORD(void);
            MATCH_KEYWORD(with);
            break;

        case 5:
            MATCH_KEYWORD(await);
            MATCH_KEYWORD(break);
            MATCH_KEYWORD(catch);
            MATCH_KEYWORD(class);
            MATCH_KEYWORD(const);
            MATCH_KEYWORD(super);
            MATCH_KEYWORD(throw);
            MATCH_KEYWORD(while);
            MATCH_KEYWORD(yield);
            break;

        case 6:
            MATCH_KEYWORD(import);
            MATCH_KEYWORD(return);
            MATCH_KEYWORD(switch);
            MATCH_KEYWORD(typeof);
            MATCH_KEYWORD(delete);
            MATCH_KEYWORD(export);
            break;

        default:
            {
                if (xjs_aux_string_match_with(value, "$inspect")) \
                    return xjs_parser_keyword_inspect; \
            }
            MATCH_KEYWORD(inspect);
            MATCH_KEYWORD(continue);
            MATCH_KEYWORD(debugger);
            MATCH_KEYWORD(default);
            MATCH_KEYWORD(extends);
            MATCH_KEYWORD(finally);
            MATCH_KEYWORD(function);
            MATCH_KEYWORD(instanceof);
            break;
    }
#undef MATCH_KEYWORD

    return xjs_parser_keyword_unknown;
}

static ec_bool
xjs_parser_match_punctuator(xjs_token_ref token, const char *punctuator)
{
    ec_bool ret;
    xjs_token_type token_type;
    ec_string *token_value;
    ec_string *s = NULL;

    if ((token_type = xjs_token_type_get(token)) != XJS_TOKEN_TYPE_PUNCTUATOR)
    { return ec_false; }
    token_value = xjs_token_value_get(token);
    if ((s = ec_string_new()) == NULL)
    { return ec_false; }
    ret = ec_string_eq(ec_string_assign_c_str(s, punctuator), token_value);
    ec_delete(s);

    return ret;
}

static ec_bool
xjs_parser_match_keyword(xjs_token_ref token, const xjs_parser_keyword_t keyword)
{
    xjs_token_type token_type;

    token_type = xjs_token_type_get(token);
    if ((token_type == XJS_TOKEN_TYPE_KEYWORD) && \
            (xjs_parser_keyword_type_resolve(token) == keyword))
    { return ec_true; }

    return ec_false;
}

static ec_bool
xjs_parser_match_assignment(xjs_token_ref token)
{
    xjs_token_type token_type;
    ec_string *token_value;

    if ((token_type = xjs_token_type_get(token)) != XJS_TOKEN_TYPE_PUNCTUATOR)
    { return ec_false; }
    token_value = xjs_token_value_get(token);

#define MATCH_STR(x) \
    if (xjs_aux_string_match_with(token_value, x)) return ec_true;

    MATCH_STR("=");
    MATCH_STR("+=");
    MATCH_STR("-=");
    MATCH_STR("*=");
    MATCH_STR("/=");
    MATCH_STR("%=");
    MATCH_STR("**=");
    MATCH_STR("<<=");
    MATCH_STR(">>=");
    MATCH_STR(">>>=");
    MATCH_STR("&=");
    MATCH_STR("|=");
    MATCH_STR("^=");

#undef MATCH_STR

    return ec_false;
}

typedef enum
{
    XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_INIT,
    XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_OPEN,
    XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_ESCAPE,
    XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_ESCAPEU,
    XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_ESCAPEX,
    XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_CLOSE,
} xjs_parser_extract_string_literal_value_state_t;

static ec_string *xjs_parser_extract_string_literal_value(ec_string *s)
{
    xjs_parser_extract_string_literal_value_state_t state;
    ec_string *result = NULL;
    ec_char_t starter = 0;

    state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_INIT;

    if ((result = ec_string_new()) == NULL)
    { return NULL; }

    {
        ect_iterator(ec_string) it;
        ect_for(ec_string, s, it)
        {
            ec_char_t ch = ect_deref(ec_char_t, it);
            switch (state)
            {
                case XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_INIT:
                    if (ch == '\"')
                    {
                        state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_OPEN;
                        starter = '\"';
                    }
                    else if (ch == '\'')
                    {
                        state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_OPEN;
                        starter = '\'';
                    }
                    else
                    {
                        goto fail;
                    }
                    break;

                case XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_OPEN:
                    if (ch == '\\')
                    {
                        state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_ESCAPE;
                    }
                    else if (ch == starter)
                    {
                        state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_CLOSE;
                    }
                    else
                    {
                        ec_string_push_back(result, ch);
                    }
                    break;

                case XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_ESCAPE:
                    if (ch == '0') { ec_string_push_back(result, '\0'); state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_OPEN; }
                    else if (ch == '\'') { ec_string_push_back(result, '\''); state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_OPEN; }
                    else if (ch == '\"') { ec_string_push_back(result, '\"'); state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_OPEN; }
                    else if (ch == '\\') { ec_string_push_back(result, '\\'); state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_OPEN; }
                    else if (ch == 'n') { ec_string_push_back(result, '\n'); state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_OPEN; }
                    else if (ch == 'r') { ec_string_push_back(result, '\r'); state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_OPEN; }
                    else if (ch == 'v') { ec_string_push_back(result, '\v'); state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_OPEN; }
                    else if (ch == 't') { ec_string_push_back(result, '\t'); state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_OPEN; }
                    else if (ch == 'b') { ec_string_push_back(result, '\b'); state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_OPEN; }
                    else if (ch == 'f') { ec_string_push_back(result, '\f'); state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_OPEN; }
                    else if (ch == 'u')
                    {
                        goto fail;
                    }
                    else
                    {
                        if (ch == starter)
                        {
                            state = XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_CLOSE;
                        }
                        else
                        {
                            goto fail;
                        }
                    }
                    break;

                case XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_ESCAPEU:
                    goto fail;

                case XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_ESCAPEX:
                    goto fail;
                case XJS_PARSER_EXTRACT_STRING_LITERAL_VALUE_STATE_IN_CLOSE:
                    goto fail;
            }
        }
    }

    goto done;
fail:
    ec_delete(result);
    result = NULL;
done:
    return result;
}

typedef enum
{
    xjs_parser_extract_number_literal_value_state_integer,
    xjs_parser_extract_number_literal_value_state_dot,
    xjs_parser_extract_number_literal_value_state_fractal,
} xjs_parser_extract_number_literal_value_state_t;

static int xjs_parser_extract_number_literal_value( \
        double *value_out, \
        ec_string *s)
{
    int ret = 0;
    xjs_parser_extract_number_literal_value_state_t state;
    double value = 0.0, base = 0.0;
    int radix = 10;

    if (ec_string_length(s) >= 2)
    {
        if ((ec_string_at(s, 0) == '0') && \
                ('0' <= ec_string_at(s, 1)) && (ec_string_at(s, 1) <= '7'))
        {
            radix = 8;
        }
        else if ((ec_string_at(s, 0) == '0') && \
                ((ec_string_at(s, 1) == 'x') || (ec_string_at(s, 1) == 'X')))
        {
            radix = 16;
        }
    }

    state = xjs_parser_extract_number_literal_value_state_integer;

    {
        ect_iterator(ec_string) it;
        ect_for(ec_string, s, it)
        {
            ec_char_t ch = ect_deref(ec_char_t, it);
            switch (state)
            {
                case xjs_parser_extract_number_literal_value_state_integer:
                    if (('0' <= ch) && (ch <= '9'))
                    { value = value * radix + (double)(ch - '0'); }
                    else if (('a' <= ch) && (ch <= 'z'))
                    { value = value * radix + (double)(ch - 'a') + 10; }
                    else if (('A' <= ch) && (ch <= 'Z'))
                    { value = value * radix + (double)(ch - 'A') + 10; }
                    else if (ch == '.')
                    { base = 10; state = xjs_parser_extract_number_literal_value_state_dot; }
                    else
                    { goto fail; }
                    break;

                case xjs_parser_extract_number_literal_value_state_dot:
                    if (('0' <= ch) && (ch <= '9'))
                    { value += (double)(ch - '0') / base; base *= 10; }
                    else
                    { goto fail; }
                    break;

                case xjs_parser_extract_number_literal_value_state_fractal:
                    if (('0' <= ch) && (ch <= '9'))
                    { value += (double)(ch - '0') / base; base *= 10; }
                    else
                    { goto fail; }
                    break;
            }
        }
    }

    *value_out = value;

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

/* Parsing Routines */

DEF_PARSE_ROUTINE(identifier, xjs_ast_identifier_ref);
DEF_PARSE_ROUTINE_ARG(statementlistitem_declaration, xjs_ast_statementlistitem_ref, \
        xjs_parser_keyword_t kw);
DEF_PARSE_ROUTINE(newexpression, xjs_ast_expression_ref);
DEF_PARSE_ROUTINE(functionexpression, xjs_ast_expression_ref);
DEF_PARSE_ROUTINE(arrowfunctionexpression, xjs_ast_expression_ref);
DEF_PARSE_ROUTINE(object_initializer, xjs_ast_expression_ref);
DEF_PARSE_ROUTINE(array_initializer, xjs_ast_expression_ref);
DEF_PARSE_ROUTINE(primaryexpression, xjs_ast_expression_ref);
DEF_PARSE_ROUTINE_ARG(callexpression, xjs_ast_expression_ref, \
        xjs_ast_expression_ref callee);
DEF_PARSE_ROUTINE(lhsexpression_allowcall, xjs_ast_expression_ref);
DEF_PARSE_ROUTINE(lhsexpression, xjs_ast_expression_ref);
DEF_PARSE_ROUTINE(updateexpression, xjs_ast_expression_ref);
DEF_PARSE_ROUTINE(unaryexpression, xjs_ast_expression_ref);
DEF_PARSE_ROUTINE(exponentiationexpression, xjs_ast_expression_ref);
DEF_PARSE_ROUTINE_ARG(binaryexpression1, xjs_ast_expression_ref, int op);
DEF_PARSE_ROUTINE(binaryexpression, xjs_ast_expression_ref);
DEF_PARSE_ROUTINE(assignmentexpression, xjs_ast_expression_ref);
DEF_PARSE_ROUTINE(expression, xjs_ast_expression_ref);
DEF_PARSE_ROUTINE(empty_statement, xjs_ast_statementlistitem_ref);
DEF_PARSE_ROUTINE(block_statement, xjs_ast_statementlistitem_ref);
DEF_PARSE_ROUTINE(expression_statement, xjs_ast_statementlistitem_ref);
DEF_PARSE_ROUTINE(returnstatement, xjs_ast_statementlistitem_ref);
DEF_PARSE_ROUTINE(breakstatement, xjs_ast_statementlistitem_ref);
DEF_PARSE_ROUTINE(ifstatement, xjs_ast_statementlistitem_ref);
DEF_PARSE_ROUTINE(statement, xjs_ast_statementlistitem_ref);
DEF_PARSE_ROUTINE(statementlistitem, xjs_ast_statementlistitem_ref);
DEF_PARSE_ROUTINE(importdeclaration, xjs_ast_moduleitem_ref);
DEF_PARSE_ROUTINE(exportdeclaration, xjs_ast_moduleitem_ref);
DEF_PARSE_ROUTINE(moduleitem, xjs_ast_moduleitem_ref);
DEF_PARSE_ROUTINE(script, xjs_ast_program_ref);
DEF_PARSE_ROUTINE(module, xjs_ast_program_ref);


DEF_PARSE_ROUTINE(identifier, xjs_ast_identifier_ref)
{
    xjs_token_ref token;
    xjs_ast_identifier_ref new_identifier = NULL;

    token = PARSE_PEEK();
    if (xjs_token_type_get(token) != XJS_TOKEN_TYPE_IDENTIFIER)
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(token, "identifier");
    }

    XJS_VNZ_ERROR_MEM(new_identifier = xjs_ast_identifier_new( \
                ec_string_clone(xjs_token_value_get(token))), ctx->err);
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN(new_identifier, token);

    PARSE_SKIP();

fail:
    return new_identifier;
}

/*
DEF_PARSE_ROUTINE(variabledeclaratorlist, xjs_ast_variabledeclaratorlist_ref)
{
    xjs_token_ref token;
    xjs_token_type token_type;
    xjs_ast_variabledeclaration_ref vardecl_ref;
    xjs_ast_variabledeclarator_ref new_vardecl = NULL;
    xjs_ast_variabledeclaratorlist_ref new_vardecl_list = NULL;

    XJS_VNZ_ERROR_MEM(new_vardecl_list = xjs_ast_variabledeclaratorlist_new(), PARSE_ERR);

    goto done;
fail:
    ec_delete(new_vardecl_list);
    new_vardecl_list = NULL;
done:
    return new_vardecl_list;
}
*/

DEF_PARSE_ROUTINE_ARG(statementlistitem_declaration, xjs_ast_statementlistitem_ref, \
        xjs_parser_keyword_t kw)
{
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_token_ref token, token_loc_start;
    xjs_token_type token_type;
    xjs_ast_variabledeclaration_ref vardecl_ref;
    xjs_ast_variabledeclarator_ref new_vardecl = NULL;

    /* New declaration */
    XJS_VNZ_ERROR_MEM(new_item = xjs_ast_statementlistitem_declaration_new(), PARSE_ERR);
    {
        xjs_ast_declaration_ref new_decl = NULL;
        XJS_VNZ_ERROR_MEM(new_decl = xjs_ast_declaration_variabledeclaration_new(), PARSE_ERR);
        xjs_ast_statementlistitem_declaration_declaration_set(new_item, new_decl);
    }
    /* Variable declaration */
    vardecl_ref = new_item->u.xjs_ast_statementlistitem_declaration->u.xjs_ast_declaration_variabledeclaration;

    /* Skip 'var', 'const' or 'let' */
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(vardecl_ref, PARSE_PEEK());
    PARSE_SKIP();

    /* Expect identifier (TODO: Optimized in the future) */
    token = PARSE_PEEK();
    if ((token_type = xjs_token_type_get(token)) != XJS_TOKEN_TYPE_IDENTIFIER)
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(token, "identifier");
    }

    for (;;)
    {
        /* Add variable */
        {
            xjs_ast_identifier_ref identifier = NULL;
            xjs_ast_variabledeclarator_id_ref vardecl_id = NULL;

            token_loc_start = PARSE_PEEK();

            if ((identifier = PARSE(identifier)) == NULL)
            { goto fail; }

            XJS_VNZ_ERROR_MEM_OR(vardecl_id \
                    = xjs_ast_variabledeclarator_id_identifier_new(identifier), PARSE_ERR, \
                    ec_delete(identifier));
            identifier = NULL;
            XJS_VNZ_ERROR_MEM(new_vardecl = xjs_ast_variabledeclarator_new(), PARSE_ERR);
            xjs_ast_variabledeclarator_id_set(new_vardecl, vardecl_id);
            vardecl_id = NULL;
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_vardecl, token_loc_start);

            /* Initialize Expression */
            if (xjs_parser_match_punctuator(PARSE_PEEK(), "="))
            {
                xjs_ast_expression_ref new_expr_init = NULL;

                /* Skip '=' */
                PARSE_SKIP();

                if ((new_expr_init = PARSE(expression)) == NULL) { goto fail; }
                xjs_ast_variabledeclarator_init_set(new_vardecl, new_expr_init);
            }

            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_vardecl, PARSE_PEEK_PREV());
            xjs_ast_variabledeclaratorlist_push_back(vardecl_ref->declarations, new_vardecl);
            new_vardecl = NULL;
        }

        token = PARSE_PEEK();
        token_type = xjs_token_type_get(token);
        if ((token_type == XJS_TOKEN_TYPE_PUNCTUATOR) && \
                xjs_aux_string_match_with(xjs_token_value_get(token), ";"))
        { PARSE_SKIP(); break; }
        else if ((token_type == XJS_TOKEN_TYPE_PUNCTUATOR) && \
                xjs_aux_string_match_with(xjs_token_value_get(token), ","))
        {
            PARSE_SKIP();
            token = PARSE_PEEK();
            if ((token_type = xjs_token_type_get(token)) != XJS_TOKEN_TYPE_IDENTIFIER)
            {
                PARSE_ERROR_PRINTF_EXPECTED_BEFORE(token, "identifier");
            }
        }
        else
        {
            PARSE_ERROR_PRINTF_EXPECTED_BEFORE(token, "';' or '='");
        }
    }

    {
        /* Kind */
        if (kw == xjs_parser_keyword_var)
        { xjs_ast_variabledeclaration_kind_set(vardecl_ref, xjs_ast_variabledeclaration_kind_var); }
        else if (kw == xjs_parser_keyword_const)
        { xjs_ast_variabledeclaration_kind_set(vardecl_ref, xjs_ast_variabledeclaration_kind_const); }
        else if (kw == xjs_parser_keyword_let)
        { xjs_ast_variabledeclaration_kind_set(vardecl_ref, xjs_ast_variabledeclaration_kind_let); }
        else { /* Impossible */ }
    }

    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(vardecl_ref, PARSE_PEEK_PREV());

    goto done;
fail:
    if (new_item != NULL)
    { ec_delete(new_item); new_item = NULL; }
done:
    ec_delete(new_vardecl);
    return new_item;
}

DEF_PARSE_ROUTINE(newexpression, xjs_ast_expression_ref)
{
    xjs_ast_newexpression_ref new_newexpr = NULL;
    xjs_ast_expression_ref new_expr = NULL;
    xjs_ast_expression_ref callee = NULL;
    xjs_ast_expressionlist_ref arguments = NULL;
    xjs_token_ref token_loc_start;

    token_loc_start = PARSE_PEEK();

    /* Skip 'new' */
    PARSE_SKIP();

    if ((callee = PARSE(lhsexpression)) == NULL) { goto fail; }

    XJS_VNZ_ERROR_MEM(arguments = ect_list_new(xjs_ast_expressionlist), ctx->err);
    if (xjs_parser_match_punctuator(PARSE_PEEK(), "("))
    {
        /* Skip '(' */
        PARSE_SKIP();
        while (!PARSE_EOF() && !xjs_parser_match_punctuator(PARSE_PEEK(), ")"))
        {
            xjs_ast_expression_ref new_arg = NULL;
            if ((new_arg = PARSE(expression)) == NULL) { goto fail; }
            ect_list_push_back(xjs_ast_expressionlist, arguments, new_arg);

            /* Skip ',' */
            if (xjs_parser_match_punctuator(PARSE_PEEK(), ","))
            {
                PARSE_SKIP();
                if ((!xjs_parser_match_punctuator(PARSE_PEEK(), ")") && \
                            (!(xjs_token_type_get(PARSE_PEEK()) == XJS_TOKEN_TYPE_IDENTIFIER))))
                {
                    PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "identifier");
                }
            }
        }
        /* Skip ')' */
        PARSE_SKIP();
    }

    XJS_VNZ_ERROR_MEM(new_newexpr = xjs_ast_newexpression_new(), ctx->err);
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_newexpr, token_loc_start);
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_newexpr, PARSE_PEEK_PREV());
    xjs_ast_newexpression_callee_set(new_newexpr, callee);
    xjs_ast_newexpression_arguments_set(new_newexpr, arguments);
    arguments = NULL;
    XJS_VNZ_ERROR_MEM(new_expr = xjs_ast_expression_newexpression_new(), ctx->err);
    xjs_ast_expression_newexpression_set(new_expr, new_newexpr);
    new_newexpr = NULL;

    goto done;
fail:
    if (new_expr != NULL)
    {
        ec_delete(new_expr);
        new_expr = NULL;
    }
done:
    ec_delete(arguments);
    ec_delete(new_newexpr);
    return new_expr;
}

DEF_PARSE_ROUTINE(functionexpression, xjs_ast_expression_ref)
{
    xjs_ast_expression_ref new_expr = NULL;
    ec_string *id = NULL;
    xjs_ast_functionexpression_ref new_functionexpression = NULL;
    xjs_token_ref token_loc_start;

    token_loc_start = PARSE_PEEK();

    /* Skip 'function' */
    PARSE_SKIP();

    XJS_VNZ_ERROR_MEM(new_functionexpression = xjs_ast_functionexpression_new(), ctx->err);

    /* Function with ID */
    if (xjs_token_type_get(PARSE_PEEK()) == XJS_TOKEN_TYPE_IDENTIFIER)
    {
        id = xjs_token_value_get(PARSE_PEEK());
        XJS_VNZ_ERROR_MEM(new_functionexpression->id = ec_string_clone(id), ctx->err);
        PARSE_SKIP();
    }

    /* Parameter List */
    if (!xjs_parser_match_punctuator(PARSE_PEEK(), "("))
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "'('");
    }
    PARSE_SKIP();
    for (;;)
    {
        if (xjs_token_type_get(PARSE_PEEK()) == XJS_TOKEN_TYPE_IDENTIFIER)
        {
            id = xjs_token_value_get(PARSE_PEEK());
            {
                xjs_ast_identifier_ref new_id;
                XJS_VNZ_ERROR_MEM(new_id = xjs_ast_identifier_new(ec_string_clone(id)), ctx->err);
                PARSE_LOC_RANGE_CLONE_FROM_TOKEN(new_id, PARSE_PEEK());
                XJS_VEZ_ERROR_MEM(xjs_ast_functionexpression_parameter_push_back( \
                            new_functionexpression, new_id), ctx->err);
            }
            PARSE_SKIP();
            if (xjs_parser_match_punctuator(PARSE_PEEK(), ")"))
            { PARSE_SKIP(); break; }
            else if (xjs_parser_match_punctuator(PARSE_PEEK(), ","))
            { PARSE_SKIP(); }
            else
            {
                PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "',' or ')'");
            }
        }
        else if (xjs_parser_match_punctuator(PARSE_PEEK(), ")"))
        { PARSE_SKIP(); break; }
        else
        {
            PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "identifier or ')'");
        }
    }

    /* Body */

    if (!xjs_parser_match_punctuator(PARSE_PEEK(), "{"))
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "'{'");
    }

    {
        xjs_ast_statementlistitem_ref new_item;

        if ((new_item = PARSE(statementlistitem)) == NULL)
        { goto fail; }

        xjs_ast_functionexpression_body_set(new_functionexpression, new_item);
    }

    XJS_VNZ_ERROR_MEM(new_expr = xjs_ast_expression_functionexpression_new(), ctx->err);
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_functionexpression, token_loc_start);
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_functionexpression, PARSE_PEEK_PREV());
    xjs_ast_expression_functionexpression_set(new_expr, new_functionexpression);
    new_functionexpression = NULL;

    goto done;
fail:
    if (new_expr != NULL)
    {
        ec_delete(new_expr);
        new_expr = NULL;
    }
done:
    ec_delete(new_functionexpression);
    return new_expr;
}

DEF_PARSE_ROUTINE(arrowfunctionexpression, xjs_ast_expression_ref)
{
    xjs_ast_expression_ref new_expr = NULL;
    xjs_bool multi_params = xjs_true;
    xjs_ast_arrowfunctionexpression_ref arrowfunc = NULL;
    xjs_token_ref token_loc_start;

    token_loc_start = PARSE_PEEK();

    XJS_VNZ_ERROR_MEM(arrowfunc = xjs_ast_arrowfunctionexpression_new(), ctx->err);

    if (xjs_parser_match_punctuator(PARSE_PEEK(), "("))
    {
        multi_params = xjs_true;
        PARSE_SKIP();
    }
    else if (xjs_token_type_get(PARSE_PEEK()) == XJS_TOKEN_TYPE_IDENTIFIER)
    {
        multi_params = xjs_false;
        xjs_ast_identifier_ref id;
        XJS_VNZ_ERROR_MEM(id = xjs_ast_identifier_new(ec_string_clone(xjs_token_value_get(PARSE_PEEK()))), ctx->err);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN(id, PARSE_PEEK());
        xjs_ast_arrowfunctionexpression_parameter_push_back(arrowfunc, id);
        PARSE_SKIP();
    }
    else
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "identifier or '('");
    }

    if (multi_params == xjs_true)
    {
        for (;;)
        {
            if (xjs_token_type_get(PARSE_PEEK()) == XJS_TOKEN_TYPE_IDENTIFIER)
            {
                xjs_ast_identifier_ref id;
                XJS_VNZ_ERROR_MEM(id = xjs_ast_identifier_new(ec_string_clone(xjs_token_value_get(PARSE_PEEK()))), ctx->err);
                PARSE_LOC_RANGE_CLONE_FROM_TOKEN(id, PARSE_PEEK());
                xjs_ast_arrowfunctionexpression_parameter_push_back(arrowfunc, id);
                PARSE_SKIP();
            }
            else if (xjs_parser_match_punctuator(PARSE_PEEK(), ")"))
            { PARSE_SKIP(); break; }
            else
            { PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "identifier or ')'"); }

            if (xjs_parser_match_punctuator(PARSE_PEEK(), ","))
            { PARSE_SKIP(); }
            else if (xjs_parser_match_punctuator(PARSE_PEEK(), ")"))
            { PARSE_SKIP(); break; }
            else
            { PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "',' or ')'"); }
        }
    }

    /* => */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), "=>")) { PARSE_SKIP(); }
    else { PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "'=>'"); }

    if (xjs_parser_match_punctuator(PARSE_PEEK(), "{"))
    {
        PARSE_SKIP();
        /* block statements */
        xjs_ast_statementlistitem_ref blockstmt;

        if ((blockstmt = PARSE(block_statement)) == NULL) { goto fail; }

        xjs_ast_arrowfunctionexpression_body_statement_set(arrowfunc, blockstmt);
    }
    else
    {
        /* expression */
        xjs_ast_expression_ref expr;

        if ((expr = PARSE(expression)) == NULL) { goto fail; }

        xjs_ast_arrowfunctionexpression_body_expr_set(arrowfunc, expr);
    }

    XJS_VNZ_ERROR_MEM(new_expr = xjs_ast_expression_arrowfunctionexpression_new(), ctx->err);
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(arrowfunc, token_loc_start);
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(arrowfunc, PARSE_PEEK_PREV());
    xjs_ast_expression_arrowfunctionexpression_set(new_expr, arrowfunc);
    arrowfunc = NULL;

    goto done;
fail:
    ec_delete(arrowfunc);
done:
    return new_expr;
}

DEF_PARSE_ROUTINE(property, xjs_ast_property_ref)
{
    xjs_ast_property_ref new_property = NULL;
    xjs_ast_expression_ref key = NULL;
    xjs_ast_expression_ref value = NULL;

    XJS_VNZ_ERROR_MEM(new_property = xjs_ast_property_new(), ctx->err);

    /* key */
    if (xjs_token_type_get(PARSE_PEEK()) == XJS_TOKEN_TYPE_IDENTIFIER)
    {
        xjs_ast_literal_ref new_lit;

        XJS_VNZ_ERROR_MEM(new_lit = xjs_ast_literal_new(), PARSE_ERR);
        {
            ec_string *s;
            s = ec_string_clone(xjs_token_value_get(PARSE_PEEK()));
            ec_string_push_front(s, '\"');
            ec_string_push_back(s, '\"');
            xjs_ast_literal_raw_set(new_lit, s);
        }
        xjs_ast_literal_value_set_string(new_lit, ec_string_clone(xjs_token_value_get(PARSE_PEEK())));

        XJS_VNZ_ERROR_MEM(key = xjs_ast_expression_literal_new(), PARSE_ERR);
        xjs_ast_expression_literal_set(key, new_lit); new_lit = NULL;
        xjs_ast_property_set_key(new_property, key);

        PARSE_SKIP();
    }
    else
    {
        if ((key = PARSE(expression)) == NULL) { goto fail; }
        xjs_ast_property_set_key(new_property, key);
    }
    key = NULL;

    /* : */
    if (!xjs_parser_match_punctuator(PARSE_PEEK(), ":"))
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "':'");
    }
    PARSE_SKIP();

    /* value */
    if ((value = PARSE(expression)) == NULL) { goto fail; }
    xjs_ast_property_set_value(new_property, value);
    value = NULL;

    goto done;
fail:
    if (new_property != NULL)
    {
        ec_delete(new_property);
        new_property = NULL;
    }
done:
    ec_delete(key);
    ec_delete(value);
    return new_property;
}

DEF_PARSE_ROUTINE(object_initializer, xjs_ast_expression_ref)
{
    xjs_ast_expression_ref new_expr = NULL;
    xjs_ast_objectexpression_ref new_objectexpression = NULL;

    /* Skip '{' */
    PARSE_SKIP();

    XJS_VNZ_ERROR_MEM(new_objectexpression = xjs_ast_objectexpression_new(), ctx->err);

    while (!xjs_parser_match_punctuator(PARSE_PEEK(), "}"))
    {
        xjs_ast_property_ref new_property = NULL;

        if ((new_property = PARSE(property)) == NULL)
        { goto fail; }

        xjs_ast_objectexpression_property_push_back(new_objectexpression, new_property);
        new_property = NULL;

        if (xjs_parser_match_punctuator(PARSE_PEEK(), ","))
        {
            /* Skip ',' */
            PARSE_SKIP();
        }
    }

    /* Skip '}' */
    PARSE_SKIP();

    XJS_VNZ_ERROR_MEM(new_expr = xjs_ast_expression_objectexpression_new(), ctx->err);
    xjs_ast_expression_objectexpression_set(new_expr, new_objectexpression);
    new_objectexpression = NULL;

    goto done;
fail:
    if (new_expr != NULL)
    {
        ec_delete(new_expr);
        new_expr = NULL;
    }
done:
    ec_delete(new_objectexpression);
    return new_expr;
}

DEF_PARSE_ROUTINE(array_initializer, xjs_ast_expression_ref)
{
    xjs_ast_expression_ref new_expr = NULL;
    xjs_ast_arrayexpression_ref new_arrayexpression = NULL;
    xjs_token_ref token_loc_start;

    token_loc_start = PARSE_PEEK();

    /* Skip '[' */
    PARSE_SKIP();

    XJS_VNZ_ERROR_MEM(new_arrayexpression = xjs_ast_arrayexpression_new(), ctx->err);

    while (!xjs_parser_match_punctuator(PARSE_PEEK(), "]"))
    {
        xjs_ast_expression_ref new_element = NULL;

        if ((new_element = PARSE(expression)) == NULL)
        { goto fail; }

        xjs_ast_arrayexpression_element_push_back(new_arrayexpression, new_element);
        new_element = NULL;

        if (xjs_parser_match_punctuator(PARSE_PEEK(), ","))
        {
            /* Skip ',' */
            PARSE_SKIP();
        }
    }

    XJS_VNZ_ERROR_MEM(new_expr = xjs_ast_expression_arrayexpression_new(), ctx->err);
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_arrayexpression, token_loc_start);
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_arrayexpression, PARSE_PEEK());
    xjs_ast_expression_arrayexpression_set(new_expr, new_arrayexpression);
    new_arrayexpression = NULL;

    /* Skip ']' */
    PARSE_SKIP();

    goto done;
fail:
    if (new_expr != NULL)
    {
        ec_delete(new_expr);
        new_expr = NULL;
    }
done:
    ec_delete(new_arrayexpression);
    return new_expr;
}

static xjs_bool at_beginning_of_arrowfunctionexpr(xjs_parser_ctx *ctx)
{
    xjs_bool ret = xjs_true;
    xjs_bool multi_params = xjs_true;

    /* <current position> */

    xjs_parser_snapshot_save(ctx);

    if (xjs_parser_match_punctuator(PARSE_PEEK(), "("))
    {
        multi_params = xjs_true;
        PARSE_SKIP();
    }
    else if (xjs_token_type_get(PARSE_PEEK()) == XJS_TOKEN_TYPE_IDENTIFIER)
    {
        multi_params = xjs_false;
        PARSE_SKIP();
    }
    else
    {
        ret = xjs_false;
        goto finish;
    }

    if (multi_params == xjs_true)
    {
        for (;;)
        {
            if (xjs_token_type_get(PARSE_PEEK()) == XJS_TOKEN_TYPE_IDENTIFIER)
            { PARSE_SKIP(); }
            else if (xjs_parser_match_punctuator(PARSE_PEEK(), ")"))
            { PARSE_SKIP(); break; }
            else { ret = xjs_false; goto finish; }

            if (xjs_parser_match_punctuator(PARSE_PEEK(), ","))
            { PARSE_SKIP(); }
            else if (xjs_parser_match_punctuator(PARSE_PEEK(), ")"))
            { PARSE_SKIP(); break; }
            else { ret = xjs_false; goto finish; }
        }
    }

    /* => */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), "=>"))
    { PARSE_SKIP(); }
    else { ret = xjs_false; goto finish; }

finish:
    xjs_parser_snapshot_restore(ctx);

    return ret;
}

DEF_PARSE_ROUTINE(primaryexpression, xjs_ast_expression_ref)
{
    xjs_ast_expression_ref new_expr = NULL;
    xjs_token_ref token;
    xjs_token_type token_type;
    xjs_token_ref token_loc_start;

    token_loc_start = PARSE_PEEK();
    token = PARSE_PEEK();
    token_type = xjs_token_type_get(token);
    switch (token_type)
    {
        case XJS_TOKEN_TYPE_COMMENT:
            XJS_ERROR_INTERNAL(ctx->err);
            break;

        case XJS_TOKEN_TYPE_NULL_LITERAL:
        case XJS_TOKEN_TYPE_BOOLEAN_LITERAL:
        case XJS_TOKEN_TYPE_NUMERIC_LITERAL:
        case XJS_TOKEN_TYPE_STRING_LITERAL:
            {
                xjs_ast_literal_ref new_lit;

                /* Skip the literal */
                PARSE_SKIP();
                {
                    XJS_VNZ_ERROR_MEM(new_lit = xjs_ast_literal_new(), PARSE_ERR);
                    xjs_ast_literal_raw_set(new_lit, ec_string_clone(xjs_token_value_get(token)));
                    if (token_type == XJS_TOKEN_TYPE_NULL_LITERAL)
                    {
                        xjs_ast_literal_value_set_null(new_lit);
                    }
                    else if (token_type == XJS_TOKEN_TYPE_BOOLEAN_LITERAL)
                    {
                        if (ec_string_match_c_str(xjs_token_value_get(token), "false") == ec_true)
                        { xjs_ast_literal_value_set_boolean(new_lit, ec_false); }
                        else if (ec_string_match_c_str(xjs_token_value_get(token), "true") == ec_true)
                        { xjs_ast_literal_value_set_boolean(new_lit, ec_true); }
                        else
                        { XJS_ERROR_INTERNAL(ctx->err); }
                    }
                    else if (token_type == XJS_TOKEN_TYPE_NUMERIC_LITERAL)
                    {
                        double number;
                        if (xjs_parser_extract_number_literal_value( \
                                    &number, xjs_token_value_get(token)) != 0)
                        { XJS_ERROR_INTERNAL(ctx->err); }
                        xjs_ast_literal_value_set_number(new_lit, number);
                    }
                    else if (token_type == XJS_TOKEN_TYPE_STRING_LITERAL)
                    {
                        ec_string *string_literal_value = NULL;
                        if ((string_literal_value = xjs_parser_extract_string_literal_value( \
                                        xjs_token_value_get(token))) == NULL)
                        { XJS_ERROR_INTERNAL(ctx->err); }
                        xjs_ast_literal_value_set_string(new_lit, string_literal_value);
                    }
                    else
                    { XJS_ERROR_INTERNAL(ctx->err); }
                }
                PARSE_LOC_RANGE_CLONE_FROM_TOKEN(new_lit, token);
                XJS_VNZ_ERROR_MEM_OR(new_expr = xjs_ast_expression_literal_new(), PARSE_ERR, ec_delete(new_lit));
                xjs_ast_expression_literal_set(new_expr, new_lit); new_lit = NULL;
            }
            break;

        case XJS_TOKEN_TYPE_IDENTIFIER:
            if (at_beginning_of_arrowfunctionexpr(PARSE_CTX))
            {
                if ((new_expr = PARSE(arrowfunctionexpression)) == NULL) { goto fail; }
            }
            else
            {
                ec_string *name;
                xjs_ast_identifier_ref new_id;

                /* Skip the literal */
                PARSE_SKIP();

                XJS_VNZ_ERROR_MEM(name = ec_string_clone(xjs_token_value_get(token)), PARSE_ERR);
                XJS_VNZ_ERROR_MEM_OR(new_id = xjs_ast_identifier_new(name), PARSE_ERR, ec_delete(name)); name = NULL;
                PARSE_LOC_RANGE_CLONE_FROM_TOKEN(new_id, token_loc_start);
                XJS_VNZ_ERROR_MEM_OR(new_expr = xjs_ast_expression_identifier_new(), PARSE_ERR, ec_delete(new_id));
                xjs_ast_expression_identifier_set(new_expr, new_id); new_id = NULL;
            }
            break;

        case XJS_TOKEN_TYPE_EOF:
        case XJS_TOKEN_TYPE_UNKNOWN:
            PARSE_ERROR_PRINTF_EXPECTED_BEFORE(token, "expression");
            goto fail;

        case XJS_TOKEN_TYPE_KEYWORD:
            if (xjs_parser_match_keyword(token, xjs_parser_keyword_function))
            {
                if ((new_expr = PARSE(functionexpression)) == NULL) { goto fail; }
            }
            else if (xjs_parser_match_keyword(token, xjs_parser_keyword_this))
            {
                xjs_ast_thisexpression_ref this_expr;
                XJS_VNZ_ERROR_MEM(this_expr = xjs_ast_thisexpression_new(), ctx->err);
                XJS_VNZ_ERROR_MEM(new_expr = xjs_ast_expression_thisexpression_new(), ctx->err);
                xjs_ast_expression_thisexpression_set(new_expr, this_expr);
                this_expr = NULL;
                PARSE_LOC_RANGE_CLONE_FROM_TOKEN(new_expr->u.xjs_ast_expression_thisexpression, PARSE_PEEK());
                PARSE_SKIP();
            }
            else
            {
                PARSE_ERROR_PRINTF_UNEXPECTED_TOKEN(token);
            }
            break;

        case XJS_TOKEN_TYPE_PUNCTUATOR:
            if (xjs_parser_match_punctuator(token, "{"))
            {
                if ((new_expr = PARSE(object_initializer)) == NULL) { goto fail; }
            }
            else if (xjs_parser_match_punctuator(token, "["))
            {
                if ((new_expr = PARSE(array_initializer)) == NULL) { goto fail; }
            }
            else if (xjs_parser_match_punctuator(token, "("))
            {
                if (at_beginning_of_arrowfunctionexpr(PARSE_CTX))
                {
                    if ((new_expr = PARSE(arrowfunctionexpression)) == NULL) { goto fail; }
                }
                else
                {
                    PARSE_SKIP();
                
                    /* Group expression */
                    if ((new_expr = PARSE(expression)) == NULL) { goto fail; }
                    if (!xjs_parser_match_punctuator(PARSE_PEEK(), ")"))
                    { PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "')'"); }
                    PARSE_SKIP();
                }
            }
            else
            { PARSE_ERROR_PRINTF_UNEXPECTED_TOKEN(token); }
            break;
    }

    goto done;
fail:
    if (new_expr != NULL) { ec_delete(new_expr); new_expr = NULL; }
done:
    return new_expr;
}

DEF_PARSE_ROUTINE(arguments, xjs_ast_expressionlist_ref)
{
    xjs_ast_expressionlist_ref arguments = NULL;

    XJS_VNZ_ERROR_MEM(arguments = ect_list_new(xjs_ast_expressionlist), PARSE_ERR);

    if (xjs_parser_match_punctuator(PARSE_PEEK(), ")") == ec_false)
    {
        for (;;)
        {
            xjs_ast_expression_ref new_argument = NULL;
            if ((new_argument = PARSE(expression)) == NULL) { goto fail; }
            ect_list_push_back(xjs_ast_expressionlist, arguments, new_argument);

            if (xjs_parser_match_punctuator(PARSE_PEEK(), ")") == ec_true)
            { break; }
            else if (xjs_parser_match_punctuator(PARSE_PEEK(), ",") == ec_true)
            { PARSE_SKIP(); }
            else
            {
                PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "',' or ')'");
            }
        }
    }
    
    goto done;
fail:
    if (arguments != NULL) { ec_delete(arguments); arguments = NULL; }
done:
    return arguments;
}

DEF_PARSE_ROUTINE_ARG(callexpression, xjs_ast_expression_ref, \
        xjs_ast_expression_ref callee)
{
    xjs_ast_callexpression_ref new_callexpr = NULL;
    xjs_ast_expression_ref new_expr = NULL;
    xjs_ast_expressionlist_ref arguments = NULL;

    /* Skip '(' */
    PARSE_SKIP();

    /* Arguments */
    if ((arguments = PARSE(arguments)) == NULL) { goto fail; }

    /* Skip ')' */
    PARSE_SKIP();

    XJS_VNZ_ERROR_MEM(new_callexpr = xjs_ast_callexpression_new(), ctx->err);
    xjs_ast_callexpression_callee_set(new_callexpr, callee);
    xjs_ast_callexpression_arguments_set(new_callexpr, arguments);
    arguments = NULL;
    XJS_VNZ_ERROR_MEM(new_expr = xjs_ast_expression_callexpression_new(), ctx->err);
    /* Delay loc start */
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_callexpr, PARSE_PEEK_PREV());
    xjs_ast_expression_callexpression_set(new_expr, new_callexpr);
    new_callexpr = NULL;

    goto done;
fail:
    if (new_expr != NULL) { ec_delete(new_expr); new_expr = NULL; }
done:
    ec_delete(new_callexpr);
    ec_delete(arguments);
    return new_expr;
}

DEF_PARSE_ROUTINE_ARG(lhsexpression_opt, xjs_ast_expression_ref, \
        ec_bool allowcall)
{
    xjs_ast_expression_ref new_expr = NULL;
    xjs_token_ref token;
    xjs_token_type token_type;
    xjs_token_ref token_loc_start;

    token_loc_start = PARSE_PEEK();

    token = PARSE_PEEK();
    token_type = xjs_token_type_get(token);
    if ((token_type == XJS_TOKEN_TYPE_KEYWORD) && \
            (xjs_parser_keyword_type_resolve(token) == xjs_parser_keyword_new))
    {
        /* new */
        if ((new_expr = PARSE(newexpression)) == NULL) { goto fail; }
    }
    else
    {
        if ((new_expr = PARSE(primaryexpression)) == NULL) { goto fail; }
    }

    for (;;)
    {
        if (xjs_parser_match_punctuator(PARSE_PEEK(), "."))
        {
            /* Static member */
            PARSE_SKIP();
            {
                xjs_ast_identifier_ref property = NULL;
                xjs_ast_expression_ref member;

                if ((property = PARSE(identifier)) == NULL)
                { goto fail; }
                XJS_VNZ_ERROR_MEM_OR(member = xjs_ast_expression_identifier_new(), \
                        ctx->err, ec_delete(property));
                xjs_ast_expression_identifier_set(member, property);
                property = NULL;

                {
                    xjs_ast_memberexpression_ref new_memberexpr;
                    XJS_VNZ_ERROR_MEM_OR(new_memberexpr = xjs_ast_memberexpression_new(), \
                            ctx->err, ec_delete(property));
                    xjs_ast_memberexpression_computed_set(new_memberexpr, ec_false);
                    xjs_ast_memberexpression_object_set(new_memberexpr, new_expr);
                    xjs_ast_memberexpression_property_set(new_memberexpr, member);
                    new_expr = NULL;

                    XJS_VNZ_ERROR_MEM_OR(new_expr = xjs_ast_expression_memberexpression_new(), \
                            ctx->err, ec_delete(new_memberexpr));
                    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_memberexpr, token_loc_start);
                    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_memberexpr, PARSE_PEEK_PREV());
                    xjs_ast_expression_memberexpression_set(new_expr, new_memberexpr);
                }

            }
        }
        else if ((allowcall == ec_true) && \
                (xjs_parser_match_punctuator(PARSE_PEEK(), "(")))
        {
            /* Call expression */
            xjs_ast_expression_ref callee = new_expr; new_expr = NULL;
            if ((new_expr = PARSEA(callexpression, callee)) == NULL)
            { ec_delete(callee); goto fail; }
            /* Patch loc start */
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_expr->u.xjs_ast_expression_callexpression, token_loc_start);
        }
        else if (xjs_parser_match_punctuator(PARSE_PEEK(), "["))
        {
            /* Computed member */
            PARSE_SKIP();
            {
                xjs_ast_expression_ref member = NULL;

                if ((member = PARSE(expression)) == NULL)
                { goto fail; }

                if (!xjs_parser_match_punctuator(PARSE_PEEK(), "]"))
                {
                    ec_delete(member);
                    PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "']'");
                }

                {
                    xjs_ast_memberexpression_ref new_memberexpr;
                    XJS_VNZ_ERROR_MEM(new_memberexpr = xjs_ast_memberexpression_new(), ctx->err);
                    xjs_ast_memberexpression_computed_set(new_memberexpr, ec_true);
                    xjs_ast_memberexpression_object_set(new_memberexpr, new_expr);
                    xjs_ast_memberexpression_property_set(new_memberexpr, member);
                    new_expr = NULL;

                    XJS_VNZ_ERROR_MEM_OR(new_expr = xjs_ast_expression_memberexpression_new(), \
                            ctx->err, ec_delete(new_memberexpr));
                    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_memberexpr, token_loc_start);
                    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_memberexpr, PARSE_PEEK());
                    xjs_ast_expression_memberexpression_set(new_expr, new_memberexpr);
                }
            }
            PARSE_SKIP();
        }
        else
        {
            break;
        }
    }

    goto done;
fail:
    if (new_expr != NULL) { ec_delete(new_expr); new_expr = NULL; }
done:
    return new_expr;
}

DEF_PARSE_ROUTINE(lhsexpression_allowcall, xjs_ast_expression_ref)
{
    return PARSEA(lhsexpression_opt, ec_true);
}

DEF_PARSE_ROUTINE(lhsexpression, xjs_ast_expression_ref)
{
    return PARSEA(lhsexpression_opt, ec_false);
}

DEF_PARSE_ROUTINE(updateexpression, xjs_ast_expression_ref)
{
    xjs_ast_expression_ref new_expr = NULL;
    xjs_token_ref token;
    xjs_token_ref token_loc_start;

    token_loc_start = PARSE_PEEK();

    token = PARSE_PEEK();

#define MATCH_UPDATE_PUNCTUATOR(token) \
    ((xjs_token_type_get(token) == XJS_TOKEN_TYPE_PUNCTUATOR) && \
     ((xjs_parser_match_punctuator(token, "++") == ec_true) || \
      (xjs_parser_match_punctuator(token, "--") == ec_true)) ? ec_true : ec_false)

    if (MATCH_UPDATE_PUNCTUATOR(token) == ec_true)
    {
        PARSE_SKIP();
        {
            xjs_ast_updateexpression_ref new_updateexpression = NULL;

            XJS_VNZ_ERROR_MEM(new_updateexpression = xjs_ast_updateexpression_new(), ctx->err);
            {
                xjs_ast_updateexpression_op_set(new_updateexpression, ec_string_clone(xjs_token_value_get(token)));
            }
            {
                xjs_ast_expression_ref new_expr_sub = NULL;
                if ((new_expr_sub = PARSE(updateexpression)) == NULL) { ec_delete(new_updateexpression); goto fail; }
                xjs_ast_updateexpression_argument_set(new_updateexpression, new_expr_sub); new_expr_sub = NULL;
            }
            {
                xjs_ast_updateexpression_prefix_set(new_updateexpression, ec_true);
            }

            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_updateexpression, token_loc_start);
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_updateexpression, PARSE_PEEK());
            XJS_VNZ_ERROR_MEM_OR(new_expr = xjs_ast_expression_updateexpression_new(), ctx->err, ec_delete(new_updateexpression));
            xjs_ast_expression_updateexpression_set(new_expr, new_updateexpression); new_updateexpression = NULL;
        }
    }
    else
    {
        if ((new_expr = PARSE(lhsexpression_allowcall)) == NULL) { goto fail; }

        if (MATCH_UPDATE_PUNCTUATOR(PARSE_PEEK()) == ec_true)
        {
            xjs_ast_updateexpression_ref new_updateexpression = NULL;

            XJS_VNZ_ERROR_MEM(new_updateexpression = xjs_ast_updateexpression_new(), ctx->err);
            {
                xjs_ast_updateexpression_op_set(new_updateexpression, ec_string_clone(xjs_token_value_get(PARSE_PEEK())));
            }
            {
                xjs_ast_updateexpression_argument_set(new_updateexpression, new_expr); new_expr = NULL;
            }
            {
                xjs_ast_updateexpression_prefix_set(new_updateexpression, ec_false);
            }

            XJS_VNZ_ERROR_MEM_OR(new_expr = xjs_ast_expression_updateexpression_new(), ctx->err, ec_delete(new_updateexpression));
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_updateexpression, token_loc_start);
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_updateexpression, PARSE_PEEK());
            xjs_ast_expression_updateexpression_set(new_expr, new_updateexpression); new_updateexpression = NULL;

            PARSE_SKIP();
        }
    }
#undef MATCH_UPDATE_PUNCTUATOR

    goto done;
fail:
    if (new_expr != NULL) { ec_delete(new_expr); new_expr = NULL; }
done:
    return new_expr;
}

DEF_PARSE_ROUTINE(unaryexpression, xjs_ast_expression_ref)
{
    ec_bool match_unary;
    xjs_ast_expression_ref new_expr = NULL;
    xjs_parser_keyword_t kw;
    xjs_token_ref token;
    xjs_token_type token_type;
    xjs_token_ref token_loc_start;

    token_loc_start = PARSE_PEEK();

    token = PARSE_PEEK();
    token_type = xjs_token_type_get(token);

    match_unary = (token_type == XJS_TOKEN_TYPE_PUNCTUATOR) && \
                  ((xjs_parser_match_punctuator(token, "+") == ec_true) || \
                   (xjs_parser_match_punctuator(token, "-") == ec_true) || \
                   (xjs_parser_match_punctuator(token, "~") == ec_true) || \
                   (xjs_parser_match_punctuator(token, "!") == ec_true)) ? ec_true : ec_false;
    if ((match_unary == ec_false) && (token_type == XJS_TOKEN_TYPE_KEYWORD))
    {
        kw = xjs_parser_keyword_type_resolve(token);
        if ((kw == xjs_parser_keyword_delete) || \
                (kw == xjs_parser_keyword_void) || \
                (kw == xjs_parser_keyword_typeof))
        { match_unary = ec_true; }
    }

    if (match_unary == ec_true)
    {
        PARSE_SKIP();
        {
            xjs_ast_unaryexpression_ref new_unaryexpression = NULL;

            XJS_VNZ_ERROR_MEM(new_unaryexpression = xjs_ast_unaryexpression_new(), ctx->err);
            {
                xjs_ast_unaryexpression_op_set(new_unaryexpression, ec_string_clone(xjs_token_value_get(token)));
            }
            {
                xjs_ast_expression_ref new_expr_sub = NULL;
                if ((new_expr_sub = PARSE(unaryexpression)) == NULL) { ec_delete(new_unaryexpression); goto fail; }
                xjs_ast_unaryexpression_argument_set(new_unaryexpression, new_expr_sub); new_expr_sub = NULL;
            }

            XJS_VNZ_ERROR_MEM_OR(new_expr = xjs_ast_expression_unaryexpression_new(), ctx->err, ec_delete(new_unaryexpression));
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_unaryexpression, token_loc_start);
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_unaryexpression, PARSE_PEEK_PREV());
            xjs_ast_expression_unaryexpression_set(new_expr, new_unaryexpression); new_unaryexpression = NULL;
        }
    }
    else
    {
        if ((new_expr = PARSE(updateexpression)) == NULL) { goto fail; }
    }

    goto done;
fail:
    if (new_expr != NULL) { ec_delete(new_expr); new_expr = NULL; }
done:
    return new_expr;
}

DEF_PARSE_ROUTINE(exponentiationexpression, xjs_ast_expression_ref)
{
    xjs_ast_expression_ref new_expr = NULL;

    if ((new_expr = PARSE(unaryexpression)) == NULL) { goto fail; }

    /* TODO: '**' */

    goto done;
fail:
    if (new_expr != NULL) { ec_delete(new_expr); new_expr = NULL; }
done:
    return new_expr;
}

static int binary_op_precedence(xjs_token_ref token)
{
    xjs_token_type token_type = xjs_token_type_get(token);
    ec_string *value = xjs_token_value_get(token);
    ec_size_t len = (ec_size_t)ec_string_length(value);

    if (token_type == XJS_TOKEN_TYPE_PUNCTUATOR)
    {
        switch (len)
        {
            case 1:
                if (xjs_aux_string_match_with(value, "*")) return 14;
                if (xjs_aux_string_match_with(value, "/")) return 14;
                if (xjs_aux_string_match_with(value, "%")) return 14;
                if (xjs_aux_string_match_with(value, "+")) return 13;
                if (xjs_aux_string_match_with(value, "-")) return 13;
                if (xjs_aux_string_match_with(value, "<")) return 11;
                if (xjs_aux_string_match_with(value, ">")) return 11;
                if (xjs_aux_string_match_with(value, "&")) return 9;
                if (xjs_aux_string_match_with(value, "^")) return 8;
                if (xjs_aux_string_match_with(value, "|")) return 7;
                break;

            case 2:
                if (xjs_aux_string_match_with(value, "<<")) return 12;
                if (xjs_aux_string_match_with(value, ">>")) return 12;
                if (xjs_aux_string_match_with(value, "<=")) return 11;
                if (xjs_aux_string_match_with(value, ">=")) return 11;
                if (xjs_aux_string_match_with(value, "==")) return 10;
                if (xjs_aux_string_match_with(value, "!=")) return 10;
                if (xjs_aux_string_match_with(value, "&&")) return 6;
                if (xjs_aux_string_match_with(value, "||")) return 5;
                break;

            case 3:
                if (xjs_aux_string_match_with(value, ">>>")) return 12;
                if (xjs_aux_string_match_with(value, "===")) return 10;
                if (xjs_aux_string_match_with(value, "!==")) return 10;
                break;
        }
    }
    else if (token_type == XJS_TOKEN_TYPE_KEYWORD)
    {
        xjs_parser_keyword_t kw = xjs_parser_keyword_type_resolve(token);
        if (kw == xjs_parser_keyword_instanceof) return 11;
        else if (kw == xjs_parser_keyword_in) return 11;
    }

    return -1;
}

/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Operator_Precedence */
DEF_PARSE_ROUTINE_ARG(binaryexpression1, xjs_ast_expression_ref, int precedence_limit)
{
    int precedence;
    xjs_ast_expression_ref new_expr = NULL;
    xjs_ast_expression_ref new_expr_rhs = NULL;
    xjs_ast_binaryexpression_ref new_binaryexpr = NULL;
    xjs_ast_logicalexpression_ref new_logicalexpr = NULL;
    ec_string *value = NULL;
    xjs_token_ref token_loc_start = PARSE_PEEK();

    if ((new_expr = PARSE(exponentiationexpression)) == NULL) { goto fail; }

    for (;;)
    {
        /* No binary operator */
        if ((precedence = binary_op_precedence(PARSE_PEEK())) < 0) break;
        if (precedence <= precedence_limit) break;

        /* Record operator */
        XJS_VNZ_ERROR_MEM(value = ec_string_clone(xjs_token_value_get(PARSE_PEEK())), PARSE_ERR);
        PARSE_SKIP();

        if ((new_expr_rhs = PARSEA(binaryexpression1, precedence)) == NULL) { goto fail; }

        if (ec_string_match_c_str(value, "&&") || \
                ec_string_match_c_str(value, "||"))
        {
            XJS_VNZ_ERROR_MEM(new_logicalexpr = xjs_ast_logicalexpression_new(), PARSE_ERR);
            xjs_ast_logicalexpression_op_set(new_logicalexpr, value); value = NULL;
            xjs_ast_logicalexpression_left_set(new_logicalexpr, new_expr); new_expr = NULL;
            xjs_ast_logicalexpression_right_set(new_logicalexpr, new_expr_rhs); new_expr_rhs = NULL;
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_logicalexpr, token_loc_start);
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_logicalexpr, PARSE_PEEK_PREV());
            XJS_VNZ_ERROR_MEM(new_expr = xjs_ast_expression_logicalexpression_new(), PARSE_ERR);
            xjs_ast_expression_logicalexpression_set(new_expr, new_logicalexpr); new_logicalexpr = NULL;
        }
        else
        {
            XJS_VNZ_ERROR_MEM(new_binaryexpr = xjs_ast_binaryexpression_new(), PARSE_ERR);
            xjs_ast_binaryexpression_op_set(new_binaryexpr, value); value = NULL;
            xjs_ast_binaryexpression_left_set(new_binaryexpr, new_expr); new_expr = NULL;
            xjs_ast_binaryexpression_right_set(new_binaryexpr, new_expr_rhs); new_expr_rhs = NULL;
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_binaryexpr, token_loc_start);
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_binaryexpr, PARSE_PEEK_PREV());
            XJS_VNZ_ERROR_MEM(new_expr = xjs_ast_expression_binaryexpression_new(), PARSE_ERR);
            xjs_ast_expression_binaryexpression_set(new_expr, new_binaryexpr); new_binaryexpr = NULL;
        }
    }

    goto done;
fail:
    if (new_expr != NULL) { ec_delete(new_expr); new_expr = NULL; }
done:
    ec_delete(new_expr_rhs);
    ec_delete(new_binaryexpr);
    ec_delete(new_logicalexpr);
    ec_delete(value);
    return new_expr;
}

DEF_PARSE_ROUTINE(binaryexpression, xjs_ast_expression_ref)
{
    return PARSEA(binaryexpression1, 0);
}

DEF_PARSE_ROUTINE(conditionalexpression, xjs_ast_expression_ref)
{
    xjs_ast_expression_ref new_expr = NULL;
    xjs_ast_expression_ref test = NULL;
    xjs_ast_expression_ref consequent = NULL;
    xjs_ast_expression_ref alternate = NULL;
    xjs_token_ref token_loc_start;

    token_loc_start = PARSE_PEEK();

    if ((test = PARSE(binaryexpression)) == NULL) { goto fail; }
    
    if (xjs_parser_match_punctuator(PARSE_PEEK(), "?") == ec_true)
    {
        /* Skip '?' */
        PARSE_SKIP();

        if ((consequent = PARSE(assignmentexpression)) == NULL) { goto fail; }
        if (xjs_parser_match_punctuator(PARSE_PEEK(), ":") == ec_false)
        {
            PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "':'");
        }
        
        /* Skjp ':' */
        PARSE_SKIP();

        if ((alternate = PARSE(assignmentexpression)) == NULL) { goto fail; }

        {
            xjs_ast_conditionalexpression_ref conditional_expr;
            XJS_VNZ_ERROR_MEM(conditional_expr = xjs_ast_conditionalexpression_new(), ctx->err);
            xjs_ast_conditionalexpression_test_set(conditional_expr, test); test = NULL;
            xjs_ast_conditionalexpression_consequent_set(conditional_expr, consequent); consequent = NULL;
            xjs_ast_conditionalexpression_alternate_set(conditional_expr, alternate); alternate = NULL;
            XJS_VNZ_ERROR_MEM_OR(new_expr = xjs_ast_expression_conditionalexpression_new(), \
                    ctx->err, ec_delete(conditional_expr));
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(conditional_expr, token_loc_start);
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(conditional_expr, PARSE_PEEK_PREV());
            xjs_ast_expression_conditionalexpression_set(new_expr, conditional_expr);
            conditional_expr = NULL;
        }
    }
    else
    {
        new_expr = test; test = NULL;
    }

    goto done;
fail:
    if (new_expr != NULL) { ec_delete(new_expr); new_expr = NULL; }
done:
    ec_delete(test);
    ec_delete(consequent);
    ec_delete(alternate);
    return new_expr;
}

DEF_PARSE_ROUTINE(assignmentexpression, xjs_ast_expression_ref)
{
    xjs_token_ref token;
    xjs_ast_expression_ref new_expr = NULL;
    xjs_ast_expression_ref new_expr_rhs = NULL;
    xjs_ast_assignmentexpression_ref new_assignmentexpression = NULL;
    xjs_token_ref token_loc_start;

    token_loc_start = PARSE_PEEK();

    if ((new_expr = PARSE(conditionalexpression)) == NULL) { goto fail; }

    token = PARSE_PEEK();
    if (xjs_parser_match_assignment(token))
    {
        /* TODO: Assignment Expression */

        /* Skip the assignment operator */
        PARSE_SKIP();
        if ((new_expr_rhs = PARSE(assignmentexpression)) == NULL) { goto fail; }

        XJS_VNZ_ERROR_MEM(new_assignmentexpression = xjs_ast_assignmentexpression_new(), PARSE_ERR);
        xjs_ast_assignmentexpression_op_set(new_assignmentexpression, ec_string_clone(xjs_token_value_get(token)));
        xjs_ast_assignmentexpression_left_set(new_assignmentexpression, new_expr); new_expr = NULL;
        xjs_ast_assignmentexpression_right_set(new_assignmentexpression, new_expr_rhs); new_expr_rhs = NULL;
        XJS_VNZ_ERROR_MEM(new_expr = xjs_ast_expression_assignmentexpression_new(), PARSE_ERR);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_assignmentexpression, token_loc_start);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_assignmentexpression, PARSE_PEEK_PREV());
        xjs_ast_expression_assignmentexpression_set(new_expr, new_assignmentexpression); new_assignmentexpression = NULL;
    }

    goto done;
fail:
    if (new_expr != NULL) { ec_delete(new_expr); new_expr = NULL; }
    ec_delete(new_expr_rhs);
    ec_delete(new_assignmentexpression);
done:
    return new_expr;
}

DEF_PARSE_ROUTINE(expression, xjs_ast_expression_ref)
{
    xjs_ast_expression_ref new_expr = NULL;

    if ((new_expr = PARSE(assignmentexpression)) == NULL) { goto fail; }

    /* TODO: ',' SequenceExpression */

    goto done;
fail:
    if (new_expr != NULL) { ec_delete(new_expr); new_expr = NULL; }
done:
    return new_expr;
}

DEF_PARSE_ROUTINE(empty_statement, xjs_ast_statementlistitem_ref)
{
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_ast_statement_ref new_stmt = NULL;

    XJS_VNZ_ERROR_MEM(new_stmt = xjs_ast_statement_emptystatement_new(), PARSE_ERR);
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN(new_stmt->u.xjs_ast_statement_emptystatement, PARSE_PEEK());
    XJS_VNZ_ERROR_MEM(new_item = xjs_ast_statementlistitem_statement_new(), PARSE_ERR);
    xjs_ast_statementlistitem_statement_statement_set(new_item, new_stmt); new_stmt = NULL;

    /* Skip ';' */
    PARSE_SKIP();

    goto done;
fail:
done:
    ec_delete(new_stmt);
    return new_item;
}

DEF_PARSE_ROUTINE(block_statement, xjs_ast_statementlistitem_ref)
{
    xjs_token_ref token;
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_ast_statementlistitem_ref new_subitem = NULL;
    xjs_ast_statement_ref new_stmt = NULL;

    XJS_VNZ_ERROR_MEM(new_stmt = xjs_ast_statement_blockstatement_new(), PARSE_ERR);
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_stmt->u.xjs_ast_statement_blockstatement, PARSE_PEEK());

    /* Skip '{' */
    PARSE_SKIP();

    for (;;)
    {
        token = PARSE_PEEK();
        if (xjs_parser_match_punctuator(token, "}") == ec_true)
        {
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_stmt->u.xjs_ast_statement_blockstatement, PARSE_PEEK());
            PARSE_SKIP();
            break;
        }

        if ((new_subitem = PARSE(statementlistitem)) == NULL)
        { goto fail; }
        xjs_ast_statement_blockstatement_push_back(new_stmt, new_subitem); new_subitem = NULL;
    }
    XJS_VNZ_ERROR_MEM(new_item = xjs_ast_statementlistitem_statement_new(), PARSE_ERR);
    xjs_ast_statementlistitem_statement_statement_set(new_item, new_stmt); new_stmt = NULL;

    goto done;
fail:
done:
    ec_delete(new_stmt);
    ec_delete(new_subitem);
    return new_item;
}

DEF_PARSE_ROUTINE(expression_statement, xjs_ast_statementlistitem_ref)
{
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_ast_statement_ref new_stmt = NULL;
    xjs_ast_expression_ref new_expr = NULL;
    xjs_token_ref token_loc_start;

    token_loc_start = PARSE_PEEK();
    
    if ((new_expr = PARSE(expression)) == NULL) { goto fail; }

    XJS_VNZ_ERROR_MEM(new_stmt = xjs_ast_statement_expressionstatement_new(), PARSE_ERR);
    xjs_ast_statement_expressionstatement_expression_set(new_stmt, new_expr); new_expr = NULL;
    XJS_VNZ_ERROR_MEM(new_item = xjs_ast_statementlistitem_statement_new(), PARSE_ERR);
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_stmt->u.xjs_ast_statement_expressionstatement, token_loc_start);
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_stmt->u.xjs_ast_statement_expressionstatement, PARSE_PEEK());
    xjs_ast_statementlistitem_statement_statement_set(new_item, new_stmt); new_stmt = NULL;

    if (xjs_parser_match_punctuator(PARSE_PEEK(), ";"))
    { PARSE_SKIP(); }

    goto done;
fail:
    if (new_item != NULL) { ec_delete(new_item); new_item = NULL; }
done:
    ec_delete(new_stmt);
    ec_delete(new_expr);
    return new_item;
}

DEF_PARSE_ROUTINE(labeledstatement, xjs_ast_statementlistitem_ref)
{
    xjs_token_ref token;
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_ast_expression_ref new_expr = NULL;
    xjs_token_ref token_loc_start;

    token_loc_start = PARSE_PEEK();

    if ((new_expr = PARSE(expression)) == NULL) { goto fail; }

    if ((new_expr->tag == xjs_ast_expression_identifier) && \
            (token = PARSE_PEEK()) && \
            (xjs_parser_match_punctuator(token, ":")))
    {
        xjs_parser_ctx_error(PARSE_CTX, XJS_ERRNO_PARSE);
    }
    else
    {
        if (xjs_parser_match_punctuator(PARSE_PEEK(), ";"))
        { PARSE_SKIP(); }

        xjs_ast_statement_ref new_stmt = NULL;
        XJS_VNZ_ERROR_MEM(new_stmt = xjs_ast_statement_expressionstatement_new(), PARSE_ERR);
        xjs_ast_statement_expressionstatement_expression_set(new_stmt, new_expr); new_expr = NULL;
        XJS_VNZ_ERROR_MEM(new_item = xjs_ast_statementlistitem_statement_new(), PARSE_ERR);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_stmt->u.xjs_ast_statement_expressionstatement, token_loc_start);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_stmt->u.xjs_ast_statement_expressionstatement, PARSE_PEEK_PREV());
        xjs_ast_statementlistitem_statement_statement_set(new_item, new_stmt); new_stmt = NULL;
    }

    goto done;
fail:
    ec_delete(new_expr);
done:
    return new_item;
}

DEF_PARSE_ROUTINE(inspectstatement, xjs_ast_statementlistitem_ref)
{
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_ast_expression_ref new_expr = NULL;
    xjs_ast_expression_ref argument = NULL;

    /* Skip '$inspect' */
    PARSE_SKIP();

    /* Condition */
    if ((argument = PARSE(expression)) == NULL) { goto fail; }

    /* Match ';' */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), ";")) { PARSE_SKIP(); }
    else
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "';'");
    }

    {
        xjs_ast_statement_ref inspect_stmt;
        XJS_VNZ_ERROR_MEM(inspect_stmt = xjs_ast_statement_inspectstatement_new(), PARSE_ERR);
        xjs_ast_statement_inspectstatement_argument_set(inspect_stmt, argument); argument = NULL;
        XJS_VNZ_ERROR_MEM_OR(new_item = xjs_ast_statementlistitem_statement_new(), \
                PARSE_ERR, ec_delete(inspect_stmt));
        xjs_ast_statementlistitem_statement_statement_set(new_item, inspect_stmt);
    }

    goto done;
fail:
    ec_delete(new_expr);
done:
    return new_item;
}

DEF_PARSE_ROUTINE(returnstatement, xjs_ast_statementlistitem_ref)
{
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_ast_expression_ref new_expr = NULL;
    xjs_ast_expression_ref argument = NULL;
    xjs_token_ref token_return;

    token_return = PARSE_PEEK();

    /* Skip 'return' */
    PARSE_SKIP();

    /* Condition */
    if ((argument = PARSE(expression)) == NULL) { goto fail; }

    {
        xjs_ast_statement_ref return_stmt;
        XJS_VNZ_ERROR_MEM(return_stmt = xjs_ast_statement_returnstatement_new(), PARSE_ERR);
        xjs_ast_statement_returnstatement_argument_set(return_stmt, argument); argument = NULL;
        XJS_VNZ_ERROR_MEM_OR(new_item = xjs_ast_statementlistitem_statement_new(), \
                PARSE_ERR, ec_delete(return_stmt));
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(return_stmt->u.xjs_ast_statement_returnstatement, token_return);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(return_stmt->u.xjs_ast_statement_returnstatement, PARSE_PEEK());
        xjs_ast_statementlistitem_statement_statement_set(new_item, return_stmt);
    }

    /* Match ';' */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), ";")) { PARSE_SKIP(); }
    else
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "';'");
    }

    goto done;
fail:
    ec_delete(new_expr);
    ec_delete(new_item);
    new_item = NULL;
done:
    ec_delete(argument);
    return new_item;
}

DEF_PARSE_ROUTINE(breakstatement, xjs_ast_statementlistitem_ref)
{
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_token_ref token_break;

    token_break = PARSE_PEEK();

    /* Skip 'break' */
    PARSE_SKIP();

    {
        xjs_ast_statement_ref break_stmt;
        XJS_VNZ_ERROR_MEM(break_stmt = xjs_ast_statement_breakstatement_new(), PARSE_ERR);
        XJS_VNZ_ERROR_MEM_OR(new_item = xjs_ast_statementlistitem_statement_new(), \
                PARSE_ERR, ec_delete(break_stmt));
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(break_stmt->u.xjs_ast_statement_breakstatement, token_break);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(break_stmt->u.xjs_ast_statement_breakstatement, PARSE_PEEK());
        xjs_ast_statementlistitem_statement_statement_set(new_item, break_stmt);
    }

    /* Match ';' */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), ";")) { PARSE_SKIP(); }
    else
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "';'");
    }

    goto done;
fail:
    ec_delete(new_item);
    new_item = NULL;
done:
    return new_item;
}

DEF_PARSE_ROUTINE(continuestatement, xjs_ast_statementlistitem_ref)
{
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_token_ref token_continue;

    token_continue = PARSE_PEEK();

    /* Skip 'continue' */
    PARSE_SKIP();

    {
        xjs_ast_statement_ref continue_stmt;
        XJS_VNZ_ERROR_MEM(continue_stmt = xjs_ast_statement_continuestatement_new(), PARSE_ERR);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(continue_stmt->u.xjs_ast_statement_continuestatement, token_continue);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(continue_stmt->u.xjs_ast_statement_continuestatement, PARSE_PEEK());
        XJS_VNZ_ERROR_MEM_OR(new_item = xjs_ast_statementlistitem_statement_new(), \
                PARSE_ERR, ec_delete(continue_stmt));
        xjs_ast_statementlistitem_statement_statement_set(new_item, continue_stmt);
    }

    /* Match ';' */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), ";")) { PARSE_SKIP(); }
    else
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "';'");
    }

    goto done;
fail:
    ec_delete(new_item);
    new_item = NULL;
done:
    return new_item;
}

DEF_PARSE_ROUTINE(ifstatement, xjs_ast_statementlistitem_ref)
{
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_ast_expression_ref test = NULL;
    xjs_ast_statementlistitem_ref consequent = NULL;
    xjs_ast_statementlistitem_ref alternate = NULL;
    xjs_token_ref token_loc_start;

    token_loc_start = PARSE_PEEK();

    /* Skip 'if' */
    PARSE_SKIP();

    /* Match '(' */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), "(")) { PARSE_SKIP(); }
    else
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "'('");
    }

    /* Condition */
    if ((test = PARSE(expression)) == NULL) { goto fail; }

    /* Match ')' */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), ")")) { PARSE_SKIP(); }
    else
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "')'");
    }

    /* Consequent */
    if ((consequent = PARSE(statement)) == NULL) { goto fail; }

    /* Alternate */
    {
        xjs_token_ref token = PARSE_PEEK();
        if ((xjs_token_type_get(token) == XJS_TOKEN_TYPE_KEYWORD) && \
                (xjs_parser_keyword_type_resolve(token) == xjs_parser_keyword_else))
        {
            /* Skip 'else' */
            PARSE_SKIP();

            /* Consequent */
            if ((alternate = PARSE(statement)) == NULL) { goto fail; }
        }
    }

    {
        xjs_ast_statement_ref if_stmt;
        XJS_VNZ_ERROR_MEM(if_stmt = xjs_ast_statement_ifstatement_new(), PARSE_ERR);
        xjs_ast_statement_ifstatement_test_set(if_stmt, test); test = NULL;
        xjs_ast_statement_ifstatement_consequent_set(if_stmt, consequent); consequent = NULL;
        xjs_ast_statement_ifstatement_alternate_set(if_stmt, alternate); alternate = NULL;
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(if_stmt->u.xjs_ast_statement_ifstatement, token_loc_start);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(if_stmt->u.xjs_ast_statement_ifstatement, PARSE_PEEK_PREV());
        XJS_VNZ_ERROR_MEM_OR(new_item = xjs_ast_statementlistitem_statement_new(), \
                PARSE_ERR, ec_delete(if_stmt));
        xjs_ast_statementlistitem_statement_statement_set(new_item, if_stmt);
    }


fail:
    ec_delete(test);
    ec_delete(consequent);
    ec_delete(alternate);
    return new_item;
}

DEF_PARSE_ROUTINE(whilestatement, xjs_ast_statementlistitem_ref)
{
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_ast_expression_ref test = NULL;
    xjs_ast_statementlistitem_ref body = NULL;
    xjs_token_ref token_loc_start = NULL;

    token_loc_start = PARSE_PEEK();

    /* Skip 'while' */
    PARSE_SKIP();

    /* Match '(' */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), "(")) { PARSE_SKIP(); }
    else
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "'('");
    }

    /* Condition */
    if ((test = PARSE(expression)) == NULL) { goto fail; }

    /* Match ')' */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), ")")) { PARSE_SKIP(); }
    else
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "')'");
    }

    /* Body */
    if ((body = PARSE(statement)) == NULL) { goto fail; }

    {
        xjs_ast_statement_ref while_stmt;
        XJS_VNZ_ERROR_MEM(while_stmt = xjs_ast_statement_whilestatement_new(), PARSE_ERR);
        xjs_ast_statement_whilestatement_test_set(while_stmt, test); test = NULL;
        xjs_ast_statement_whilestatement_body_set(while_stmt, body); body = NULL;
        XJS_VNZ_ERROR_MEM_OR(new_item = xjs_ast_statementlistitem_statement_new(), \
                PARSE_ERR, ec_delete(while_stmt));
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(while_stmt->u.xjs_ast_statement_whilestatement, token_loc_start);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(while_stmt->u.xjs_ast_statement_whilestatement, PARSE_PEEK_PREV());
        xjs_ast_statementlistitem_statement_statement_set(new_item, while_stmt);
    }

fail:
    ec_delete(test);
    ec_delete(body);
    return new_item;
}

DEF_PARSE_ROUTINE(dostatement, xjs_ast_statementlistitem_ref)
{
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_ast_statementlistitem_ref body = NULL;
    xjs_ast_expression_ref test = NULL;
    xjs_token_ref token_loc_start = NULL;

    token_loc_start = PARSE_PEEK();

    /* Skip 'do' */
    PARSE_SKIP();

    /* Body */
    if ((body = PARSE(statement)) == NULL) { goto fail; }

    /* Match 'while' */
    if (xjs_parser_match_keyword(PARSE_PEEK(), xjs_parser_keyword_while) == ec_false)
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "'while'");
    }
    PARSE_SKIP();

    /* Match '(' */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), "(")) { PARSE_SKIP(); }
    else { PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "'('"); }

    /* Condition */
    if ((test = PARSE(expression)) == NULL) { goto fail; }

    /* Match ')' */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), ")")) { PARSE_SKIP(); }
    else
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "')'");
    }

    {
        xjs_ast_statement_ref do_stmt;
        XJS_VNZ_ERROR_MEM(do_stmt = xjs_ast_statement_dostatement_new(), PARSE_ERR);
        xjs_ast_statement_dostatement_body_set(do_stmt, body); body = NULL;
        xjs_ast_statement_dostatement_test_set(do_stmt, test); test = NULL;
        XJS_VNZ_ERROR_MEM_OR(new_item = xjs_ast_statementlistitem_statement_new(), \
                PARSE_ERR, ec_delete(do_stmt));
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(do_stmt->u.xjs_ast_statement_dostatement, token_loc_start);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(do_stmt->u.xjs_ast_statement_dostatement, PARSE_PEEK());
        xjs_ast_statementlistitem_statement_statement_set(new_item, do_stmt);
    }

    /* Match ';' */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), ";")) { PARSE_SKIP(); }
    else
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "';'");
    }

fail:
    ec_delete(test);
    ec_delete(body);
    return new_item;
}

DEF_PARSE_ROUTINE(forstatement, xjs_ast_statementlistitem_ref)
{
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_ast_statement_ref for_stmt = NULL;
    xjs_ast_statementlistitem_ref body = NULL;
    xjs_token_ref token_loc_start;

    token_loc_start = PARSE_PEEK();

    /* Skip 'for' */
    PARSE_SKIP();

    /* Match '(' */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), "(")) { PARSE_SKIP(); }
    else { PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "'('"); }

    XJS_VNZ_ERROR_MEM(for_stmt = xjs_ast_statement_forstatement_new(), PARSE_ERR);

    /* init */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), ";"))
    {
        xjs_ast_statement_forstatement_init_set_null(for_stmt);
        PARSE_SKIP();

        /* test */
        if (xjs_parser_match_punctuator(PARSE_PEEK(), ";"))
        {
            xjs_ast_statement_forstatement_test_set_null(for_stmt);
            PARSE_SKIP();
        }
        else
        {
            xjs_ast_expression_ref test_expr = NULL;
            if ((test_expr = PARSE(expression)) == NULL) { goto fail; }
            xjs_ast_statement_forstatement_test_set_expr(for_stmt, test_expr);

            /* Match ';' */
            if (xjs_parser_match_punctuator(PARSE_PEEK(), ";")) { PARSE_SKIP(); }
            else { PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "';'"); }
        }

        /* update */
        if (xjs_parser_match_punctuator(PARSE_PEEK(), ")"))
        {
            xjs_ast_statement_forstatement_update_set_null(for_stmt);
        }
        else
        {
            xjs_ast_expression_ref update_expr = NULL;
            if ((update_expr = PARSE(expression)) == NULL) { goto fail; }
            xjs_ast_statement_forstatement_update_set_expr(for_stmt, update_expr);
        }
    }
    else
    {
        if (xjs_parser_match_keyword(PARSE_PEEK(), xjs_parser_keyword_var))
        {
            /* for (var */
            PARSE_ERROR_PRINTF_UNEXPECTED_TOKEN(PARSE_PEEK());
        }
        else if (xjs_parser_match_keyword(PARSE_PEEK(), xjs_parser_keyword_const) || \
                xjs_parser_match_keyword(PARSE_PEEK(), xjs_parser_keyword_let))
        {
            /* for (const */
            /* for (let */
            PARSE_ERROR_PRINTF_UNEXPECTED_TOKEN(PARSE_PEEK());
        }
        else
        {
            /* for (varname */
            {
                xjs_ast_expression_ref init_expr = NULL;
                if ((init_expr = PARSE(expression)) == NULL) { goto fail; }
                xjs_ast_statement_forstatement_init_set_expr(for_stmt, init_expr);
            }
            /* Match ';' */
            if (xjs_parser_match_punctuator(PARSE_PEEK(), ";")) { PARSE_SKIP(); }
            else { PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "';'"); }

            /* test */
            if (xjs_parser_match_punctuator(PARSE_PEEK(), ";"))
            {
                xjs_ast_statement_forstatement_test_set_null(for_stmt);
                PARSE_SKIP();
            }
            else
            {
                xjs_ast_expression_ref test_expr = NULL;
                if ((test_expr = PARSE(expression)) == NULL) { goto fail; }
                xjs_ast_statement_forstatement_test_set_expr(for_stmt, test_expr);

                /* Match ';' */
                if (xjs_parser_match_punctuator(PARSE_PEEK(), ";")) { PARSE_SKIP(); }
                else { PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "';'"); }
            }

            /* update */
            if (xjs_parser_match_punctuator(PARSE_PEEK(), ")"))
            {
                xjs_ast_statement_forstatement_update_set_null(for_stmt);
            }
            else
            {
                xjs_ast_expression_ref update_expr = NULL;
                if ((update_expr = PARSE(expression)) == NULL) { goto fail; }
                xjs_ast_statement_forstatement_update_set_expr(for_stmt, update_expr);
            }
        }
    }

    /* Match ')' */
    if (xjs_parser_match_punctuator(PARSE_PEEK(), ")")) { PARSE_SKIP(); }
    else { PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "')'"); }

    if ((body = PARSE(statement)) == NULL) { goto fail; }
    xjs_ast_statement_forstatement_body_set(for_stmt, body); body = NULL;

    XJS_VNZ_ERROR_MEM_OR(new_item = xjs_ast_statementlistitem_statement_new(), \
            PARSE_ERR, ec_delete(for_stmt));
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(for_stmt->u.xjs_ast_statement_forstatement, token_loc_start);
    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(for_stmt->u.xjs_ast_statement_forstatement, PARSE_PEEK_PREV());
    xjs_ast_statementlistitem_statement_statement_set(new_item, for_stmt); for_stmt = NULL;

    goto done;
fail:
    ec_delete(body);
    ec_delete(for_stmt);
    ec_delete(new_item);
    new_item = NULL;
done:
    return new_item;
}

DEF_PARSE_ROUTINE(statement, xjs_ast_statementlistitem_ref)
{
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_token_ref token;
    xjs_token_type token_type;

    token = PARSE_PEEK();
    token_type = xjs_token_type_get(token);
    switch (token_type)
    {
        case XJS_TOKEN_TYPE_COMMENT:
            XJS_ERROR_INTERNAL(ctx->err);
            break;

        case XJS_TOKEN_TYPE_NULL_LITERAL:
        case XJS_TOKEN_TYPE_BOOLEAN_LITERAL:
        case XJS_TOKEN_TYPE_NUMERIC_LITERAL:
        case XJS_TOKEN_TYPE_STRING_LITERAL:
            new_item = PARSE(expression_statement);
            break;

        case XJS_TOKEN_TYPE_UNKNOWN:
        case XJS_TOKEN_TYPE_EOF:
            {
                PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "declaration or statement");
            }
            break;
        case XJS_TOKEN_TYPE_KEYWORD:
            {
                xjs_parser_keyword_t kw = xjs_parser_keyword_type_resolve(token);
                if (kw == xjs_parser_keyword_inspect) { new_item = PARSE(inspectstatement); }
                else if (kw == xjs_parser_keyword_break) { new_item = PARSE(breakstatement); }
                else if (kw == xjs_parser_keyword_continue) { new_item = PARSE(continuestatement); }
                else if (kw == xjs_parser_keyword_return) { new_item = PARSE(returnstatement); }
                else if (kw == xjs_parser_keyword_this) { new_item = PARSE(expression_statement); }
                else if (kw == xjs_parser_keyword_if) { new_item = PARSE(ifstatement); }
                else if (kw == xjs_parser_keyword_while) { new_item = PARSE(whilestatement); }
                else if (kw == xjs_parser_keyword_do) { new_item = PARSE(dostatement); }
                else if (kw == xjs_parser_keyword_for) { new_item = PARSE(forstatement); }
                else if (kw == xjs_parser_keyword_function)
                {
                    PARSE_ERROR_PRINTF("{c_str}:{size_t}:{size_t} :"
                            "error: named function is not supported", \
                            PARSE_SRCLOC, xjs_token_loc_start_ln_get(token), xjs_token_loc_start_col_get(token) + 1);
                }
                else
                {
                    PARSE_ERROR_PRINTF("{c_str}:{size_t}:{size_t} :"
                            "error: unexpected token {string}", \
                            PARSE_SRCLOC, xjs_token_loc_start_ln_get(token), xjs_token_loc_start_col_get(token) + 1, \
                            xjs_token_value_get(token));
                }
            }
            break;

        case XJS_TOKEN_TYPE_IDENTIFIER:
            new_item = PARSE(labeledstatement);
            break;

        case XJS_TOKEN_TYPE_PUNCTUATOR:
            if (xjs_parser_match_punctuator(token, ";") == ec_true)
            {
                new_item = PARSE(empty_statement);
            }
            else if (xjs_parser_match_punctuator(token, "{") == ec_true)
            {
                new_item = PARSE(block_statement);
            }
            else if (xjs_parser_match_punctuator(token, "(") == ec_true)
            {
                new_item = PARSE(expression_statement);
            }
            else
            {
                new_item = PARSE(expression_statement);
            }
    }

fail:
    return new_item;
}

DEF_PARSE_ROUTINE(statementlistitem, xjs_ast_statementlistitem_ref)
{
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_token_ref token;

    token = PARSE_PEEK();

    if (xjs_token_type_get(token) == XJS_TOKEN_TYPE_KEYWORD)
    {
        xjs_parser_keyword_t kw = xjs_parser_keyword_type_resolve(token);
        if ((kw == xjs_parser_keyword_var) || \
                (kw == xjs_parser_keyword_const) || \
                (kw == xjs_parser_keyword_let))
        {
            new_item = PARSEA(statementlistitem_declaration, kw);
        }
        else
        {
            new_item = PARSE(statement);
        }
    }
    else
    {
        new_item = PARSE(statement);
    }

    return new_item;
}

/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Statements/import */
DEF_PARSE_ROUTINE(importdeclaration, xjs_ast_moduleitem_ref)
{
    xjs_ast_moduleitem_ref new_moduleitem = NULL;
    xjs_ast_importdeclaration_ref new_importdeclaration = NULL;
    xjs_ast_importspecifier_ref new_importspecifier = NULL;
    xjs_token_ref token_cur, token_loc_start;

    token_loc_start = PARSE_PEEK();

    /* Skip 'import' */
    PARSE_SKIP();

    token_cur = PARSE_PEEK();

    XJS_VNZ_ERROR_MEM(new_moduleitem = xjs_ast_moduleitem_importdeclaration_new(), PARSE_ERR);

    if (xjs_token_type_get(token_cur) == XJS_TOKEN_TYPE_STRING_LITERAL)
    {
        xjs_ast_literal_ref new_literal;

        /* import 'module-name' */
        XJS_VNZ_ERROR_MEM(new_importdeclaration = xjs_ast_importdeclaration_new(), PARSE_ERR);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_importdeclaration, token_loc_start);
        {
            XJS_VNZ_ERROR_MEM(new_literal = xjs_ast_literal_new(), PARSE_ERR);
            xjs_ast_literal_raw_set(new_literal, ec_string_clone(xjs_token_value_get(token_cur)));
            {
                ec_string *string_literal_value = NULL;
                if ((string_literal_value = xjs_parser_extract_string_literal_value( \
                                xjs_token_value_get(token_cur))) == NULL)
                { XJS_ERROR_INTERNAL(ctx->err); }
                xjs_ast_literal_value_set_string(new_literal, string_literal_value);
            }
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN(new_literal, PARSE_PEEK());
        }
        PARSE_SKIP();

        xjs_ast_importdeclaration_source_set(new_importdeclaration, new_literal);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_importdeclaration, PARSE_PEEK());
        new_literal = NULL;
        xjs_ast_moduleitem_importdeclaration_set(new_moduleitem, new_importdeclaration);
        new_importdeclaration = NULL;
        /* Match ';' */
        if (xjs_parser_match_punctuator(PARSE_PEEK(), ";")) { PARSE_SKIP(); }
        else
        {
            PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "';'");
        }
    }
    else if (xjs_parser_match_punctuator(PARSE_PEEK(), "{"))
    {
        /* import { member } from 'moduleName' */
        PARSE_SKIP();

        XJS_VNZ_ERROR_MEM(new_importdeclaration = xjs_ast_importdeclaration_new(), PARSE_ERR);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_importdeclaration, token_loc_start);
        for (;;)
        {
            token_cur = PARSE_PEEK();
            if (xjs_parser_match_punctuator(token_cur, "}"))
            { PARSE_SKIP(); break; }

            token_cur = PARSE_PEEK();
            if (xjs_token_type_get(token_cur) == XJS_TOKEN_TYPE_IDENTIFIER)
            {
                xjs_token_ref local = NULL, imported = token_cur;

                PARSE_SKIP();
                token_cur = PARSE_PEEK();
                if ((xjs_token_type_get(token_cur) == XJS_TOKEN_TYPE_KEYWORD) && \
                        (xjs_parser_keyword_type_resolve(token_cur) == xjs_parser_keyword_as))
                {
                    PARSE_SKIP();
                    token_cur = PARSE_PEEK();
                    if (xjs_token_type_get(token_cur) == XJS_TOKEN_TYPE_IDENTIFIER)
                    { local = token_cur; }
                    else
                    {
                        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "identifier");
                    }
                    PARSE_SKIP();
                }
                else
                {
                    local = imported;
                }

                XJS_VNZ_ERROR_MEM(new_importspecifier = xjs_ast_importspecifier_importspecifier_new(), PARSE_ERR);
                new_importspecifier->u.importspecifier.local = xjs_ast_identifier_new(ec_string_clone(xjs_token_value_get(local)));
                PARSE_LOC_RANGE_CLONE_FROM_TOKEN(new_importspecifier->u.importspecifier.local, local);
                new_importspecifier->u.importspecifier.imported = xjs_ast_identifier_new(ec_string_clone(xjs_token_value_get(imported)));
                PARSE_LOC_RANGE_CLONE_FROM_TOKEN(new_importspecifier->u.importspecifier.imported, imported);
                PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_importspecifier, local);
                PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_importspecifier, imported);
                xjs_ast_importdeclaration_specifiers_push_back( \
                        new_importdeclaration, new_importspecifier);
                new_importspecifier = NULL;
            }
            else
            {
                PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "identifier");
            }

            if (xjs_parser_match_punctuator(PARSE_PEEK(), ","))
            { PARSE_SKIP(); }
        }

        token_cur = PARSE_PEEK();
        if ((xjs_token_type_get(token_cur) == XJS_TOKEN_TYPE_KEYWORD) && \
                (xjs_parser_keyword_type_resolve(token_cur) == xjs_parser_keyword_from))
        {
            PARSE_SKIP();
        }
        else
        {
            PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "'from'");
        }

        {
            xjs_ast_literal_ref new_literal;

            token_cur = PARSE_PEEK();
            if (xjs_token_type_get(token_cur) != XJS_TOKEN_TYPE_STRING_LITERAL)
            {
                PARSE_ERROR_PRINTF_EXPECTED_BEFORE(token_cur, "string literal");
            }

            XJS_VNZ_ERROR_MEM(new_literal = xjs_ast_literal_new(), PARSE_ERR);
            xjs_ast_literal_raw_set(new_literal, ec_string_clone(xjs_token_value_get(token_cur)));
            {
                ec_string *string_literal_value = NULL;
                if ((string_literal_value = xjs_parser_extract_string_literal_value( \
                                xjs_token_value_get(token_cur))) == NULL)
                { XJS_ERROR_INTERNAL(ctx->err); }
                xjs_ast_literal_value_set_string(new_literal, string_literal_value);
            }
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN(new_literal, token_cur);
            PARSE_SKIP();
            xjs_ast_importdeclaration_source_set(new_importdeclaration, new_literal);
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_importdeclaration, PARSE_PEEK());
            new_literal = NULL;
            xjs_ast_moduleitem_importdeclaration_set(new_moduleitem, new_importdeclaration);
            new_importdeclaration = NULL;
        }

        /* Match ';' */
        if (xjs_parser_match_punctuator(PARSE_PEEK(), ";")) { PARSE_SKIP(); }
        else
        {
            PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "';'");
        }
    }
    else if (xjs_token_type_get(token_cur) == XJS_TOKEN_TYPE_IDENTIFIER)
    {
        /* import importDefaultSpecifier from 'moduleName' */
        xjs_token_ref import_default_specifier;
        xjs_token_ref source = NULL;

        /* importDefaultSpecifier */
        import_default_specifier = token_cur;
        PARSE_SKIP();

        /* 'from */
        token_cur = PARSE_PEEK();
        if (!((xjs_token_type_get(token_cur) == XJS_TOKEN_TYPE_KEYWORD) && \
                    (xjs_parser_keyword_type_resolve(token_cur) == xjs_parser_keyword_from)))
        {
            PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "'from'");
        }
        PARSE_SKIP();

        /* "source" */
        token_cur = PARSE_PEEK();
        if (xjs_token_type_get(token_cur) != XJS_TOKEN_TYPE_STRING_LITERAL)
        {
            PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "string literal");
        }
        source = token_cur;
        PARSE_SKIP();

        /* Match ';' */
        if (!xjs_parser_match_punctuator(PARSE_PEEK(), ";"))
        {
            PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "';'");
        }
        PARSE_SKIP();

        XJS_VNZ_ERROR_MEM(new_importdeclaration = xjs_ast_importdeclaration_new(), PARSE_ERR);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_importdeclaration, token_loc_start);
        {
            xjs_ast_literal_ref new_literal;

            XJS_VNZ_ERROR_MEM(new_literal = xjs_ast_literal_new(), PARSE_ERR);
            xjs_ast_literal_raw_set(new_literal, ec_string_clone(xjs_token_value_get(source)));
            {
                ec_string *string_literal_value = NULL;
                if ((string_literal_value = xjs_parser_extract_string_literal_value( \
                                xjs_token_value_get(source))) == NULL)
                { XJS_ERROR_INTERNAL(ctx->err); }
                xjs_ast_literal_value_set_string(new_literal, string_literal_value);
            }
            xjs_ast_importdeclaration_source_set(new_importdeclaration, new_literal);
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN(new_literal, source);
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_importdeclaration, PARSE_PEEK_PREV());
            new_literal = NULL;
            {
                XJS_VNZ_ERROR_MEM(new_importspecifier = xjs_ast_importspecifier_importdefaultspecifier_new(), PARSE_ERR);
                new_importspecifier->u.importdefaultspecifier.local = xjs_ast_identifier_new(ec_string_clone(xjs_token_value_get(import_default_specifier)));
                PARSE_LOC_RANGE_CLONE_FROM_TOKEN(new_importspecifier->u.importdefaultspecifier.local, import_default_specifier);
                PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_importspecifier, import_default_specifier);
                PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_importspecifier, import_default_specifier);
                xjs_ast_importdeclaration_specifiers_push_back( \
                        new_importdeclaration, new_importspecifier);
                new_importspecifier = NULL;
            }
            xjs_ast_moduleitem_importdeclaration_set(new_moduleitem, new_importdeclaration);
            new_importdeclaration = NULL;
        }
    }
    else
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "'{', identifier or string literal");
    }

    goto done;
fail:
    if (new_moduleitem != NULL)
    { ec_delete(new_moduleitem); new_moduleitem = NULL; }
done:
    ec_delete(new_importdeclaration);
    ec_delete(new_importspecifier);
    return new_moduleitem;
}

DEF_PARSE_ROUTINE(exportdeclaration, xjs_ast_moduleitem_ref)
{
    xjs_ast_moduleitem_ref new_moduleitem = NULL;
    xjs_ast_exportdeclaration_ref new_exportdeclaration = NULL;
    xjs_token_ref token_cur, token_loc_start;

    token_loc_start = PARSE_PEEK();

    /* Skip 'export' */
    PARSE_SKIP();

    token_cur = PARSE_PEEK();

    if ((xjs_token_type_get(token_cur) == XJS_TOKEN_TYPE_KEYWORD) && \
            (xjs_parser_keyword_type_resolve(token_cur) == xjs_parser_keyword_default))
    {
        /* Skip 'default' */
        PARSE_SKIP();

        token_cur = PARSE_PEEK();
        if (xjs_token_type_get(token_cur) == XJS_TOKEN_TYPE_IDENTIFIER)
        {
            XJS_VNZ_ERROR_MEM(new_exportdeclaration = xjs_ast_exportdeclaration_default_new(), PARSE_ERR);
            {
                xjs_ast_exportdefaultdeclaration_ref new_defaultdecl;
                XJS_VNZ_ERROR_MEM(new_defaultdecl = xjs_ast_exportdefaultdeclaration_identifier_new(), PARSE_ERR);
                XJS_VNZ_ERROR_MEM_OR(new_defaultdecl->declaration.as_identifier = xjs_ast_identifier_new( \
                        ec_string_clone(xjs_token_value_get(token_cur))), \
                        PARSE_ERR, ec_delete(new_defaultdecl));
                PARSE_LOC_RANGE_CLONE_FROM_TOKEN(new_defaultdecl->declaration.as_identifier, token_cur);
                xjs_ast_exportdeclaration_default_set(new_exportdeclaration, new_defaultdecl);
                new_defaultdecl = NULL;
            }
            /* Skip identifier */
            PARSE_SKIP();
            XJS_VNZ_ERROR_MEM(new_moduleitem = xjs_ast_moduleitem_exportdeclaration_new(), PARSE_ERR);
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_exportdeclaration, token_loc_start);
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_exportdeclaration, PARSE_PEEK());
            xjs_ast_moduleitem_exportdeclaration_set(new_moduleitem, new_exportdeclaration);
            new_exportdeclaration = NULL;
        }
        else
        {
            xjs_ast_expression_ref new_expr;

            if (xjs_parser_match_punctuator(token_cur, "{"))
            {
                if ((new_expr = PARSE(object_initializer)) == NULL) { goto fail; }
            }
            else if (xjs_parser_match_punctuator(token_cur, "["))
            {
                if ((new_expr = PARSE(array_initializer)) == NULL) { goto fail; }
            }
            else
            {
                if ((new_expr = PARSE(assignmentexpression)) == NULL) { goto fail; }
            }

            XJS_VNZ_ERROR_MEM(new_exportdeclaration = xjs_ast_exportdeclaration_default_new(), PARSE_ERR);
            {
                xjs_ast_exportdefaultdeclaration_ref new_defaultdecl;
                XJS_VNZ_ERROR_MEM(new_defaultdecl = xjs_ast_exportdefaultdeclaration_expression_new(), PARSE_ERR);
                new_defaultdecl->declaration.as_expression = new_expr; new_expr = NULL;
                xjs_ast_exportdeclaration_default_set(new_exportdeclaration, new_defaultdecl);
                new_defaultdecl = NULL;
            }
            XJS_VNZ_ERROR_MEM(new_moduleitem = xjs_ast_moduleitem_exportdeclaration_new(), PARSE_ERR);
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_exportdeclaration, token_loc_start);
            PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_exportdeclaration, PARSE_PEEK());
            xjs_ast_moduleitem_exportdeclaration_set(new_moduleitem, new_exportdeclaration);
            new_exportdeclaration = NULL;
        }

        /* Match ';' */
        if (xjs_parser_match_punctuator(PARSE_PEEK(), ";")) { PARSE_SKIP(); }
        else
        {
            PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "';'");
        }
    }
    else if (xjs_parser_match_punctuator(PARSE_PEEK(), "{"))
    {
        /* export { ... }; */
        PARSE_SKIP();

        XJS_VNZ_ERROR_MEM(new_exportdeclaration = xjs_ast_exportdeclaration_named_new(), PARSE_ERR);
        {
            xjs_ast_exportnameddeclaration_ref new_nameddecl;
            XJS_VNZ_ERROR_MEM(new_nameddecl = xjs_ast_exportnameddeclaration_new(), PARSE_ERR);
            new_exportdeclaration->u.as_named = new_nameddecl;
        }
        for (;;)
        {
            token_cur = PARSE_PEEK();
            if (xjs_parser_match_punctuator(token_cur, "}"))
            { PARSE_SKIP(); break; }

            token_cur = PARSE_PEEK();
            if (xjs_token_type_get(token_cur) == XJS_TOKEN_TYPE_IDENTIFIER)
            {
                xjs_token_ref local = token_cur, exported = NULL;
                xjs_ast_exportspecifier_ref new_exportspecifier;

                PARSE_SKIP();
                token_cur = PARSE_PEEK();
                if ((xjs_token_type_get(token_cur) == XJS_TOKEN_TYPE_KEYWORD) && \
                        (xjs_parser_keyword_type_resolve(token_cur) == xjs_parser_keyword_as))
                {
                    PARSE_SKIP();
                    token_cur = PARSE_PEEK();
                    if (xjs_token_type_get(token_cur) == XJS_TOKEN_TYPE_IDENTIFIER)
                    { exported = token_cur; }
                    else
                    {
                        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(token_cur, "identifier");
                    }
                    PARSE_SKIP();
                }
                else
                {
                    exported = local;
                }

                XJS_VNZ_ERROR_MEM(new_exportspecifier = xjs_ast_exportspecifier_new(), PARSE_ERR);
                new_exportspecifier->local = xjs_ast_identifier_new(ec_string_clone(xjs_token_value_get(local)));
                PARSE_LOC_RANGE_CLONE_FROM_TOKEN(new_exportspecifier->local, local);
                new_exportspecifier->exported = xjs_ast_identifier_new(ec_string_clone(xjs_token_value_get(exported)));
                PARSE_LOC_RANGE_CLONE_FROM_TOKEN(new_exportspecifier->exported, exported);
                xjs_ast_exportdeclaration_named_specifiers_push_back( \
                        new_exportdeclaration, new_exportspecifier);
            }
            else
            {
                PARSE_ERROR_PRINTF_EXPECTED_BEFORE(token_cur, "'}' or identifier");
            }

            if (xjs_parser_match_punctuator(PARSE_PEEK(), ","))
            { PARSE_SKIP(); }
        }

        XJS_VNZ_ERROR_MEM(new_moduleitem = xjs_ast_moduleitem_exportdeclaration_new(), PARSE_ERR);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(new_exportdeclaration, token_loc_start);
        PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(new_exportdeclaration, PARSE_PEEK());
        xjs_ast_moduleitem_exportdeclaration_set(new_moduleitem, new_exportdeclaration);
        new_exportdeclaration = NULL;

        /* Match ';' */
        if (xjs_parser_match_punctuator(PARSE_PEEK(), ";")) { PARSE_SKIP(); }
        else
        {
            PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "';'");
        }
    }
    else
    {
        PARSE_ERROR_PRINTF_EXPECTED_BEFORE(PARSE_PEEK(), "'{' or 'default'");
    }

    goto done;
fail:
    if (new_moduleitem != NULL)
    { ec_delete(new_moduleitem); new_moduleitem = NULL; }
done:
    ec_delete(new_exportdeclaration);
    return new_moduleitem;
}

/*
DEF_PARSE_ROUTINE(importspecifier, xjs_ast_importspecifier_ref)
{
    XJS_ERROR_INTERNAL(ctx->err);
fail:
    return NULL;
}
*/

DEF_PARSE_ROUTINE(moduleitem, xjs_ast_moduleitem_ref)
{
    xjs_ast_moduleitem_ref new_moduleitem = NULL;
    xjs_ast_statementlistitem_ref new_item = NULL;
    xjs_token_ref token;
    xjs_token_type token_type;

    token = PARSE_PEEK();
    token_type = xjs_token_type_get(token);
    if (token_type == XJS_TOKEN_TYPE_KEYWORD)
    {
        xjs_parser_keyword_t kw = xjs_parser_keyword_type_resolve(token);
        if (kw == xjs_parser_keyword_import)
        {
            new_moduleitem = PARSE(importdeclaration);
        }
        else if (kw == xjs_parser_keyword_export)
        {
            new_moduleitem = PARSE(exportdeclaration);
        }
        else
        {
            if ((new_item = PARSE(statementlistitem)) == NULL)
            { goto fail; }
            XJS_VNZ_ERROR_MEM(new_moduleitem = xjs_ast_moduleitem_statementlistitem_new(), ctx->err);
            xjs_ast_moduleitem_statementlistitem_set(new_moduleitem, new_item);
            new_item = NULL;
        }
    }
    else
    {
        if ((new_item = PARSE(statementlistitem)) == NULL)
        { goto fail; }
        XJS_VNZ_ERROR_MEM(new_moduleitem = xjs_ast_moduleitem_statementlistitem_new(), ctx->err);
        xjs_ast_moduleitem_statementlistitem_set(new_moduleitem, new_item);
        new_item = NULL;
    }

fail:
    ec_delete(new_item);
    return new_moduleitem;
}

DEF_PARSE_ROUTINE(script, xjs_ast_program_ref)
{
    xjs_ast_program_ref new_program = NULL;
    xjs_ast_statementlistitem_ref new_item;

    XJS_VNZ_ERROR_MEM(new_program = xjs_ast_program_script_new(), PARSE_ERR);

    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(&(new_program->u.xjs_ast_program_script), PARSE_PEEK());

    while (!PARSE_EOF())
    {
        if ((new_item = PARSE(statementlistitem)) == NULL)
        { goto fail; }
        xjs_ast_program_script_push_back(new_program, new_item);
    }

    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(&(new_program->u.xjs_ast_program_script), PARSE_PEEK_PREV());

    goto done;
fail:
    if (new_program != NULL)
    { ec_delete(new_program); new_program = NULL; }
done:
    return new_program;
}

DEF_PARSE_ROUTINE(module, xjs_ast_program_ref)
{
    xjs_ast_program_ref new_program = NULL;
    xjs_ast_moduleitem_ref new_item;

    XJS_VNZ_ERROR_MEM(new_program = xjs_ast_program_module_new(), PARSE_ERR);

    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_START(&(new_program->u.xjs_ast_program_module), PARSE_PEEK());

    while (!PARSE_EOF())
    {
        if ((new_item = PARSE(moduleitem)) == NULL)
        { goto fail; }
        xjs_ast_program_module_push_back(new_program, new_item);
    }

    PARSE_LOC_RANGE_CLONE_FROM_TOKEN_END(&(new_program->u.xjs_ast_program_module), PARSE_PEEK_PREV());

    goto done;
fail:
    if (new_program != NULL)
    { ec_delete(new_program); new_program = NULL; }
done:
    return new_program;
}

xjs_ast_program_ref xjs_parser_start_ex( \
        xjs_error_ref err, \
        const xjs_token_list_ref tokens, \
        const xjs_bool module_mode, \
        const char *filename, \
        const xjs_bool loc)
{
    xjs_parser_ctx ctx;
    xjs_ast_program_ref new_program = NULL;
    token_iterator_stack *snapshot = NULL;

    (void)loc;

    snapshot = ect_stack_new(token_iterator_stack);

    xjs_parser_ctx_init(&ctx, err, tokens, snapshot);
    xjs_parser_ctx_set_source_filename(&ctx, filename);

    /* Parse Script */
    if (module_mode == xjs_false)
    {
        if ((new_program = PARSE_START(script, &ctx)) == NULL)
        { goto fail; }
    }
    else
    {
        if ((new_program = PARSE_START(module, &ctx)) == NULL)
        { goto fail; }
    }

    goto done;
fail:
    if (new_program != NULL)
    { ec_delete(new_program); new_program = NULL; }
done:
    ec_delete(snapshot);
    return new_program;
}

xjs_ast_program_ref xjs_parser_start( \
        xjs_error_ref err, \
        const xjs_token_list_ref tokens, \
        const xjs_bool module_mode)
{
    static const char *untitled = "untitled";
    return xjs_parser_start_ex( \
            err, \
            tokens, \
            module_mode, \
            untitled, \
            xjs_false);
}

