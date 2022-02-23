/* XenonJS : C0 : Context
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_C0_CTX_H
#define XJS_C0_CTX_H

#include <ec_set.h>
#include <ec_list.h>
#include <ec_stack.h>
#include <ec_string.h>
#include "xjs_dt.h"
#include "xjs_types.h"
#include "xjs_error.h"
#include "xjs_cfgbuilder.h"

 
/* Variable Name Set */ 

struct xjs_opaque_varname
{
    ec_string *name;
    struct
    {
        struct
        {
            int ln, col;
        } start, end;
    } loc;
    struct
    {
        int start, end;
    } range;
};
typedef struct xjs_opaque_varname xjs_varname;

xjs_varname *xjs_varname_new(ec_string *name);

ect_set_declare(xjs_varname_set, xjs_varname *);
typedef xjs_varname_set *xjs_varname_set_ref;

/* Scope */

struct xjs_opaque_c0_scope;
typedef struct xjs_opaque_c0_scope xjs_c0_scope;
typedef struct xjs_opaque_c0_scope *xjs_c0_scope_ref;

struct xjs_opaque_c0_scope
{
    xjs_varname_set_ref hoisted_vars;
    xjs_c0_scope_ref parent;
};

xjs_c0_scope_ref xjs_c0_scope_new(void);

/* Scope Stack */

ect_list_declare(xjs_c0_scope_list, xjs_c0_scope_ref);
ect_stack_declare(xjs_c0_scope_stack, xjs_c0_scope_ref, xjs_c0_scope_list);
typedef xjs_c0_scope_stack *xjs_c0_scope_stack_ref;

/* Break Point */

struct xjs_opaque_c0_breakpoint;
typedef struct xjs_opaque_c0_breakpoint xjs_c0_breakpoint;
typedef struct xjs_opaque_c0_breakpoint *xjs_c0_breakpoint_ref;

struct xjs_opaque_c0_breakpoint
{
    xjs_cfg_node_ref node_break;
    xjs_cfg_node_ref node_continue;
};

xjs_c0_breakpoint_ref xjs_c0_breakpoint_new(void);

/* Break Point Stack */

ect_list_declare(xjs_c0_breakpoint_list, xjs_c0_breakpoint_ref);
ect_stack_declare(xjs_c0_breakpoint_stack, xjs_c0_breakpoint_ref, xjs_c0_breakpoint_list);
typedef xjs_c0_breakpoint_stack *xjs_c0_breakpoint_stack_ref;

/* Context */
typedef struct
{
    xjs_error_ref err;
    xjs_cfgbuilder_ref cfgb;

    struct
    {
        /* Scope Stack */
        xjs_c0_scope_stack_ref scopes;
        /* The Root Scope */
        xjs_c0_scope_ref root_scope;
        /* Top */
        xjs_c0_scope_ref top_scope;
    } scope;

    xjs_c0_breakpoint_stack_ref bps;

    const char *source_filename;
} xjs_c0_ctx;

void xjs_c0_ctx_init( \
        xjs_c0_ctx *ctx, \
        xjs_error_ref err, \
        xjs_cfgbuilder_ref ir, \
        xjs_c0_scope_stack_ref scopes, \
        xjs_c0_breakpoint_stack_ref bps);

/* Create a new scope, append to the top and return the scope itself */
xjs_c0_scope_ref xjs_c0_ctx_append_scope( \
        xjs_c0_ctx *ctx);

/* Pop the top level of scope */
void xjs_c0_ctx_pop_scope( \
        xjs_c0_ctx *ctx);

int xjs_c0_ctx_push_break_continue_points(xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref break_point, xjs_cfg_node_ref continue_point);

int xjs_c0_ctx_pop_break_continue_points(xjs_c0_ctx *ctx);

xjs_cfg_node_ref xjs_c0_ctx_top_break_point(xjs_c0_ctx *ctx);
xjs_cfg_node_ref xjs_c0_ctx_top_continue_point(xjs_c0_ctx *ctx);


void xjs_c0_ctx_error_raw(xjs_c0_ctx *ctx, int err_no, \
        const char *__file__, const xjs_size_t __line__);

void xjs_c0_ctx_set_source_filename( \
        xjs_c0_ctx *ctx, const char *source_filename);

#define xjs_c0_ctx_error(_ctx, _err_no) \
    do { \
        xjs_c0_ctx_error_raw(_ctx, _err_no, __FILE__, __LINE__); \
        goto fail; \
    } while (0)


#endif

