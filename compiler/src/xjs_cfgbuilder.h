/* XenonJS : IR Builder
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_CFGBUILDER_H
#define XJS_CFGBUILDER_H

#include "xjs_types.h"

struct xjs_opaque_cfgbuilder;
typedef struct xjs_opaque_cfgbuilder xjs_cfgbuilder;
typedef struct xjs_opaque_cfgbuilder *xjs_cfgbuilder_ref;

/* Allocate a variable */
xjs_cfg_var xjs_cfgbuilder_allocate_var(xjs_cfgbuilder_ref cfgb);

/* Create a new builder */
xjs_cfgbuilder_ref xjs_cfgbuilder_new(void);

/* Literal */
xjs_cfg_node_ref xjs_cfgbuilder_literal_undefined_new( \
        xjs_cfgbuilder_ref cfgb, xjs_cfg_var var);
xjs_cfg_node_ref xjs_cfgbuilder_literal_null_new( \
        xjs_cfgbuilder_ref cfgb, xjs_cfg_var var);
xjs_cfg_node_ref xjs_cfgbuilder_literal_false_new( \
        xjs_cfgbuilder_ref cfgb, xjs_cfg_var var);
xjs_cfg_node_ref xjs_cfgbuilder_literal_true_new( \
        xjs_cfgbuilder_ref cfgb, xjs_cfg_var var);
xjs_cfg_node_ref xjs_cfgbuilder_literal_number_new( \
        xjs_cfgbuilder_ref cfgb, double value, xjs_cfg_var var);
xjs_cfg_node_ref xjs_cfgbuilder_literal_string_new( \
        xjs_cfgbuilder_ref cfgb, ec_string *value, xjs_cfg_var var);
xjs_cfg_node_ref xjs_cfgbuilder_literal_object_new( \
        xjs_cfgbuilder_ref cfgb, xjs_cfg_var var);
xjs_cfg_node_ref xjs_cfgbuilder_literal_array_new( \
        xjs_cfgbuilder_ref cfgb, xjs_cfg_var var);

/* Alloca */
xjs_cfg_node_ref xjs_cfgbuilder_alloca_new( \
        xjs_cfgbuilder_ref cfgb, xjs_cfg_var var);

/* Load */
xjs_cfg_node_ref xjs_cfgbuilder_load_new( \
        xjs_cfgbuilder_ref cfgb, \
        ec_string *name, xjs_cfg_var var);

/* Store */
xjs_cfg_node_ref xjs_cfgbuilder_store_new( \
        xjs_cfgbuilder_ref cfgb, \
        ec_string *name, xjs_cfg_var var);

/* Object Set */
xjs_cfg_node_ref xjs_cfgbuilder_object_set_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var dst, xjs_cfg_var obj, xjs_cfg_var key, xjs_cfg_var src);

/* Object Get */
xjs_cfg_node_ref xjs_cfgbuilder_object_get_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var dst, xjs_cfg_var obj, xjs_cfg_var key);

/* Array Push */
xjs_cfg_node_ref xjs_cfgbuilder_array_push_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var arr, xjs_cfg_var elem);

/* Unary Operations */
xjs_cfg_node_ref xjs_cfgbuilder_unary_op_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_unary_op_type type, \
        xjs_cfg_var var_dst, xjs_cfg_var var_src);

/* Binary Operations */
xjs_cfg_node_ref xjs_cfgbuilder_binary_op_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_binary_op_type type, \
        xjs_cfg_var var_dst, \
        xjs_cfg_var var_src1, xjs_cfg_var var_src2);

/* Drop */
xjs_cfg_node_ref xjs_cfgbuilder_drop_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_node_ref value);

/* Halt */
xjs_cfg_node_ref xjs_cfgbuilder_halt_new( \
        xjs_cfgbuilder_ref cfgb);

/* Jump */
xjs_cfg_node_ref xjs_cfgbuilder_jump_new( \
        xjs_cfgbuilder_ref cfgb);
void xjs_cfgbuilder_jump_dest_set( \
        xjs_cfg_node_ref jump, xjs_cfg_node_ref dest);

/* Branch */
xjs_cfg_node_ref xjs_cfgbuilder_branch_new( \
        xjs_cfgbuilder_ref cfgb);
