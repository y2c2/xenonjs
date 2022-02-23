/* XenonJS : C0 : Context
 * Copyright(c) 2017 y2c2 */

#include "xjs_error.h"
#include "xjs_c0_ctx.h"

#define _basename(_fullname) \
    (strrchr(_fullname, '/') ? strrchr(_fullname, '/') + 1 : _fullname)

static void xjs_varname_ctor(void *data)
{
    xjs_varname *r = data;
    r->name = NULL;
    r->loc.start.ln = -1;
    r->loc.start.col = -1;
    r->loc.end.ln = -1;
    r->loc.end.col = -1;
    r->range.start = -1;
    r->range.end = -1;
}

static void xjs_varname_dtor(void *data)
{
    xjs_varname *r = data;
    ec_delete(r->name);
}

xjs_varname *xjs_varname_new(ec_string *name)
{
    xjs_varname *r = ec_newcd( \
            xjs_varname, \
            xjs_varname_ctor, \
            xjs_varname_dtor);
    r->name = name;
    return r;
}

/* Variable Name Set */ 

static int xjs_varname_set_key_ctor(xjs_varname **detta_key, xjs_varname **key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_varname_set_key_dtor(xjs_varname **detta_key)
{
    ec_delete(*detta_key);
    return 0;
}

static int xjs_varname_set_key_cmp(xjs_varname **a, xjs_varname **b)
{
    return ec_string_cmp((*a)->name, (*b)->name);
}

ect_set_define_declared(xjs_varname_set, xjs_varname *, \
        xjs_varname_set_key_ctor, xjs_varname_set_key_dtor, \
        xjs_varname_set_key_cmp);

/* Scope */

static void xjs_c0_scope_ctor(void *data)
{
    xjs_c0_scope_ref r = data;
    r->hoisted_vars = NULL;
    r->parent = NULL;
}

static void xjs_c0_scope_dtor(void *data)
{
    xjs_c0_scope_ref r = data;
    ec_delete(r->hoisted_vars);
}

xjs_c0_scope_ref xjs_c0_scope_new(void)
{
    xjs_c0_scope_ref r = ec_newcd( \
            xjs_c0_scope, \
            xjs_c0_scope_ctor, \
            xjs_c0_scope_dtor);
    return r;
}

/* Scope Stack */

static void xjs_c0_scope_list_node_dtor(xjs_c0_scope_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_c0_scope_list, \
        xjs_c0_scope_ref, \
        xjs_c0_scope_list_node_dtor);

ect_stack_define(xjs_c0_scope_stack, \
        xjs_c0_scope_ref, \
        xjs_c0_scope_list);

static void xjs_c0_breakpoint_ctor(void *data)
{
    xjs_c0_breakpoint_ref r = data;
    r->node_break = NULL;
    r->node_continue = NULL;
}

static void xjs_c0_breakpoint_dtor(void *data)
{
    xjs_c0_breakpoint_ref r = data;
    (void)r;
}

xjs_c0_breakpoint_ref xjs_c0_breakpoint_new(void)
{
    xjs_c0_breakpoint_ref r = ec_newcd( \
            xjs_c0_breakpoint, \
            xjs_c0_breakpoint_ctor, \
            xjs_c0_breakpoint_dtor);
    return r;
}

static void xjs_c0_breakpoint_list_node_dtor(xjs_c0_breakpoint_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_c0_breakpoint_list, \
        xjs_c0_breakpoint_ref, \
        xjs_c0_breakpoint_list_node_dtor);

ect_stack_define(xjs_c0_breakpoint_stack, \
        xjs_c0_breakpoint_ref, \
        xjs_c0_breakpoint_list);

/* Context */

void xjs_c0_ctx_init( \
        xjs_c0_ctx *ctx, \
        xjs_error_ref err, \
        xjs_cfgbuilder_ref cfgb, \
        xjs_c0_scope_stack_ref scopes, \
        xjs_c0_breakpoint_stack_ref bps)
{
    ctx->err = err;
    ctx->cfgb = cfgb;
    ctx->scope.scopes = scopes;
    ctx->scope.root_scope = NULL;
    ctx->scope.top_scope = NULL;
    ctx->bps = bps;
    ctx->source_filename = NULL;
}

xjs_c0_scope_ref xjs_c0_ctx_append_scope( \
        xjs_c0_ctx *ctx)
{
    xjs_c0_scope_ref new_scope;

    if ((new_scope = xjs_c0_scope_new()) == NULL) return NULL;
    xjs_c0_scope_stack_push(ctx->scope.scopes, new_scope);

    if (ctx->scope.root_scope == NULL)
    {
        ctx->scope.root_scope = ctx->scope.top_scope = new_scope;
    }
    else
    {
        new_scope->parent = ctx->scope.top_scope;
        ctx->scope.top_scope = new_scope;
    }

    return new_scope;
}

/* Pop the top level of scope */
void xjs_c0_ctx_pop_scope( \
        xjs_c0_ctx *ctx)
{
    ctx->scope.top_scope = ctx->scope.top_scope->parent;
    if (ctx->scope.top_scope == NULL) ctx->scope.root_scope = ctx->scope.top_scope;
    xjs_c0_scope_stack_pop(ctx->scope.scopes);
}

int xjs_c0_ctx_push_break_continue_points(xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref break_point, xjs_cfg_node_ref continue_point)
{
    xjs_c0_breakpoint_ref new_bp = xjs_c0_breakpoint_new();
    if (new_bp == NULL) return -1;
    new_bp->node_break = break_point;
    new_bp->node_continue = continue_point;
    ect_stack_push(xjs_c0_breakpoint_stack, ctx->bps, new_bp);

    return 0;
}

int xjs_c0_ctx_pop_break_continue_points(xjs_c0_ctx *ctx)
{
    if (ect_stack_size(xjs_c0_breakpoint_stack, ctx->bps) == 0) return -1;
    ect_stack_pop(xjs_c0_breakpoint_stack, ctx->bps);
    return 0;
}

xjs_cfg_node_ref xjs_c0_ctx_top_break_point(xjs_c0_ctx *ctx)
{
    if (ect_stack_size(xjs_c0_breakpoint_stack, ctx->bps) == 0) return NULL;
    return ect_stack_top(xjs_c0_breakpoint_stack, ctx->bps)->node_break;
}

xjs_cfg_node_ref xjs_c0_ctx_top_continue_point(xjs_c0_ctx *ctx)
{
    if (ect_stack_size(xjs_c0_breakpoint_stack, ctx->bps) == 0) return NULL;
    return ect_stack_top(xjs_c0_breakpoint_stack, ctx->bps)->node_continue;
}

void xjs_c0_ctx_error_raw(xjs_c0_ctx *ctx, int err_no, \
        const char *__file__, const xjs_size_t __line__)
{
    xjs_error_update(ctx->err, __file__, __line__, err_no);
}

void xjs_c0_ctx_set_source_filename( \
        xjs_c0_ctx *ctx, const char *source_filename)
{
    ctx->source_filename = _basename(source_filename);
}

