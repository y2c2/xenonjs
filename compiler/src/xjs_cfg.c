/* XenonJS : Control Flow Graph
 * Copyright(c) 2017 y2c2 */

#include <ec_encoding.h>
#include <ec_string.h>
#include "xjs_types.h"
#include "xjs_aux.h"
#include "xjs_helper.h"
#include "xjs_cfg.h"

/* CFG var list */

ect_list_define_declared(xjs_cfg_var_list, xjs_cfg_var, NULL);

/* Loc */

static void xjs_cfg_loc_init(xjs_cfg_loc_ref loc)
{
    loc->start.ln = -1;
    loc->start.col = -1;
    loc->end.ln = -1;
    loc->end.col = -1;
}

static void xjs_cfg_range_init(xjs_cfg_range_ref range)
{
    range->start = -1;
    range->end = -1;
}

/* Context */

static void xjs_cfg_cfgbuilder_context_node_list_node_dtor(xjs_cfg_node_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_cfg_cfgbuilder_context_node_list, \
        xjs_cfg_node_ref, xjs_cfg_cfgbuilder_context_node_list_node_dtor);

/* Comment */
int xjs_cfg_node_comment(xjs_cfg_node_ref node, const char *comment)
{
    ec_string *decoded_comment;
    ec_encoding_t enc;
    ec_encoding_utf8_init(&enc);
    if (ec_encoding_decode(&enc, &decoded_comment, \
                (const ec_byte_t *)comment, strlen(comment)) != 0)
    { return -1; }
    ec_delete(node->comment);
    node->comment = decoded_comment;
    return 0;
}

/* Loc */
void xjs_cfg_node_set_loc_start(xjs_cfg_node_ref node, const int start_ln, const int start_col)
{
    node->loc.start.ln = start_ln;
    node->loc.start.col = start_col;
}

void xjs_cfg_node_set_loc_end(xjs_cfg_node_ref node, const int end_ln, const int end_col)
{
    node->loc.end.ln = end_ln;
    node->loc.end.col = end_col;
}

/* Range */
void xjs_cfg_node_set_range_start(xjs_cfg_node_ref node, const int start)
{
    node->range.start = start;
}

void xjs_cfg_node_set_range_end(xjs_cfg_node_ref node, const int end)
{
    node->range.end = end;
}

/* Literal */

static void xjs_cfg_literal_undefined_ctor(void *data)
{
    xjs_cfg_literal_ref r = data;
    r->type = XJS_CFG_LITERAL_TYPE_UNDEFINED;
}

xjs_cfg_literal_ref xjs_cfg_literal_undefined_new(xjs_cfg_var var)
{
    xjs_cfg_literal_ref r = ec_newcd(xjs_cfg_literal, \
            xjs_cfg_literal_undefined_ctor, NULL);
    r->var = var;
    return r;
}

static void xjs_cfg_literal_null_ctor(void *data)
{
    xjs_cfg_literal_ref r = data;
    r->type = XJS_CFG_LITERAL_TYPE_NULL;
}

xjs_cfg_literal_ref xjs_cfg_literal_null_new(xjs_cfg_var var)
{
    xjs_cfg_literal_ref r = ec_newcd(xjs_cfg_literal, \
            xjs_cfg_literal_null_ctor, NULL);
    r->var = var;
    return r;
}

static void xjs_cfg_literal_false_ctor(void *data)
{
    xjs_cfg_literal_ref r = data;
    r->type = XJS_CFG_LITERAL_TYPE_BOOL;
    r->u.as_bool = ec_false;
}

xjs_cfg_literal_ref xjs_cfg_literal_false_new(xjs_cfg_var var)
{
    xjs_cfg_literal_ref r = ec_newcd(xjs_cfg_literal, \
            xjs_cfg_literal_false_ctor, NULL);
    r->var = var;
    return r;
}

static void xjs_cfg_literal_true_ctor(void *data)
{
    xjs_cfg_literal_ref r = data;
    r->type = XJS_CFG_LITERAL_TYPE_BOOL;
    r->u.as_bool = ec_true;
}

xjs_cfg_literal_ref xjs_cfg_literal_true_new(xjs_cfg_var var)
{
    xjs_cfg_literal_ref r = ec_newcd(xjs_cfg_literal, \
            xjs_cfg_literal_true_ctor, NULL);
    r->var = var;
    return r;
}

static void xjs_cfg_literal_number_ctor(void *data)
{
    xjs_cfg_literal_ref r = data;
    r->type = XJS_CFG_LITERAL_TYPE_NUMBER;
}

