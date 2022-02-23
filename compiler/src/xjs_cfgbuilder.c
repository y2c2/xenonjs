/* XenonJS : IR Builder
 * Copyright(c) 2017 y2c2 */

#include "xjs_cfg.h"
#include "xjs_helper.h"
#include "xjs_cfgbuilder.h"

#define XJS_CFG_VAR_INIT (0)

static void cfg_node_loc_range_clone(xjs_cfg_node_ref dst, xjs_cfg_node_ref src)
{
    dst->loc.start.ln = src->loc.start.ln;
    dst->loc.start.col = src->loc.start.col;
    dst->loc.end.ln = src->loc.end.ln;
    dst->loc.end.col = src->loc.end.col;
    dst->range.start = src->range.start;
    dst->range.end = src->range.end;
}

/* Allocate a variable */
xjs_cfg_var xjs_cfgbuilder_allocate_var(xjs_cfgbuilder_ref cfgb)
{
    xjs_cfg_var r = cfgb->var_pool++;
    return r;
}

/* Create a new builder */

static void xjs_cfgbuilder_ctor(void *data)  
{
    xjs_cfgbuilder_ref r = data;
    r->cfg = xjs_cfg_create();
    r->var_pool = XJS_CFG_VAR_INIT;
}

static void xjs_cfgbuilder_dtor(void *data)  
{
    xjs_cfgbuilder_ref r = data;
    ec_delete(r->cfg);
}

xjs_cfgbuilder_ref xjs_cfgbuilder_new(void)
{
    xjs_cfgbuilder_ref r = ec_newcd(xjs_cfgbuilder, \
            xjs_cfgbuilder_ctor, xjs_cfgbuilder_dtor);
    return r;
}

/* Alloca */
xjs_cfg_node_ref xjs_cfgbuilder_alloca_new( \
        xjs_cfgbuilder_ref cfgb, xjs_cfg_var var)
{
    xjs_cfg_alloca_ref alloca;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(alloca = xjs_cfg_alloca_new(var));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_alloca_new(alloca), ec_delete(alloca));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Load */
xjs_cfg_node_ref xjs_cfgbuilder_load_new( \
        xjs_cfgbuilder_ref cfgb, \
        ec_string *name, xjs_cfg_var var)
{
    ec_string *new_name;
    xjs_cfg_load_ref id;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(new_name = ec_string_clone(name));
    XJS_VNZ_RET_OR(id = xjs_cfg_load_new(new_name, var), ec_delete(new_name)); new_name = NULL;
    XJS_VNZ_RET_OR(r = xjs_cfg_node_load_new(id), ec_delete(id));
    ec_delete(name);
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Store */
xjs_cfg_node_ref xjs_cfgbuilder_store_new( \
        xjs_cfgbuilder_ref cfgb, \
        ec_string *name, xjs_cfg_var var)
{
    ec_string *v;
    xjs_cfg_store_ref store1;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(v = ec_string_clone(name));
    XJS_VNZ_RET_OR(store1 = xjs_cfg_store_new(v, var), ec_delete(v));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_store_new(store1), ec_delete(store1));
    ec_delete(name);
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Object Set */
xjs_cfg_node_ref xjs_cfgbuilder_object_set_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var dst, xjs_cfg_var obj, xjs_cfg_var key, xjs_cfg_var src)
{
    xjs_cfg_object_set_ref object_set;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(object_set = xjs_cfg_object_set_new(dst, obj, key, src));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_object_set_new(object_set), ec_delete(object_set));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Object Get */
xjs_cfg_node_ref xjs_cfgbuilder_object_get_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var dst, xjs_cfg_var obj, xjs_cfg_var key)
{
    xjs_cfg_object_get_ref object_get;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(object_get = xjs_cfg_object_get_new(dst, obj, key));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_object_get_new(object_get), ec_delete(object_get));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Array Push */
xjs_cfg_node_ref xjs_cfgbuilder_array_push_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var arr, xjs_cfg_var elem)
{
    xjs_cfg_array_push_ref array_push;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(array_push = xjs_cfg_array_push_new(arr, elem));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_array_push_new(array_push), ec_delete(array_push));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Literal */
xjs_cfg_node_ref xjs_cfgbuilder_literal_undefined_new( \
        xjs_cfgbuilder_ref cfgb, xjs_cfg_var var)
{
    xjs_cfg_literal_ref lit;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(lit = xjs_cfg_literal_undefined_new(var));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_literal_new(lit), ec_delete(lit));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

