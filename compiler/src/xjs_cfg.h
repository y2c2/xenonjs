/* XenonJS : Control Flow Graph
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_CFG_H
#define XJS_CFG_H

#include "xjs_types.h"

/* CFG Context Nodes */

struct xjs_opaque_cfgbuilder
{
    xjs_cfg_ref cfg;
    xjs_cfg_var var_pool;
};


/*  Design
 *  ------
 *  Different component has different types, node is something
 *  we use to wcfge things up.
 *
 *  Layout Overview
 *  ----------------
 *  CFG
 *    Function[] -> 
 *      Function(Parameter, Block)
 *  
 */

/* Comment */
int xjs_cfg_node_comment(xjs_cfg_node_ref node, const char *comment);

/* Loc */
void xjs_cfg_node_set_loc_start(xjs_cfg_node_ref node, const int start_ln, const int start_col);
void xjs_cfg_node_set_loc_end(xjs_cfg_node_ref node, const int end_ln, const int end_col);
/* Range */
void xjs_cfg_node_set_range_start(xjs_cfg_node_ref node, const int start);
void xjs_cfg_node_set_range_end(xjs_cfg_node_ref node, const int end);

/* Literal */
xjs_cfg_literal_ref xjs_cfg_literal_undefined_new(xjs_cfg_var var);
xjs_cfg_literal_ref xjs_cfg_literal_null_new(xjs_cfg_var var);
xjs_cfg_literal_ref xjs_cfg_literal_false_new(xjs_cfg_var var);
xjs_cfg_literal_ref xjs_cfg_literal_true_new(xjs_cfg_var var);
xjs_cfg_literal_ref xjs_cfg_literal_number_new(double value, xjs_cfg_var var);
xjs_cfg_literal_ref xjs_cfg_literal_string_new(ec_string *value, xjs_cfg_var var);
xjs_cfg_literal_ref xjs_cfg_literal_object_new(xjs_cfg_var var);
xjs_cfg_literal_ref xjs_cfg_literal_array_new(xjs_cfg_var var);
xjs_cfg_node_ref xjs_cfg_node_literal_new(xjs_cfg_literal_ref lit);

/* Alloca */
xjs_cfg_alloca_ref xjs_cfg_alloca_new(xjs_cfg_var var);
xjs_cfg_node_ref xjs_cfg_node_alloca_new(xjs_cfg_alloca_ref alloca);

/* Load from Identifier */
xjs_cfg_load_ref xjs_cfg_load_new(ec_string *name, xjs_cfg_var var);
xjs_cfg_node_ref xjs_cfg_node_load_new(xjs_cfg_load_ref id);

/* Assign Value to Variable */
xjs_cfg_store_ref xjs_cfg_store_new(ec_string *name, xjs_cfg_var var);
xjs_cfg_node_ref xjs_cfg_node_store_new(xjs_cfg_store_ref store1);

/* Object Set */
xjs_cfg_object_set_ref xjs_cfg_object_set_new( \
        xjs_cfg_var dst, xjs_cfg_var obj, xjs_cfg_var key, xjs_cfg_var src);
xjs_cfg_node_ref xjs_cfg_node_object_set_new(xjs_cfg_object_set_ref object_set);

/* Object Get */
xjs_cfg_object_get_ref xjs_cfg_object_get_new( \
        xjs_cfg_var dst, xjs_cfg_var obj, xjs_cfg_var key);
xjs_cfg_node_ref xjs_cfg_node_object_get_new(xjs_cfg_object_get_ref object_get);

/* Array Push */
xjs_cfg_array_push_ref xjs_cfg_array_push_new( \
        xjs_cfg_var arr, xjs_cfg_var elem);
xjs_cfg_node_ref xjs_cfg_node_array_push_new(xjs_cfg_array_push_ref array_push);

/* Unary Operations */ 
xjs_cfg_unary_op_ref xjs_cfg_unary_op_new( \
        xjs_cfg_unary_op_type type, \
        xjs_cfg_var var_dst, xjs_cfg_var var_src);
xjs_cfg_node_ref xjs_cfg_node_unary_op_new(xjs_cfg_unary_op_ref unary);

/* Binary Operations */ 
xjs_cfg_binary_op_ref xjs_cfg_binary_op_new( \
        xjs_cfg_binary_op_type type, \
        xjs_cfg_var var_dst, xjs_cfg_var var_src1, xjs_cfg_var var_src2);
xjs_cfg_node_ref xjs_cfg_node_binary_op_new(xjs_cfg_binary_op_ref binary);

/* Drop Value */
xjs_cfg_drop_ref xjs_cfg_drop_new(xjs_cfg_node_ref value);
xjs_cfg_node_ref xjs_cfg_node_drop_new(xjs_cfg_drop_ref drop);

/* Halt */
xjs_cfg_node_ref xjs_cfg_node_halt_new(void);

/* Jump */
xjs_cfg_jump_ref xjs_cfg_jump_new(void);
void xjs_cfg_jump_dest_set(xjs_cfg_jump_ref jump, xjs_cfg_node_ref dest);
xjs_cfg_node_ref xjs_cfg_node_jump_new(xjs_cfg_jump_ref jump);