xjs_cfg_literal_ref xjs_cfg_literal_number_new(double value, xjs_cfg_var var)
{
    xjs_cfg_literal_ref r = ec_newcd(xjs_cfg_literal, \
            xjs_cfg_literal_number_ctor, NULL);
    r->u.as_number = value;
    r->var = var;
    return r;
}

static void xjs_cfg_literal_string_ctor(void *data)
{
    xjs_cfg_literal_ref r = data;
    r->type = XJS_CFG_LITERAL_TYPE_STRING;
    r->u.as_string = NULL;
}

static void xjs_cfg_literal_string_dtor(void *data)
{
    xjs_cfg_literal_ref r = data;
    r->type = XJS_CFG_LITERAL_TYPE_STRING;
    ec_delete(r->u.as_string);
}

xjs_cfg_literal_ref xjs_cfg_literal_string_new(ec_string *value, xjs_cfg_var var)
{
    xjs_cfg_literal_ref r = ec_newcd(xjs_cfg_literal, \
            xjs_cfg_literal_string_ctor, xjs_cfg_literal_string_dtor);
    r->u.as_string = value;
    r->var = var;
    return r;
}

static void xjs_cfg_literal_object_ctor(void *data)
{
    xjs_cfg_literal_ref r = data;
    r->type = XJS_CFG_LITERAL_TYPE_OBJECT;
}

static void xjs_cfg_literal_object_dtor(void *data)
{
    xjs_cfg_literal_ref r = data;
    (void)r;
}

xjs_cfg_literal_ref xjs_cfg_literal_object_new(xjs_cfg_var var)
{
    xjs_cfg_literal_ref r = ec_newcd(xjs_cfg_literal, \
            xjs_cfg_literal_object_ctor, xjs_cfg_literal_object_dtor);
    r->var = var;
    return r;
}

static void xjs_cfg_literal_array_ctor(void *data)
{
    xjs_cfg_literal_ref r = data;
    r->type = XJS_CFG_LITERAL_TYPE_ARRAY;
}

static void xjs_cfg_literal_array_dtor(void *data)
{
    xjs_cfg_literal_ref r = data;
    (void)r;
}

xjs_cfg_literal_ref xjs_cfg_literal_array_new(xjs_cfg_var var)
{
    xjs_cfg_literal_ref r = ec_newcd(xjs_cfg_literal, \
            xjs_cfg_literal_array_ctor, xjs_cfg_literal_array_dtor);
    r->var = var;
    return r;
}

static void xjs_cfg_node_literal_ctor(void *data)
{ 
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_LITERAL;
    r->u.as_literal = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_literal_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_literal);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_literal_new(xjs_cfg_literal_ref lit)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_literal_ctor, xjs_cfg_node_literal_dtor);
    r->u.as_literal = lit;
    return r;
}

/* Alloca */

static void xjs_cfg_alloca_ctor(void *data)
{
    xjs_cfg_alloca_ref r = data;
    r->var = 0;
}

xjs_cfg_alloca_ref xjs_cfg_alloca_new(xjs_cfg_var var)
{
    xjs_cfg_alloca_ref r = ec_newcd(xjs_cfg_alloca, \
            xjs_cfg_alloca_ctor, NULL);
    r->var = var;
    return r;
}

static void xjs_cfg_node_alloca_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_ALLOCA;
    r->u.as_alloca = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_alloca_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_alloca);
}

xjs_cfg_node_ref xjs_cfg_node_alloca_new(xjs_cfg_alloca_ref alloca)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_alloca_ctor, xjs_cfg_node_alloca_dtor);
    r->u.as_alloca = alloca;
    return r;
}

/* Load from Identifier */

static void xjs_cfg_load_ctor(void *data)
{ xjs_cfg_load_ref r = data; r->name = NULL; }

static void xjs_cfg_load_dtor(void *data)
{ xjs_cfg_load_ref r = data; ec_delete(r->name); }

xjs_cfg_load_ref xjs_cfg_load_new(ec_string *name, xjs_cfg_var var)
{
    xjs_cfg_load_ref r = ec_newcd(xjs_cfg_load, \
            xjs_cfg_load_ctor,xjs_cfg_load_dtor);
    r->name = name;
    r->var = var;
    return r;
}

static void xjs_cfg_node_load_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_LOAD;
    r->u.as_load = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_load_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_load);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_load_new(xjs_cfg_load_ref id)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_load_ctor, xjs_cfg_node_load_dtor);
    r->u.as_load = id;
    return r;
}

/* Unary Operations */ 

static void xjs_cfg_unary_op_ctor(void *data)
{
    xjs_cfg_unary_op_ref r = data;
    r->type = XJS_CFG_UNARY_OP_TYPE_UNKNOWN;
}