xjs_cfg_node_ref xjs_cfgbuilder_literal_null_new( \
        xjs_cfgbuilder_ref cfgb, xjs_cfg_var var)
{
    xjs_cfg_literal_ref lit;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(lit = xjs_cfg_literal_null_new(var));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_literal_new(lit), ec_delete(lit));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

xjs_cfg_node_ref xjs_cfgbuilder_literal_false_new( \
        xjs_cfgbuilder_ref cfgb, xjs_cfg_var var)
{
    xjs_cfg_literal_ref lit;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(lit = xjs_cfg_literal_false_new(var));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_literal_new(lit), ec_delete(lit));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

xjs_cfg_node_ref xjs_cfgbuilder_literal_true_new( \
        xjs_cfgbuilder_ref cfgb, xjs_cfg_var var)
{
    xjs_cfg_literal_ref lit;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(lit = xjs_cfg_literal_true_new(var));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_literal_new(lit), ec_delete(lit));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

xjs_cfg_node_ref xjs_cfgbuilder_literal_number_new( \
        xjs_cfgbuilder_ref cfgb, double value, xjs_cfg_var var)
{
    xjs_cfg_literal_ref lit;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(lit = xjs_cfg_literal_number_new(value, var));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_literal_new(lit), ec_delete(lit));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

xjs_cfg_node_ref xjs_cfgbuilder_literal_string_new( \
        xjs_cfgbuilder_ref cfgb, ec_string *value, xjs_cfg_var var)
{
    ec_string *v;
    xjs_cfg_literal_ref lit;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(v = ec_string_clone(value));
    XJS_VNZ_RET_OR(lit = xjs_cfg_literal_string_new(v, var), ec_delete(v));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_literal_new(lit), ec_delete(lit));
    ec_delete(value);
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

xjs_cfg_node_ref xjs_cfgbuilder_literal_object_new( \
        xjs_cfgbuilder_ref cfgb, xjs_cfg_var var)
{
    xjs_cfg_literal_ref lit;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(lit = xjs_cfg_literal_object_new(var));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_literal_new(lit), ec_delete(lit));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

xjs_cfg_node_ref xjs_cfgbuilder_literal_array_new( \
        xjs_cfgbuilder_ref cfgb, xjs_cfg_var var)
{
    xjs_cfg_literal_ref lit;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(lit = xjs_cfg_literal_array_new(var));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_literal_new(lit), ec_delete(lit));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Unary Operations */
xjs_cfg_node_ref xjs_cfgbuilder_unary_op_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_unary_op_type type, xjs_cfg_var var_dst, xjs_cfg_var var_src)
{
    xjs_cfg_unary_op_ref unary_op;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(unary_op = xjs_cfg_unary_op_new(type, var_dst, var_src));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_unary_op_new(unary_op), ec_delete(unary_op));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Binary Operations */
xjs_cfg_node_ref xjs_cfgbuilder_binary_op_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_binary_op_type type, \
        xjs_cfg_var var_dst, \
        xjs_cfg_var var_src1, xjs_cfg_var var_src2)
{
    xjs_cfg_binary_op_ref binary_op;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(binary_op = xjs_cfg_binary_op_new(type, var_dst, var_src1, var_src2));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_binary_op_new(binary_op), ec_delete(binary_op));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Drop */
xjs_cfg_node_ref xjs_cfgbuilder_drop_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_node_ref value)
{
    xjs_cfg_drop_ref drop;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(drop = xjs_cfg_drop_new(value));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_drop_new(drop), ec_delete(drop));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Halt */
