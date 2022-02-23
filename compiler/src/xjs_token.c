/* XenonJS : Token
 * Copyright(c) 2017 y2c2 */

#include <ec_alloc.h>
#include <ec_string.h>
#include <ec_list.h>
#include "xjs_dt.h"
#include "xjs_alloc.h"
#include "xjs_helper.h"
#include "xjs_token.h"

struct xjs_opaque_token
{
    xjs_token_type type;

    /* Text */
    ec_string *value;

    /* Location */
    struct
    {
        struct
        {
            xjs_size_t ln, col;
        } start, end;
    } loc;

    /* Range */
    struct
    {
        xjs_size_t start, end;
    } range;
};

static void xjs_token_ctor(void *data)
{
    xjs_token_ref r = data;
    r->type = XJS_TOKEN_TYPE_UNKNOWN;
    r->value = NULL;
    r->loc.start.ln = 0;
    r->loc.start.col = 0;
    r->loc.end.ln = 0;
    r->loc.end.col = 0;
    r->range.start = 0;
    r->range.end = 0;
}

static void xjs_token_dtor(void *data)
{
    xjs_token_ref r = data;
    ec_delete(r->value);
}

xjs_token_ref xjs_token_new(void)
{
    xjs_token_ref r;

    XJS_VNZ_RET(r = xjs_newcd(xjs_token, xjs_token_ctor, xjs_token_dtor));

    return r;
}

xjs_token_type xjs_token_type_get(xjs_token_ref token)
{
    return token->type;
}

ec_string *xjs_token_value_get(xjs_token_ref token)
{
    return token->value;
}

void xjs_token_type_set(xjs_token_ref token, xjs_token_type type)
{
    token->type = type;
}

void xjs_token_value_set(xjs_token_ref token, ec_string *value)
{
    ec_delete(token->value);
    token->value = value;
}

static void xjs_token_list_node_dtor(xjs_token_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_token_list, xjs_token_ref, xjs_token_list_node_dtor);

/* Loc */
void xjs_token_loc_start_set(xjs_token_ref token, \
        const xjs_size_t start_ln, const xjs_size_t start_col)
{
    token->loc.start.ln = start_ln;
    token->loc.start.col = start_col;
}

void xjs_token_loc_end_set(xjs_token_ref token, \
        const xjs_size_t end_ln, const xjs_size_t end_col)
{
    token->loc.end.ln = end_ln;
    token->loc.end.col = end_col;
}

xjs_size_t xjs_token_loc_start_ln_get(xjs_token_ref token)
{
    return token->loc.start.ln;
}

xjs_size_t xjs_token_loc_start_col_get(xjs_token_ref token)
{
    return token->loc.start.col;
}

xjs_size_t xjs_token_loc_end_ln_get(xjs_token_ref token)
{
    return token->loc.end.ln;
}

xjs_size_t xjs_token_loc_end_col_get(xjs_token_ref token)
{
    return token->loc.end.col;
}

void xjs_token_range_start_set(xjs_token_ref token, \
        const xjs_size_t start)
{
    token->range.start = start;
}

void xjs_token_range_end_set(xjs_token_ref token, \
        const xjs_size_t end)
{
    token->range.end = end;
}

xjs_size_t xjs_token_range_start_get(xjs_token_ref token)
{
    return token->range.start;
}

xjs_size_t xjs_token_range_end_get(xjs_token_ref token)
{
    return token->range.end;
}