xjs_cfg_unary_op_ref xjs_cfg_unary_op_new( \
        xjs_cfg_unary_op_type type, \
        xjs_cfg_var var_dst, xjs_cfg_var var_src)
{
    xjs_cfg_unary_op_ref r = ec_newcd(xjs_cfg_unary_op, \
            xjs_cfg_unary_op_ctor, NULL);
    r->type = type;
    r->var_dst = var_dst;
    r->var_src = var_src;
    return r;
}

static void xjs_cfg_node_unary_op_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_UNARY_OP;
    r->u.as_unary_op = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_unary_op_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_unary_op);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_unary_op_new(xjs_cfg_unary_op_ref unary)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_unary_op_ctor, xjs_cfg_node_unary_op_dtor);
    r->u.as_unary_op = unary;
    return r;
}

/* Binary Operations */ 

static void xjs_cfg_binary_op_ctor(void *data)
{
    xjs_cfg_binary_op_ref r = data;
    r->type = XJS_CFG_BINARY_OP_TYPE_UNKNOWN;
}

xjs_cfg_binary_op_ref xjs_cfg_binary_op_new( \
        xjs_cfg_binary_op_type type, \
        xjs_cfg_var var_dst, xjs_cfg_var var_src1, xjs_cfg_var var_src2)
{
    xjs_cfg_binary_op_ref r = ec_newcd(xjs_cfg_binary_op, \
            xjs_cfg_binary_op_ctor, NULL);
    r->type = type;
    r->var_dst = var_dst;
    r->var_src1 = var_src1;
    r->var_src2 = var_src2;
    return r;
}

static void xjs_cfg_node_binary_op_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_BINARY_OP;
    r->u.as_binary_op = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_binary_op_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_binary_op);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_binary_op_new(xjs_cfg_binary_op_ref binary)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_binary_op_ctor, xjs_cfg_node_binary_op_dtor);
    r->u.as_binary_op = binary;
    return r;
}

/* Drop Value */

static void xjs_cfg_drop_ctor(void *data)
{ xjs_cfg_drop_ref r = data; r->value = NULL; }

static void xjs_cfg_drop_dtor(void *data)
{
    xjs_cfg_drop_ref r = data;
    /* Managed by CFG Builder Context */ 
    (void)(r->value);
}

xjs_cfg_drop_ref xjs_cfg_drop_new(xjs_cfg_node_ref value)
{
    xjs_cfg_drop_ref r = ec_newcd(xjs_cfg_drop, \
            xjs_cfg_drop_ctor, xjs_cfg_drop_dtor);
    r->value = value;
    return r;
}

static void xjs_cfg_node_drop_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_DROP;
    r->u.as_drop = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_drop_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_drop);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_drop_new(xjs_cfg_drop_ref drop)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_drop_ctor, xjs_cfg_node_drop_dtor);
    r->u.as_drop = drop;
    return r;
}

/* Halt */

static void xjs_cfg_node_halt_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_HALT;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_halt_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_halt_new(void)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_halt_ctor, xjs_cfg_node_halt_dtor);
    return r;
}

/* Jump */

static void xjs_cfg_jump_ctor(void *data)
{ xjs_cfg_jump_ref r = data; r->dest = NULL; }

xjs_cfg_jump_ref xjs_cfg_jump_new(void)
{
    xjs_cfg_jump_ref r = ec_newcd(xjs_cfg_jump, \
            xjs_cfg_jump_ctor, NULL);
    return r;
}

void xjs_cfg_jump_dest_set(xjs_cfg_jump_ref jump, xjs_cfg_node_ref dest)
{
    jump->dest = dest;
}

static void xjs_cfg_node_jump_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_JUMP;
    r->u.as_jump = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_jump_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_jump);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_jump_new(xjs_cfg_jump_ref jump)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_jump_ctor, xjs_cfg_node_jump_dtor);
    r->u.as_jump = jump;
    return r;
}

/* Branch */

static void xjs_cfg_branch_ctor(void *data)
{
    xjs_cfg_branch_ref r = data;
    r->cond = 0;
    r->true_branch = NULL;
    r->false_branch = NULL;
}

static void xjs_cfg_branch_dtor(void *data)
{
    /* Managed by CFG Builder Context */ 
    xjs_cfg_branch_ref r = data;
    (void)r;
}

xjs_cfg_branch_ref xjs_cfg_branch_new(void)
{
    xjs_cfg_branch_ref r = ec_newcd(xjs_cfg_branch, \
            xjs_cfg_branch_ctor, xjs_cfg_branch_dtor);
    return r;
}