xjs_cfg_node_ref xjs_cfgbuilder_halt_new( \
        xjs_cfgbuilder_ref cfgb)
{
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(r = xjs_cfg_node_halt_new());
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Jump */
xjs_cfg_node_ref xjs_cfgbuilder_jump_new( \
        xjs_cfgbuilder_ref cfgb)
{
    xjs_cfg_jump_ref jump;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(jump = xjs_cfg_jump_new());
    XJS_VNZ_RET_OR(r = xjs_cfg_node_jump_new(jump), ec_delete(jump));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

void xjs_cfgbuilder_jump_dest_set( \
        xjs_cfg_node_ref jump, xjs_cfg_node_ref dest)
{
    xjs_cfg_jump_dest_set(jump->u.as_jump, dest);
}

/* Branch */

xjs_cfg_node_ref xjs_cfgbuilder_branch_new( \
        xjs_cfgbuilder_ref cfgb)
{
    xjs_cfg_branch_ref branch;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(branch = xjs_cfg_branch_new());
    XJS_VNZ_RET_OR(r = xjs_cfg_node_branch_new(branch), ec_delete(branch));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

void xjs_cfgbuilder_branch_cond_set( \
        xjs_cfg_node_ref branch, xjs_cfg_var cond)
{
    xjs_cfg_branch_cond_set(branch->u.as_branch, cond);
}

void xjs_cfgbuilder_branch_true_branch_set( \
        xjs_cfg_node_ref branch, xjs_cfg_node_ref true_branch)
{
    xjs_cfg_branch_true_branch_set(branch->u.as_branch, true_branch);
}

void xjs_cfgbuilder_branch_false_branch_set( \
        xjs_cfg_node_ref branch, xjs_cfg_node_ref false_branch)
{
    xjs_cfg_branch_false_branch_set(branch->u.as_branch, false_branch);
}

/* Merge */

xjs_cfg_node_ref xjs_cfgbuilder_merge_new( \
        xjs_cfgbuilder_ref cfgb)
{
    xjs_cfg_merge_ref merge;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(merge = xjs_cfg_merge_new());
    XJS_VNZ_RET_OR(r = xjs_cfg_node_merge_new(merge), ec_delete(merge));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

void xjs_cfgbuilder_merge_test_set( \
        xjs_cfg_node_ref merge, xjs_cfg_var test)
{
    xjs_cfg_merge_test_set(merge->u.as_merge, test);
}

void xjs_cfgbuilder_merge_consequent_set( \
        xjs_cfg_node_ref merge, xjs_cfg_var consequent)
{
    xjs_cfg_merge_consequent_set(merge->u.as_merge, consequent);
}

void xjs_cfgbuilder_merge_alternate_set( \
        xjs_cfg_node_ref merge, xjs_cfg_var alternate)
{
    xjs_cfg_merge_alternate_set(merge->u.as_merge, alternate);
}

void xjs_cfgbuilder_merge_dst_set( \
        xjs_cfg_node_ref merge, xjs_cfg_var dst)
{
    xjs_cfg_merge_dst_set(merge->u.as_merge, dst);
}

/* Block */

xjs_cfg_node_ref xjs_cfgbuilder_block_new( \
        xjs_cfgbuilder_ref cfgb)
{
    xjs_cfg_block_ref block;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(block = xjs_cfg_block_new());
    XJS_VNZ_RET_OR(r = xjs_cfg_node_block_new(block), ec_delete(block));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

void xjs_cfgbuilder_block_push_back( \
        xjs_cfg_node_ref block, xjs_cfg_node_ref sub)
{
    xjs_cfg_block_push_back(block->u.as_block, sub);
}

/* DeclVar */

xjs_cfg_node_ref xjs_cfgbuilder_declvar_new( \
        xjs_cfgbuilder_ref cfgb, \
        ec_string *name)
{
    ec_string *v;
    xjs_cfg_declvar_ref declvar;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(v = ec_string_clone(name));
    XJS_VNZ_RET_OR(declvar = xjs_cfg_declvar_new(v), ec_delete(v));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_declvar_new(declvar), ec_delete(declvar));
    ec_delete(name);
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Function */
xjs_cfg_parameter_ref xjs_cfgbuilder_parameter_new(ec_string *name)
{
    return xjs_cfg_parameter_new(name);
}

xjs_cfg_function_ref xjs_cfgbuilder_function_new( \
        xjs_cfg_parameter_list_ref parameters, xjs_cfg_node_ref body)
{
    return xjs_cfg_function_new(parameters, body);
}

/* Make Function */
xjs_cfg_node_ref xjs_cfgbuilder_make_function_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_function_ref func, \
        xjs_cfg_var var_dst)
{
    xjs_cfg_make_function_ref make_function;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(make_function = xjs_cfg_make_function_new(func, var_dst));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_make_function_new(make_function), ec_delete(make_function));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Make Arrow Function */
xjs_cfg_node_ref xjs_cfgbuilder_make_arrow_function_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_function_ref func, \
        xjs_cfg_var var_dst)
{
    xjs_cfg_make_function_ref make_function;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(make_function = xjs_cfg_make_function_new(func, var_dst));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_make_function_new(make_function), ec_delete(make_function));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Inspect */
xjs_cfg_node_ref xjs_cfgbuilder_inspect_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var var)
{
    xjs_cfg_inspect_ref ret;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(ret = xjs_cfg_inspect_new(var));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_inspect_new(ret), ec_delete(ret));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Return */
xjs_cfg_node_ref xjs_cfgbuilder_return_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var var)
{
    xjs_cfg_return_ref ret;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(ret = xjs_cfg_return_new(var));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_return_new(ret), ec_delete(ret));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Call */
xjs_cfg_node_ref xjs_cfgbuilder_call_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var dst, \
        xjs_cfg_var callee, \
        xjs_cfg_var_list_ref arguments)
{
    xjs_cfg_call_ref call;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(call = xjs_cfg_call_new(dst, callee, arguments));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_call_new(call), ec_delete(call));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