/* Branch */
xjs_cfg_branch_ref xjs_cfg_branch_new(void);
void xjs_cfg_branch_cond_set(xjs_cfg_branch_ref branch, xjs_cfg_var cond);
void xjs_cfg_branch_true_branch_set(xjs_cfg_branch_ref branch, xjs_cfg_node_ref true_branch);
void xjs_cfg_branch_false_branch_set(xjs_cfg_branch_ref branch, xjs_cfg_node_ref false_branch);
xjs_cfg_node_ref xjs_cfg_node_branch_new(xjs_cfg_branch_ref branch);

/* Merge */
xjs_cfg_merge_ref xjs_cfg_merge_new(void);
void xjs_cfg_merge_test_set(xjs_cfg_merge_ref merge, xjs_cfg_var test);
void xjs_cfg_merge_consequent_set(xjs_cfg_merge_ref merge, xjs_cfg_var consequent);
void xjs_cfg_merge_alternate_set(xjs_cfg_merge_ref merge, xjs_cfg_var alternate);
void xjs_cfg_merge_dst_set(xjs_cfg_merge_ref merge, xjs_cfg_var dst);
xjs_cfg_node_ref xjs_cfg_node_merge_new(xjs_cfg_merge_ref merge);

/* Block */
xjs_cfg_block_ref xjs_cfg_block_new(void);
void xjs_cfg_block_push_back(xjs_cfg_block_ref block, xjs_cfg_node_ref sub);
xjs_cfg_node_ref xjs_cfg_node_block_new(xjs_cfg_block_ref block);

/* Declaration Variable */
xjs_cfg_declvar_ref xjs_cfg_declvar_new(ec_string *name);
xjs_cfg_node_ref xjs_cfg_node_declvar_new(xjs_cfg_declvar_ref declvar);

/* Make Function */
xjs_cfg_make_function_ref xjs_cfg_make_function_new( \
        xjs_cfg_function_ref func, \
        xjs_cfg_var var_dst);
xjs_cfg_node_ref xjs_cfg_node_make_function_new(xjs_cfg_make_function_ref make_function);

/* Make Arrow Function */
xjs_cfg_make_arrow_function_ref xjs_cfg_make_arrow_function_new( \
        xjs_cfg_function_ref func, \
        xjs_cfg_var var_dst);
xjs_cfg_node_ref xjs_cfg_node_make_arrow_function_new(xjs_cfg_make_arrow_function_ref make_arrow_function);

/* Inspect */
xjs_cfg_inspect_ref xjs_cfg_inspect_new( \
        xjs_cfg_var var);
xjs_cfg_node_ref xjs_cfg_node_inspect_new(xjs_cfg_inspect_ref ret);

/* Return */
xjs_cfg_return_ref xjs_cfg_return_new( \
        xjs_cfg_var var);
xjs_cfg_node_ref xjs_cfg_node_return_new(xjs_cfg_return_ref ret);

/* Call */
xjs_cfg_call_ref xjs_cfg_call_new( \
        xjs_cfg_var dst, \
        xjs_cfg_var callee, \
        xjs_cfg_var_list_ref arguments);
xjs_cfg_call_ref xjs_cfg_call_new_bound_this( \
        xjs_cfg_var dst, \
        xjs_cfg_var callee, \
        xjs_cfg_var_list_ref arguments, \
        xjs_cfg_var _this);
xjs_cfg_node_ref xjs_cfg_node_call_new(xjs_cfg_call_ref call);

/* New */
xjs_cfg_new_ref xjs_cfg_new_new( \
        xjs_cfg_var dst, \
        xjs_cfg_var callee, \
        xjs_cfg_var_list_ref arguments);
xjs_cfg_node_ref xjs_cfg_node_new_new(xjs_cfg_new_ref call);

/* This */
xjs_cfg_this_ref xjs_cfg_this_new(xjs_cfg_var dst);
xjs_cfg_node_ref xjs_cfg_node_this_new(xjs_cfg_this_ref _this);

/* Function */
xjs_cfg_parameter_ref xjs_cfg_parameter_new(ec_string *name);
xjs_cfg_function_ref xjs_cfg_function_new( \
        xjs_cfg_parameter_list_ref parameters, xjs_cfg_node_ref body);

/* Export Symbol */
xjs_cfg_export_symbol_ref xjs_cfg_export_symbol_new( \
        ec_string *exported, ec_string *local);

/* Import Symbol */
xjs_cfg_import_symbol_ref xjs_cfg_import_symbol_new( \
        ec_string *local, ec_string *imported, ec_string *source);

/* CFG */
xjs_cfg_ref xjs_cfg_create(void);
void xjs_cfg_set_top_level( \
        xjs_cfg_ref cfg, xjs_cfg_function_ref top_level);
void xjs_cfg_append_function( \
        xjs_cfg_ref cfg, xjs_cfg_function_ref func);
void xjs_cfg_append_export_symbol( \
        xjs_cfg_ref cfg, xjs_cfg_export_symbol_ref new_export_symbol);
void xjs_cfg_append_import_symbol( \
        xjs_cfg_ref cfg, xjs_cfg_import_symbol_ref new_import_symbol);

void xjs_cfg_manage( \
        xjs_cfg_ref cfg, \
        xjs_cfg_node_ref node);

#endif