void xjs_cfg_branch_cond_set(xjs_cfg_branch_ref branch, xjs_cfg_var cond)
{ branch->cond = cond; }

void xjs_cfg_branch_true_branch_set(xjs_cfg_branch_ref branch, xjs_cfg_node_ref true_branch)
{ branch->true_branch = true_branch; }

void xjs_cfg_branch_false_branch_set(xjs_cfg_branch_ref branch, xjs_cfg_node_ref false_branch)
{ branch->false_branch = false_branch; }

static void xjs_cfg_node_branch_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_BRANCH;
    r->u.as_branch = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_branch_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_branch);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_branch_new(xjs_cfg_branch_ref branch)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_branch_ctor, xjs_cfg_node_branch_dtor);
    r->u.as_branch = branch;
    return r;
}

/* Merge */

static void xjs_cfg_merge_ctor(void *data)
{
    xjs_cfg_merge_ref r = data;
    r->test = 0;
    r->consequent = 0;
    r->alternate = 0;
    r->dst = 0;
}

static void xjs_cfg_merge_dtor(void *data)
{
    /* Managed by CFG Builder Context */ 
    xjs_cfg_merge_ref r = data;
    (void)r;
}

xjs_cfg_merge_ref xjs_cfg_merge_new(void)
{
    xjs_cfg_merge_ref r = ec_newcd(xjs_cfg_merge, \
            xjs_cfg_merge_ctor, xjs_cfg_merge_dtor);
    return r;
}

void xjs_cfg_merge_test_set(xjs_cfg_merge_ref merge, xjs_cfg_var test)
{
    merge->test = test;
}

void xjs_cfg_merge_consequent_set(xjs_cfg_merge_ref merge, xjs_cfg_var consequent)
{
    merge->consequent = consequent;
}

void xjs_cfg_merge_alternate_set(xjs_cfg_merge_ref merge, xjs_cfg_var alternate)
{
    merge->alternate = alternate;
}

void xjs_cfg_merge_dst_set(xjs_cfg_merge_ref merge, xjs_cfg_var dst)
{
    merge->dst = dst;
}

static void xjs_cfg_node_merge_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_MERGE;
    r->u.as_merge = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_merge_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_merge);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_merge_new(xjs_cfg_merge_ref merge)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_merge_ctor, xjs_cfg_node_merge_dtor);
    r->u.as_merge = merge;
    return r;
}

/* Block */

static void xjs_cfg_node_list_node_dtor(xjs_cfg_node_ref node)
{
    (void)node;
}

ect_list_define_declared(xjs_cfg_node_list, xjs_cfg_node_ref, xjs_cfg_node_list_node_dtor);

static void xjs_cfg_block_ctor(void *data)
{
    xjs_cfg_block_ref r = data;
    r->items = xjs_cfg_node_list_new();
}

static void xjs_cfg_block_dtor(void *data)
{ xjs_cfg_block_ref r = data; ec_delete(r->items); }

xjs_cfg_block_ref xjs_cfg_block_new(void)
{
    xjs_cfg_block_ref r = ec_newcd(xjs_cfg_block, \
            xjs_cfg_block_ctor, xjs_cfg_block_dtor);
    return r;
}

static void xjs_cfg_node_block_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_BLOCK;
    r->u.as_block = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_block_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_block);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_block_new(xjs_cfg_block_ref block)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_block_ctor, xjs_cfg_node_block_dtor);
    r->u.as_block = block;
    return r;
}

void xjs_cfg_block_push_back(xjs_cfg_block_ref block, xjs_cfg_node_ref sub)
{
    xjs_cfg_node_list_push_back(block->items, sub);
}

/* Declaration Variable */

static void xjs_cfg_declvar_ctor(void *data)
{ xjs_cfg_declvar_ref r = data; r->name = NULL; }

static void xjs_cfg_declvar_dtor(void *data)
{ xjs_cfg_declvar_ref r = data; ec_delete(r->name); }

xjs_cfg_declvar_ref xjs_cfg_declvar_new(ec_string *name)
{
    xjs_cfg_declvar_ref r = ec_newcd(xjs_cfg_declvar, \
            xjs_cfg_declvar_ctor, xjs_cfg_declvar_dtor);
    r->name = name;
    return r;
}

static void xjs_cfg_node_declvar_ctor(void *data)
{
    xjs_cfg_node_ref r = data; r->type = XJS_CFG_NODE_TYPE_DECLVAR; r->u.as_declvar = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_declvar_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_declvar);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_declvar_new(xjs_cfg_declvar_ref declvar)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_declvar_ctor, xjs_cfg_node_declvar_dtor);
    r->u.as_declvar = declvar;
    return r;
}