xjs_cfg_node_ref xjs_cfgbuilder_call_bound_this_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var dst, \
        xjs_cfg_var callee, \
        xjs_cfg_var_list_ref arguments, \
        xjs_cfg_var _this)
{
    xjs_cfg_call_ref call;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(call = xjs_cfg_call_new_bound_this(dst, callee, arguments, _this));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_call_new(call), ec_delete(call));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* New */
xjs_cfg_node_ref xjs_cfgbuilder_new_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var dst, \
        xjs_cfg_var callee, \
        xjs_cfg_var_list_ref arguments)
{
    xjs_cfg_new_ref _new;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(_new = xjs_cfg_new_new(dst, callee, arguments));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_new_new(_new), ec_delete(_new));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* This */
xjs_cfg_node_ref xjs_cfgbuilder_this_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var dst)
{
    xjs_cfg_this_ref _this;
    xjs_cfg_node_ref r;
    XJS_VNZ_RET(_this = xjs_cfg_this_new(dst));
    XJS_VNZ_RET_OR(r = xjs_cfg_node_this_new(_this), ec_delete(_this));
    xjs_cfg_manage(cfgb->cfg, r);
    return r;
}

/* Link */
xjs_cfg_node_ref xjs_cfgbuilder_link( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_node_ref src, xjs_cfg_node_ref dest)
{
    xjs_cfg_node_ref ret = NULL;
    xjs_cfg_node_ref node_jump;

    if ((node_jump = xjs_cfgbuilder_jump_new(cfgb)) == NULL) { return NULL; }
    cfg_node_loc_range_clone(node_jump, src);
    xjs_cfgbuilder_jump_dest_set(node_jump, dest);

    if (src->type == XJS_CFG_NODE_TYPE_BLOCK)
    {
        xjs_cfgbuilder_block_push_back(src, node_jump);
        ret = src;
    }
    else
    {
        xjs_cfg_node_ref node_block;
        if ((node_block = xjs_cfgbuilder_block_new(cfgb)) == NULL) { return NULL; }
        cfg_node_loc_range_clone(node_block, src);
        xjs_cfgbuilder_block_push_back(node_block, src);
        xjs_cfgbuilder_block_push_back(node_block, node_jump);
        ret = node_block;
    }

    return ret;
}

/* Export Symbol */
xjs_cfg_export_symbol_ref xjs_cfgbuilder_export_symbol_new( \
        ec_string *exported, ec_string *local)
{
    xjs_cfg_export_symbol_ref r;

    if ((r = xjs_cfg_export_symbol_new(exported, local)) == NULL)
    { return NULL; }

    return r;
}

/* Import Symbol */
xjs_cfg_import_symbol_ref xjs_cfgbuilder_import_symbol_new( \
        ec_string *local, ec_string *imported, ec_string *source)
{
    xjs_cfg_import_symbol_ref r;

    if ((r = xjs_cfg_import_symbol_new(local, imported, source)) == NULL)
    { return NULL; }

    return r;
}

/* CFG */
void xjs_cfgbuilder_set_top_level( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_function_ref top_level)
{
    xjs_cfg_set_top_level(cfgb->cfg, top_level);
}

void xjs_cfgbuilder_append_function( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_function_ref func)
{
    xjs_cfg_append_function(cfgb->cfg, func);
}

void xjs_cfgbuilder_append_export_symbol( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_export_symbol_ref new_symbol)
{
    xjs_cfg_append_export_symbol(cfgb->cfg, new_symbol);
}

void xjs_cfgbuilder_append_import_symbol( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_import_symbol_ref new_symbol)
{
    xjs_cfg_append_import_symbol(cfgb->cfg, new_symbol);
}

/* Generate CFG */
xjs_cfg_ref xjs_cfgbuilder_cfg_generate( \
        xjs_cfgbuilder_ref cfgb)
{
    xjs_cfg_ref cfg = cfgb->cfg;
    cfgb->cfg = NULL;
    return cfg;
}