void xjs_cfgbuilder_branch_cond_set( \
        xjs_cfg_node_ref branch, xjs_cfg_var cond);
void xjs_cfgbuilder_branch_true_branch_set( \
        xjs_cfg_node_ref branch, xjs_cfg_node_ref true_branch);
void xjs_cfgbuilder_branch_false_branch_set( \
        xjs_cfg_node_ref branch, xjs_cfg_node_ref false_branch);

/* Merge */
xjs_cfg_node_ref xjs_cfgbuilder_merge_new( \
        xjs_cfgbuilder_ref cfgb);
void xjs_cfgbuilder_merge_test_set( \
        xjs_cfg_node_ref branch, xjs_cfg_var test);
void xjs_cfgbuilder_merge_consequent_set( \
        xjs_cfg_node_ref branch, xjs_cfg_var consequent);
void xjs_cfgbuilder_merge_alternate_set( \
        xjs_cfg_node_ref branch, xjs_cfg_var alternate);
void xjs_cfgbuilder_merge_dst_set( \
        xjs_cfg_node_ref branch, xjs_cfg_var dst);

/* Block */
xjs_cfg_node_ref xjs_cfgbuilder_block_new( \
        xjs_cfgbuilder_ref cfgb);
void xjs_cfgbuilder_block_push_back( \
        xjs_cfg_node_ref block, xjs_cfg_node_ref sub);

/* DeclVar */
xjs_cfg_node_ref xjs_cfgbuilder_declvar_new( \
        xjs_cfgbuilder_ref cfgb, \
        ec_string *name);

/* Make Function */
xjs_cfg_node_ref xjs_cfgbuilder_make_function_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_function_ref func, \
        xjs_cfg_var var_dst);

/* Make Arrow Function */
xjs_cfg_node_ref xjs_cfgbuilder_make_arrow_function_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_function_ref func, \
        xjs_cfg_var var_dst);

/* Inspect */
xjs_cfg_node_ref xjs_cfgbuilder_inspect_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var var);

/* Return */
xjs_cfg_node_ref xjs_cfgbuilder_return_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var var);

/* Call */
xjs_cfg_node_ref xjs_cfgbuilder_call_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var dst, \
        xjs_cfg_var callee, \
        xjs_cfg_var_list_ref arguments);

xjs_cfg_node_ref xjs_cfgbuilder_call_bound_this_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var dst, \
        xjs_cfg_var callee, \
        xjs_cfg_var_list_ref arguments, \
        xjs_cfg_var _this);

/* New */
xjs_cfg_node_ref xjs_cfgbuilder_new_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var dst, \
        xjs_cfg_var callee, \
        xjs_cfg_var_list_ref arguments);

/* This */
xjs_cfg_node_ref xjs_cfgbuilder_this_new( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_var dst);

/* Function */
xjs_cfg_parameter_ref xjs_cfgbuilder_parameter_new(ec_string *name);
xjs_cfg_function_ref xjs_cfgbuilder_function_new( \
        xjs_cfg_parameter_list_ref parameters, xjs_cfg_node_ref body);

/* Export Symbol */
xjs_cfg_export_symbol_ref xjs_cfgbuilder_export_symbol_new( \
        ec_string *exported, ec_string *local);

/* Import Symbol */
xjs_cfg_import_symbol_ref xjs_cfgbuilder_import_symbol_new( \
        ec_string *local, ec_string *imported, ec_string *source);

/* Link */
xjs_cfg_node_ref xjs_cfgbuilder_link( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_node_ref src, xjs_cfg_node_ref dest);

/* IR */
void xjs_cfgbuilder_set_top_level( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_function_ref top_level);
void xjs_cfgbuilder_append_function( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_function_ref func);
void xjs_cfgbuilder_append_export_symbol( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_export_symbol_ref new_symbol);
void xjs_cfgbuilder_append_import_symbol( \
        xjs_cfgbuilder_ref cfgb, \
        xjs_cfg_import_symbol_ref new_symbol);

/* Generate IR */
xjs_cfg_ref xjs_cfgbuilder_cfg_generate( \
        xjs_cfgbuilder_ref cfgb);


#endif