/* Make Function */

static void xjs_cfg_make_function_ctor(void *data)
{ xjs_cfg_make_function_ref r = data; r->func = NULL; }

static void xjs_cfg_make_function_dtor(void *data)
{ xjs_cfg_make_function_ref r = data; (void)r; }

xjs_cfg_make_function_ref xjs_cfg_make_function_new( \
        xjs_cfg_function_ref func, \
        xjs_cfg_var var_dst)
{
    xjs_cfg_make_function_ref r = ec_newcd(xjs_cfg_make_function, \
            xjs_cfg_make_function_ctor, \
            xjs_cfg_make_function_dtor);
    r->func = func;
    r->var_dst = var_dst;
    return r;
}

static void xjs_cfg_node_make_function_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_MAKE_FUNCTION;
    r->u.as_make_function = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_make_function_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_make_function);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_make_function_new(xjs_cfg_make_function_ref make_function)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_make_function_ctor, xjs_cfg_node_make_function_dtor);
    r->u.as_make_function = make_function;
    return r;
}

/* Make Arrow Function */

static void xjs_cfg_make_arrow_function_ctor(void *data)
{ xjs_cfg_make_arrow_function_ref r = data; r->func = NULL; }

static void xjs_cfg_make_arrow_function_dtor(void *data)
{ xjs_cfg_make_arrow_function_ref r = data; (void)r; }

xjs_cfg_make_arrow_function_ref xjs_cfg_make_arrow_function_new( \
        xjs_cfg_function_ref func, \
        xjs_cfg_var var_dst)
{
    xjs_cfg_make_arrow_function_ref r = ec_newcd(xjs_cfg_make_arrow_function, \
            xjs_cfg_make_arrow_function_ctor, \
            xjs_cfg_make_arrow_function_dtor);
    r->func = func;
    r->var_dst = var_dst;
    return r;
}

static void xjs_cfg_node_make_arrow_function_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_MAKE_ARROW_FUNCTION;
    r->u.as_make_arrow_function = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_make_arrow_function_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_make_arrow_function);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_make_arrow_function_new(xjs_cfg_make_arrow_function_ref make_arrow_function)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_make_arrow_function_ctor, xjs_cfg_node_make_arrow_function_dtor);
    r->u.as_make_arrow_function = make_arrow_function;
    return r;
}

/* Inspect */

static void xjs_cfg_inspect_ctor(void *data)
{ xjs_cfg_inspect_ref r = data; r->var = 0; }

static void xjs_cfg_inspect_dtor(void *data)
{ xjs_cfg_inspect_ref r = data; (void)r; }

xjs_cfg_inspect_ref xjs_cfg_inspect_new( \
        xjs_cfg_var var)
{
    xjs_cfg_inspect_ref r = ec_newcd(xjs_cfg_inspect, \
            xjs_cfg_inspect_ctor, \
            xjs_cfg_inspect_dtor);
    r->var = var;
    return r;
}

static void xjs_cfg_node_inspect_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_INSPECT;
    r->u.as_inspect = NULL;
}

static void xjs_cfg_node_inspect_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_inspect);
}

xjs_cfg_node_ref xjs_cfg_node_inspect_new(xjs_cfg_inspect_ref ret)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_inspect_ctor, xjs_cfg_node_inspect_dtor);
    r->u.as_inspect = ret;
    return r;
}

/* Return */

static void xjs_cfg_return_ctor(void *data)
{ xjs_cfg_return_ref r = data; r->var = 0; }

static void xjs_cfg_return_dtor(void *data)
{ xjs_cfg_return_ref r = data; (void)r; }

xjs_cfg_return_ref xjs_cfg_return_new( \
        xjs_cfg_var var)
{
    xjs_cfg_return_ref r = ec_newcd(xjs_cfg_return, \
            xjs_cfg_return_ctor, \
            xjs_cfg_return_dtor);
    r->var = var;
    return r;
}

static void xjs_cfg_node_return_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_RETURN;
    r->u.as_return = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_return_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_return);
}

xjs_cfg_node_ref xjs_cfg_node_return_new(xjs_cfg_return_ref ret)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_return_ctor, xjs_cfg_node_return_dtor);
    r->u.as_return = ret;
    return r;
}

/* Call */

static void xjs_cfg_call_ctor(void *data)
{ xjs_cfg_call_ref r = data; r->dst = 0; r->callee = 0; r->arguments = NULL; }

static void xjs_cfg_call_dtor(void *data)
{ xjs_cfg_call_ref r = data; ec_delete(r->arguments); }

xjs_cfg_call_ref xjs_cfg_call_new( \
        xjs_cfg_var dst, \
        xjs_cfg_var callee, \
        xjs_cfg_var_list_ref arguments)
{
    xjs_cfg_call_ref r = ec_newcd(xjs_cfg_call, \
            xjs_cfg_call_ctor, xjs_cfg_call_dtor);
    r->dst = dst;
    r->callee = callee;
    r->arguments = arguments;
    r->bound_this.enabled = xjs_false;
    return r;
}

xjs_cfg_call_ref xjs_cfg_call_new_bound_this( \
        xjs_cfg_var dst, \
        xjs_cfg_var callee, \
        xjs_cfg_var_list_ref arguments, \
        xjs_cfg_var _this)
{
    xjs_cfg_call_ref r = ec_newcd(xjs_cfg_call, \
            xjs_cfg_call_ctor, xjs_cfg_call_dtor);
    r->dst = dst;
    r->callee = callee;
    r->arguments = arguments;
    r->bound_this.enabled = xjs_true;
    r->bound_this._this = _this;
    return r;
}

static void xjs_cfg_node_call_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_CALL;
    r->u.as_call = NULL;
}

static void xjs_cfg_node_call_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_call);
}

xjs_cfg_node_ref xjs_cfg_node_call_new(xjs_cfg_call_ref call)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_call_ctor, xjs_cfg_node_call_dtor);
    r->u.as_call = call;
    return r;
}

/* New */

static void xjs_cfg_new_ctor(void *data)
{ xjs_cfg_new_ref r = data; r->dst = 0; r->callee = 0; r->arguments = NULL; }

static void xjs_cfg_new_dtor(void *data)
{ xjs_cfg_new_ref r = data; ec_delete(r->arguments); }

xjs_cfg_new_ref xjs_cfg_new_new( \
        xjs_cfg_var dst, \
        xjs_cfg_var callee, \
        xjs_cfg_var_list_ref arguments)
{
    xjs_cfg_new_ref r = ec_newcd(xjs_cfg_new, \
            xjs_cfg_new_ctor, xjs_cfg_new_dtor);
    r->dst = dst;
    r->callee = callee;
    r->arguments = arguments;
    return r;
}

static void xjs_cfg_node_new_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_NEW;
    r->u.as_new = NULL;
}

static void xjs_cfg_node_new_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_new);
}

xjs_cfg_node_ref xjs_cfg_node_new_new(xjs_cfg_new_ref _new)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_new_ctor, xjs_cfg_node_new_dtor);
    r->u.as_new = _new;
    return r;
}

/* This */

static void xjs_cfg_this_ctor(void *data)
{ xjs_cfg_this_ref r = data; r->dst = 0; }

xjs_cfg_this_ref xjs_cfg_this_new(xjs_cfg_var dst)
{
    xjs_cfg_this_ref r = ec_newcd(xjs_cfg_this, \
            xjs_cfg_this_ctor, NULL);
    r->dst = dst;
    return r;
}

static void xjs_cfg_node_this_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_THIS;
    r->u.as_this = NULL;
}

static void xjs_cfg_node_this_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_this);
}

xjs_cfg_node_ref xjs_cfg_node_this_new(xjs_cfg_this_ref _this)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_this_ctor, xjs_cfg_node_this_dtor);
    r->u.as_this = _this;
    return r;
}

/* Assign Value to Variable */

static void xjs_cfg_store_ctor(void *data)
{ xjs_cfg_store_ref r = data; r->name = NULL; }

static void xjs_cfg_store_dtor(void *data)
{ xjs_cfg_store_ref r = data; ec_delete(r->name); }

xjs_cfg_store_ref xjs_cfg_store_new(ec_string *name, xjs_cfg_var var)
{
    xjs_cfg_store_ref r = ec_newcd(xjs_cfg_store, \
            xjs_cfg_store_ctor, xjs_cfg_store_dtor);
    r->name = name;
    r->var = var;
    return r;
}

static void xjs_cfg_node_store_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_STORE;
    r->u.as_store = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_store_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_store);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_store_new(xjs_cfg_store_ref store1)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_store_ctor, xjs_cfg_node_store_dtor);
    r->u.as_store = store1;
    return r;
}

/* Object Set */

static void xjs_cfg_object_set_ctor(void *data)
{ xjs_cfg_object_set_ref r = data; r->key = 0; r->obj = 0; r->value = 0; }

static void xjs_cfg_object_set_dtor(void *data)
{ xjs_cfg_object_set_ref r = data; (void)r; }

xjs_cfg_object_set_ref xjs_cfg_object_set_new( \
        xjs_cfg_var dst, xjs_cfg_var obj, xjs_cfg_var key, xjs_cfg_var value)
{
    xjs_cfg_object_set_ref r = ec_newcd(xjs_cfg_object_set, \
            xjs_cfg_object_set_ctor, xjs_cfg_object_set_dtor);
    r->dst = dst;
    r->obj = obj;
    r->key = key;
    r->value = value;
    return r;
}

static void xjs_cfg_node_object_set_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_OBJECT_SET;
    r->u.as_object_set = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_object_set_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_object_set);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_object_set_new(xjs_cfg_object_set_ref object_set)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_object_set_ctor, xjs_cfg_node_object_set_dtor);
    r->u.as_object_set = object_set;
    return r;
}

/* Object Get */

static void xjs_cfg_object_get_ctor(void *data)
{ xjs_cfg_object_get_ref r = data; r->dst = 0; r->obj = 0; r->key = 0; }

static void xjs_cfg_object_get_dtor(void *data)
{ xjs_cfg_object_set_ref r = data; (void)r; }

xjs_cfg_object_get_ref xjs_cfg_object_get_new( \
        xjs_cfg_var dst, xjs_cfg_var obj, xjs_cfg_var key)
{
    xjs_cfg_object_get_ref r = ec_newcd(xjs_cfg_object_get, \
            xjs_cfg_object_get_ctor, xjs_cfg_object_get_dtor);
    r->dst = dst; r->obj = obj; r->key = key;
    return r;
}

static void xjs_cfg_node_object_get_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_OBJECT_GET;
    r->u.as_object_set = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_object_get_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_object_set);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_object_get_new(xjs_cfg_object_get_ref object_get)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_object_get_ctor, xjs_cfg_node_object_get_dtor);
    r->u.as_object_get = object_get;
    return r;
}

/* Array Push */

static void xjs_cfg_array_push_ctor(void *data)
{ xjs_cfg_array_push_ref r = data; r->arr = 0; r->elem = 0; }

static void xjs_cfg_array_push_dtor(void *data)
{ xjs_cfg_object_set_ref r = data; (void)r; }

xjs_cfg_array_push_ref xjs_cfg_array_push_new( \
        xjs_cfg_var arr, xjs_cfg_var elem)
{
    xjs_cfg_array_push_ref r = ec_newcd(xjs_cfg_array_push, \
            xjs_cfg_array_push_ctor, xjs_cfg_array_push_dtor);
    r->arr = arr; r->elem = elem;
    return r;
}

static void xjs_cfg_node_array_push_ctor(void *data)
{
    xjs_cfg_node_ref r = data;
    r->type = XJS_CFG_NODE_TYPE_ARRAY_PUSH;
    r->u.as_array_push = NULL;
    r->comment = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_node_array_push_dtor(void *data)
{
    xjs_cfg_node_ref r = data;
    ec_delete(r->u.as_array_push);
    ec_delete(r->comment);
}

xjs_cfg_node_ref xjs_cfg_node_array_push_new(xjs_cfg_array_push_ref array_push)
{
    xjs_cfg_node_ref r = ec_newcd(xjs_cfg_node, \
            xjs_cfg_node_array_push_ctor, xjs_cfg_node_array_push_dtor);
    r->u.as_array_push = array_push;
    return r;
}

/* Function */

static void xjs_cfg_parameter_ctor(void *data)
{
    xjs_cfg_parameter_ref r = data;
    r->name = NULL;
    xjs_cfg_loc_init(&r->loc);
    xjs_cfg_range_init(&r->range);
}

static void xjs_cfg_parameter_dtor(void *data)
{
    xjs_cfg_parameter_ref r = data;
    ec_delete(r->name);
}

xjs_cfg_parameter_ref xjs_cfg_parameter_new(ec_string *name)
{
    xjs_cfg_parameter_ref r = ec_newcd(xjs_cfg_parameter, \
            xjs_cfg_parameter_ctor, xjs_cfg_parameter_dtor);
    r->name = name;
    return r;
}

static void xjs_cfg_parameter_list_node_dtor(xjs_cfg_parameter_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_cfg_parameter_list, xjs_cfg_parameter_ref, xjs_cfg_parameter_list_node_dtor);

static void xjs_cfg_function_ctor(void *data)
{
    xjs_cfg_function_ref r = data;
    r->parameters = NULL;
    r->body = NULL;
}

static void xjs_cfg_function_dtor(void *data)
{
    xjs_cfg_function_ref r = data;
    ec_delete(r->parameters);
    /* Managed by CFG Builder Context */ 
    (void)(r->body);
}

xjs_cfg_function_ref xjs_cfg_function_new( \
        xjs_cfg_parameter_list_ref parameters, xjs_cfg_node_ref body)
{
    xjs_cfg_function_ref r = ec_newcd(xjs_cfg_function, xjs_cfg_function_ctor, xjs_cfg_function_dtor);
    r->exporting.type = xjs_cfg_function_export_type_none;
    r->parameters = parameters;
    r->body = body;
    return r;
}

static void xjs_cfg_function_list_node_dtor(xjs_cfg_function_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_cfg_function_list, xjs_cfg_function_ref, xjs_cfg_function_list_node_dtor);

/* Export Symbol */

static void xjs_cfg_export_symbol_ctor(void *data)
{
    xjs_cfg_export_symbol_ref r = data;
    r->exported = NULL;
    r->local = NULL;
}

static void xjs_cfg_export_symbol_dtor(void *data)
{
    xjs_cfg_export_symbol_ref r = data;
    ec_delete(r->exported);
    ec_delete(r->local);
}

xjs_cfg_export_symbol_ref xjs_cfg_export_symbol_new( \
        ec_string *exported, ec_string *local)
{
    xjs_cfg_export_symbol_ref r = ec_newcd(xjs_cfg_export_symbol, \
            xjs_cfg_export_symbol_ctor,  
            xjs_cfg_export_symbol_dtor);
    r->exported = exported;
    r->local = local;
    return r;
}

static void xjs_cfg_export_symbol_list_node_dtor(xjs_cfg_export_symbol_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_cfg_export_symbol_list, \
        xjs_cfg_export_symbol_ref, xjs_cfg_export_symbol_list_node_dtor);

/* Import Symbol */

static void xjs_cfg_import_symbol_ctor(void *data)
{
    xjs_cfg_import_symbol_ref r = data;
    r->imported = NULL;
    r->local = NULL;
    r->source = NULL;
}

static void xjs_cfg_import_symbol_dtor(void *data)
{
    xjs_cfg_import_symbol_ref r = data;
    ec_delete(r->imported);
    ec_delete(r->local);
    ec_delete(r->source);
}

xjs_cfg_import_symbol_ref xjs_cfg_import_symbol_new( \
        ec_string *local, ec_string *imported, ec_string *source)
{
    xjs_cfg_import_symbol_ref r = ec_newcd(xjs_cfg_import_symbol, \
            xjs_cfg_import_symbol_ctor,  
            xjs_cfg_import_symbol_dtor);
    r->local = local;
    r->imported = imported;
    r->source = source;
    return r;
}

static void xjs_cfg_import_symbol_list_node_dtor(xjs_cfg_import_symbol_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_cfg_import_symbol_list, \
        xjs_cfg_import_symbol_ref, xjs_cfg_import_symbol_list_node_dtor);

/* CFG */

static void xjs_cfg_ctor(void *data)
{
    xjs_cfg_ref r = data;
    r->allocated_nodes = xjs_cfg_cfgbuilder_context_node_list_new();
    r->functions = xjs_cfg_function_list_new();
    r->top_level = NULL;
    r->export_symbols = xjs_cfg_export_symbol_list_new();
    r->import_symbols = xjs_cfg_import_symbol_list_new();
}

static void xjs_cfg_dtor(void *data)
{
    xjs_cfg_ref r = data;
    ec_delete(r->allocated_nodes);
    ec_delete(r->functions);
    ec_delete(r->export_symbols);
    ec_delete(r->import_symbols);
}

xjs_cfg_ref xjs_cfg_create(void)
{
    xjs_cfg_ref r = ec_newcd(xjs_cfg, xjs_cfg_ctor, xjs_cfg_dtor);
    return r;
}

void xjs_cfg_set_top_level( \
        xjs_cfg_ref cfg, xjs_cfg_function_ref top_level)
{
    cfg->top_level = top_level;
}

void xjs_cfg_append_function( \
        xjs_cfg_ref cfg, xjs_cfg_function_ref func)
{
    xjs_cfg_function_list_push_back(cfg->functions, func);
}

void xjs_cfg_append_export_symbol( \
        xjs_cfg_ref cfg, xjs_cfg_export_symbol_ref new_export_symbol)
{
    xjs_cfg_export_symbol_list_push_back(cfg->export_symbols, new_export_symbol);
}

void xjs_cfg_append_import_symbol( \
        xjs_cfg_ref cfg, xjs_cfg_import_symbol_ref new_import_symbol)
{
    xjs_cfg_import_symbol_list_push_back(cfg->import_symbols, new_import_symbol);
}

void xjs_cfg_manage( \
        xjs_cfg_ref cfg, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_cfgbuilder_context_node_list_push_back(cfg->allocated_nodes, node);
}

