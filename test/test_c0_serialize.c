#include <ec_encoding.h>
#include <ec_string.h>
#include <ec_map.h>
#include <ec_set.h>
#include <ec_list.h>
#include <ec_stack.h>
#include <ec_algorithm.h>
#include "xjs.h"
#include "test_serialize_helper.h"
#include "test_c0_serialize.h"

ect_map_declare(xjs_cfg_node_map, xjs_cfg_node_ref, int);
ect_map_declare(xjs_cfg_function_map, xjs_cfg_function_ref, int);

/* static void xjs_cfg_node_list_node_dtor(xjs_cfg_node_ref node) */
/* { */
/*     (void)node; */
/* } */

/* ect_list_define_declared(xjs_cfg_node_list, xjs_cfg_node_ref, xjs_cfg_node_list_node_dtor); */
ect_stack_declare(xjs_cfg_node_stack, xjs_cfg_node_ref, xjs_cfg_node_list);
/* ect_stack_define(xjs_cfg_node_stack, xjs_cfg_node_ref, xjs_cfg_node_list); */

ect_set_declare(xjs_visited_node_set, xjs_cfg_node_ref);

static int xjs_cfg_node_set_node_ctor(xjs_cfg_node_ref *detta_key, xjs_cfg_node_ref *key)
{ *detta_key = *key; return 0; }

static void xjs_cfg_node_set_node_dtor(xjs_cfg_node_ref *detta_key)
{ (void)detta_key; }

static int xjs_cfg_node_set_node_cmp(xjs_cfg_node_ref *a, xjs_cfg_node_ref *b)
{
    if (*a > *b) return 1;
    else if (*a < *b) return -1;
    return 0;
}

ect_set_define_declared(xjs_visited_node_set, xjs_cfg_node_ref, \
        xjs_cfg_node_set_node_ctor, \
        xjs_cfg_node_set_node_dtor, \
        xjs_cfg_node_set_node_cmp);

static int xjs_cfg_node_map_key_ctor(xjs_cfg_node_ref *detta_key, xjs_cfg_node_ref *key)
{
    *detta_key = *key;
    return 0;
}

static void xjs_cfg_node_map_key_dtor(xjs_cfg_node_ref *key)
{
    (void)key;
}

static int xjs_cfg_node_map_key_cmp(xjs_cfg_node_ref *a, xjs_cfg_node_ref *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

static int xjs_cfg_node_map_value_ctor(int *detta_value, int *value)
{
    *detta_value = *value;
    return 0;
}

ect_map_define_declared(xjs_cfg_node_map, \
        xjs_cfg_node_ref, xjs_cfg_node_map_key_ctor, xjs_cfg_node_map_key_dtor, \
        int, xjs_cfg_node_map_value_ctor, NULL, \
        xjs_cfg_node_map_key_cmp);
typedef xjs_cfg_node_map *xjs_cfg_node_map_ref;

static int xjs_cfg_function_map_value_ctor(int *detta_value, int *value)
{
    *detta_value = *value;
    return 0;
}

static int xjs_cfg_function_map_key_ctor(xjs_cfg_function_ref *detta_key, xjs_cfg_function_ref *key)
{
    *detta_key = *key;
    return 0;
}

static void xjs_cfg_function_map_key_dtor(xjs_cfg_function_ref *key)
{
    (void)key;
}

static int xjs_cfg_function_map_key_cmp(xjs_cfg_function_ref *a, xjs_cfg_function_ref *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

ect_map_define_declared(xjs_cfg_function_map, \
        xjs_cfg_function_ref, xjs_cfg_function_map_key_ctor, xjs_cfg_function_map_key_dtor, \
        int, xjs_cfg_function_map_value_ctor, NULL, \
        xjs_cfg_function_map_key_cmp);
typedef xjs_cfg_function_map *xjs_cfg_function_map_ref;

struct xjs_opaque_cfg_serialize_ctx 
{
    xjs_cfg_node_map_ref nodes;
    xjs_cfg_function_map_ref functions;
    ec_string *result;
    int node_index;
    ec_bool first;
};
typedef struct xjs_opaque_cfg_serialize_ctx xjs_cfg_serialize_ctx;
typedef struct xjs_opaque_cfg_serialize_ctx *xjs_cfg_serialize_ctx_ref;

#define SERIALIZE_APPEND_INT(_result, _x) \
    do { char index_buf[10 + 1]; \
      snprintf(index_buf, 10, "%d", _x); \
        ec_string_append_c_str(_result, index_buf); } while (0)


#define SERIALIZE_HEADER(idx) \
    do { \
    if (ctx->first == ec_false) ec_string_append_c_str(ctx->result, ","); else ctx->first = ec_false; \
    ec_string_append_c_str(ctx->result, "{\"index\":"); \
    { char index_buf[10 + 1]; \
      snprintf(index_buf, 10, "%d", idx); \
        ec_string_append_c_str(ctx->result, index_buf); } } while (0)

#define SERIALIZE_FOOTER() \
    do { ec_string_append_c_str(ctx->result, "}"); } while (0)

/* static int xjs_cfg_serialize_node( \ */
/*         xjs_cfg_serialize_ctx_ref ctx, \ */
/*         xjs_cfg_node_ref node); */

static int xjs_cfg_serialize_append_unescaped_json_string( \
        ec_string *result, ec_string *value_string)
{
    ect_iterator(ec_string) it;
    ec_char_t ch;

    ect_for(ec_string, value_string, it)
    {
        ch = ect_deref(ec_char_t , it);
        if (ch == '"')
        {
            ec_string_push_back(result, '\\');
            ec_string_push_back(result, '\"');
        }
        else if (ch == '\\')
        {
            ec_string_push_back(result, '\\');
            ec_string_push_back(result, '\\');
        }
        else
        {
            ec_string_push_back(result, ch);
        }
    }

    return 0;
}

static int xjs_cfg_serialize_jump( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_jump_ref jump = node->u.as_jump;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    ec_string_append_c_str(ctx->result, ",\"type\":\"Jump\",\"dest\":");
    {
        int index = xjs_cfg_node_map_get(ctx->nodes, jump->dest);
        SERIALIZE_APPEND_INT(ctx->result, index);
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_halt( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    ec_string_append_c_str(ctx->result, ",\"type\":\"Halt\"");
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_block( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node, \
        ec_string *comment)
{
    xjs_cfg_block_ref block = node->u.as_block;
    xjs_cfg_node_list_ref items = block->items;
    ect_iterator(xjs_cfg_node_list) it;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    ec_string_append_c_str(ctx->result, ",\"type\":\"Block\",\"items\":[");
    {
        ec_bool first = ec_true;
        ect_for(xjs_cfg_node_list, items, it)
        {
            if (first) { first = ec_false;} else ec_string_append_c_str(ctx->result, ",");
            xjs_cfg_node_ref node_child = ect_deref(xjs_cfg_node_ref, it);
            {
                int index = xjs_cfg_node_map_get(ctx->nodes, node_child);
                SERIALIZE_APPEND_INT(ctx->result, index);
            }
        }
    }
    ec_string_append_c_str(ctx->result, "]");
    if (comment != NULL)
    {
        ec_string_append_c_str(ctx->result, ",\"comment\":\"");
        ec_string_append(ctx->result, comment);
        ec_string_append_c_str(ctx->result, "\"");
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_declvar( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_declvar_ref declvar = node->u.as_declvar;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    {
        ec_string_append_c_str(ctx->result, ",\"type\":\"DeclVar\",\"name\":\"");
        ec_string_append(ctx->result, declvar->name);
        ec_string_append_c_str(ctx->result, "\"");
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_store( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_store_ref store = node->u.as_store;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    {
        ec_string_append_c_str(ctx->result, ",\"type\":\"Store\",\"name\":\"");
        ec_string_append(ctx->result, store->name);
        ec_string_append_c_str(ctx->result, "\",\"var\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)store->var);
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_object_set( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_object_set_ref objset = node->u.as_object_set;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    {
        ec_string_append_c_str(ctx->result, ",\"type\":\"ObjectSet\",\"obj\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)objset->obj);
        ec_string_append_c_str(ctx->result, ",\"key\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)objset->key);
        ec_string_append_c_str(ctx->result, ",\"value\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)objset->value);
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_object_get( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_object_get_ref objget = node->u.as_object_get;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    {
        ec_string_append_c_str(ctx->result, ",\"type\":\"ObjectGet\",\"dst\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)objget->dst);
        ec_string_append_c_str(ctx->result, ",\"obj\":\"");
        SERIALIZE_APPEND_INT(ctx->result, (int)objget->obj);
        ec_string_append_c_str(ctx->result, ",\"key\":\"");
        SERIALIZE_APPEND_INT(ctx->result, (int)objget->key);
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_array_push( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_array_push_ref array_push = node->u.as_array_push;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    {
        ec_string_append_c_str(ctx->result, ",\"type\":\"ArrayPush\",\"array\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)array_push->arr);
        ec_string_append_c_str(ctx->result, ",\"element\":\"");
        SERIALIZE_APPEND_INT(ctx->result, (int)array_push->elem);
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_branch( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_branch_ref branch = node->u.as_branch;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    {
        ec_string_append_c_str(ctx->result, ",\"type\":\"Branch\",\"cond\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)branch->cond);
        ec_string_append_c_str(ctx->result, ",\"consequent\":");
        SERIALIZE_APPEND_INT(ctx->result, xjs_cfg_node_map_get(ctx->nodes, branch->true_branch));
        ec_string_append_c_str(ctx->result, ",\"alternate\":");
        SERIALIZE_APPEND_INT(ctx->result, xjs_cfg_node_map_get(ctx->nodes, branch->false_branch));
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_merge( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_merge_ref merge = node->u.as_merge;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    {
        ec_string_append_c_str(ctx->result, ",\"type\":\"Merge\",\"test\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)merge->test);
        ec_string_append_c_str(ctx->result, ",\"consequent\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)merge->consequent);
        ec_string_append_c_str(ctx->result, ",\"alternate\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)merge->alternate);
        ec_string_append_c_str(ctx->result, ",\"dst\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)merge->dst);
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_make_function( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_make_function_ref make_function = node->u.as_make_function;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    {
        ec_string_append_c_str(ctx->result, ",\"type\":\"MakeFunction\",\"var\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)make_function->var_dst);
        ec_string_append_c_str(ctx->result, ",\"function\":");
        SERIALIZE_APPEND_INT(ctx->result, \
                xjs_cfg_function_map_get(ctx->functions, make_function->func));
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_make_arrow_function( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_make_function_ref make_function = node->u.as_make_function;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    {
        ec_string_append_c_str(ctx->result, ",\"type\":\"MakeArrowFunction\",\"var\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)make_function->var_dst);
        ec_string_append_c_str(ctx->result, ",\"function\":");
        SERIALIZE_APPEND_INT(ctx->result, \
                xjs_cfg_function_map_get(ctx->functions, make_function->func));
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_inspect( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_inspect_ref ret = node->u.as_inspect;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    {
        ec_string_append_c_str(ctx->result, ",\"type\":\"Inspect\",\"var\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)ret->var);
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_return( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_return_ref ret = node->u.as_return;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    {
        ec_string_append_c_str(ctx->result, ",\"type\":\"Return\",\"var\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)ret->var);
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_call( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_call_ref call = node->u.as_call;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    {
        ec_string_append_c_str(ctx->result, ",\"type\":\"Call\",\"dst\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)call->dst);
        ec_string_append_c_str(ctx->result, ",\"callee\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)call->callee);
        ec_string_append_c_str(ctx->result, ",\"arguments\":[");
        {
            ec_bool first = ec_true;
            ect_iterator(xjs_cfg_var_list) it;
            ect_for(xjs_cfg_var_list, call->arguments, it)
            {
                xjs_cfg_var var_arg = ect_deref(xjs_cfg_var, it);
                if (first) first = ec_false; else ec_string_append_c_str(ctx->result, ",");
                SERIALIZE_APPEND_INT(ctx->result, (int)var_arg);
            }
        }
        ec_string_append_c_str(ctx->result, "]");
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_new( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_new_ref _new = node->u.as_new;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    {
        ec_string_append_c_str(ctx->result, ",\"type\":\"New\",\"dst\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)_new->dst);
        ec_string_append_c_str(ctx->result, ",\"callee\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)_new->callee);
        ec_string_append_c_str(ctx->result, ",\"arguments\":[");
        {
            ec_bool first = ec_true;
            ect_iterator(xjs_cfg_var_list) it;
            ect_for(xjs_cfg_var_list, _new->arguments, it)
            {
                xjs_cfg_var var_arg = ect_deref(xjs_cfg_var, it);
                if (first) first = ec_false; else ec_string_append_c_str(ctx->result, ",");
                SERIALIZE_APPEND_INT(ctx->result, (int)var_arg);
            }
        }
        ec_string_append_c_str(ctx->result, "]");
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_this( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_this_ref _this = node->u.as_this;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    {
        ec_string_append_c_str(ctx->result, ",\"type\":\"This\",\"dst\":");
        SERIALIZE_APPEND_INT(ctx->result, (int)_this->dst);
    }
    SERIALIZE_FOOTER();

    return 0;
}

static int xjs_cfg_serialize_literal( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_literal_ref literal = node->u.as_literal;
    int ret = 0;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    switch (literal->type)
    {
        case XJS_CFG_LITERAL_TYPE_UNKNOWN:
            ret = -1;
            break;
        case XJS_CFG_LITERAL_TYPE_UNDEFINED:
            ec_string_append_c_str(ctx->result, ",\"type\":\"Undefined\"");
            break;
        case XJS_CFG_LITERAL_TYPE_NULL:
            ec_string_append_c_str(ctx->result, ",\"type\":\"Null\"");
            break;
        case XJS_CFG_LITERAL_TYPE_BOOL:
            if (literal->u.as_bool == ec_false)
            { ec_string_append_c_str(ctx->result, ",\"type\":\"Boolean\",\"value\":false"); }
            else
            { ec_string_append_c_str(ctx->result, ",\"type\":\"Boolean\",\"value\":true"); }
            break;
        case XJS_CFG_LITERAL_TYPE_NUMBER:
            ec_string_append_c_str(ctx->result, ",\"type\":\"Number\",\"value\":");
            xjs_serialize_append_number(ctx->result, literal->u.as_number);
            break;
        case XJS_CFG_LITERAL_TYPE_STRING:
            ec_string_append_c_str(ctx->result, ",\"type\":\"String\",\"value\":\"");
            xjs_cfg_serialize_append_unescaped_json_string(ctx->result, literal->u.as_string);
            ec_string_append_c_str(ctx->result, "\"");
            break;
        case XJS_CFG_LITERAL_TYPE_OBJECT:
            ec_string_append_c_str(ctx->result, ",\"type\":\"Object\"");
            break;
        case XJS_CFG_LITERAL_TYPE_ARRAY:
            ec_string_append_c_str(ctx->result, ",\"type\":\"Array\"");
            break;
    }
    ec_string_append_c_str(ctx->result, ",\"var\":");
    SERIALIZE_APPEND_INT(ctx->result, (int)literal->var);
    SERIALIZE_FOOTER();

    return ret;
}

static int xjs_cfg_serialize_alloca( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_alloca_ref alloca = node->u.as_alloca;
    int ret = 0;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));
    ec_string_append_c_str(ctx->result, ",\"type\":\"Alloca\"");
    ec_string_append_c_str(ctx->result, ",\"var\":");
    SERIALIZE_APPEND_INT(ctx->result, (int)(alloca->var));
    SERIALIZE_FOOTER();

    return ret;
}

static int xjs_cfg_serialize_load( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node) 
{
    xjs_cfg_load_ref load = node->u.as_load;
    int ret = 0;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));

    ec_string_append_c_str(ctx->result, ",\"type\":\"Load\",\"name\":\"");
    xjs_cfg_serialize_append_unescaped_json_string(ctx->result, load->name);
    ec_string_append_c_str(ctx->result, "\"");
    ec_string_append_c_str(ctx->result, ",\"var\":");
    SERIALIZE_APPEND_INT(ctx->result, (int)(load->var));

    SERIALIZE_FOOTER();

    return ret;
}

static int xjs_cfg_serialize_unary_op( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_unary_op_ref unary_op = node->u.as_unary_op;
    int ret = 0;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));

    ec_string_append_c_str(ctx->result, ",\"type\":\"UnaryOp\",\"op\":");
    switch (unary_op->type)
    {
        case XJS_CFG_UNARY_OP_TYPE_UNKNOWN:
            ret = -1;
            break;

        case XJS_CFG_UNARY_OP_TYPE_NOT:
            ec_string_append_c_str(ctx->result, "\"!\"");
            break;
        case XJS_CFG_UNARY_OP_TYPE_BNOT:
            ec_string_append_c_str(ctx->result, "\"~\"");
            break;
        case XJS_CFG_UNARY_OP_TYPE_ADD:
            ec_string_append_c_str(ctx->result, "\"+\"");
            break;
        case XJS_CFG_UNARY_OP_TYPE_SUB:
            ec_string_append_c_str(ctx->result, "\"-\"");
            break;
    }
    ec_string_append_c_str(ctx->result, ",\"dst\":");
    SERIALIZE_APPEND_INT(ctx->result, (int)unary_op->var_dst);
    ec_string_append_c_str(ctx->result, ",\"src\":");
    SERIALIZE_APPEND_INT(ctx->result, (int)unary_op->var_src);

    SERIALIZE_FOOTER();

    return ret;
}

static int xjs_cfg_serialize_binary_op( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref node)
{
    xjs_cfg_binary_op_ref binary_op = node->u.as_binary_op;
    int ret = 0;

    SERIALIZE_HEADER(xjs_cfg_node_map_get(ctx->nodes, node));

    ec_string_append_c_str(ctx->result, ",\"type\":\"BinaryOp\",\"op\":");
    switch (binary_op->type)
    {
        case XJS_CFG_BINARY_OP_TYPE_UNKNOWN:
            ret = -1;
            break;

        case XJS_CFG_BINARY_OP_TYPE_ADD:
            ec_string_append_c_str(ctx->result, "\"+\"");
            break;
        case XJS_CFG_BINARY_OP_TYPE_SUB:
            ec_string_append_c_str(ctx->result, "\"-\"");
            break;
        case XJS_CFG_BINARY_OP_TYPE_MUL:
            ec_string_append_c_str(ctx->result, "\"*\"");
            break;
        case XJS_CFG_BINARY_OP_TYPE_DIV:
            ec_string_append_c_str(ctx->result, "\"/\"");
            break;
        case XJS_CFG_BINARY_OP_TYPE_MOD:
            ec_string_append_c_str(ctx->result, "\"%\"");
            break;
        case XJS_CFG_BINARY_OP_TYPE_E2:
            ec_string_append_c_str(ctx->result, "\"==\"");
            break;
        case XJS_CFG_BINARY_OP_TYPE_NE2:
            ec_string_append_c_str(ctx->result, "\"!=\"");
            break;
        case XJS_CFG_BINARY_OP_TYPE_E3:
            ec_string_append_c_str(ctx->result, "\"===\"");
            break;
        case XJS_CFG_BINARY_OP_TYPE_NE3:
            ec_string_append_c_str(ctx->result, "\"!==\"");
            break;
        case XJS_CFG_BINARY_OP_TYPE_L:
            ec_string_append_c_str(ctx->result, "\"<\"");
            break;
        case XJS_CFG_BINARY_OP_TYPE_LE:
            ec_string_append_c_str(ctx->result, "\"<=\"");
            break;
        case XJS_CFG_BINARY_OP_TYPE_G:
            ec_string_append_c_str(ctx->result, "\">\"");
            break;
        case XJS_CFG_BINARY_OP_TYPE_GE:
            ec_string_append_c_str(ctx->result, "\">=\"");
            break;
        case XJS_CFG_BINARY_OP_TYPE_AND:
            ec_string_append_c_str(ctx->result, "\"&&\"");
            break;
        case XJS_CFG_BINARY_OP_TYPE_OR:
            ec_string_append_c_str(ctx->result, "\"||\"");
            break;
    }
    ec_string_append_c_str(ctx->result, ",\"dst\":");
    SERIALIZE_APPEND_INT(ctx->result, (int)(binary_op->var_dst));
    ec_string_append_c_str(ctx->result, ",\"src1\":");
    SERIALIZE_APPEND_INT(ctx->result, (int)(binary_op->var_src1));
    ec_string_append_c_str(ctx->result, ",\"src2\":");
    SERIALIZE_APPEND_INT(ctx->result, (int)(binary_op->var_src2));

    SERIALIZE_FOOTER();

    return ret;
}

/* static int xjs_cfg_serialize_node( \ */
/*         xjs_cfg_serialize_ctx_ref ctx, \ */
/*         xjs_cfg_node_ref node) */
/* { */
/*     int ret = 0; */
/*     ec_size_t count = xjs_cfg_node_map_count(ctx->nodes, node); */

/*     /1* Serialized this node, ignore it *1/ */
/*     if (count == 1) return 0; */

/*     if (count == 0) */
/*     { */
/*         xjs_cfg_node_map_insert(ctx->nodes, node, ctx->node_index++); */
/*         switch (node->type) */
/*         { */
/*             case XJS_CFG_NODE_TYPE_UNKNOWN: */
/*                 return -1; */

/*             case XJS_CFG_NODE_TYPE_LITERAL: */
/*                 ret = xjs_cfg_serialize_literal(ctx, node->u.as_literal); */
/*                 break; */

/*             case XJS_CFG_NODE_TYPE_ALLOCA: */
/*                 ret = xjs_cfg_serialize_alloca(ctx, node->u.as_alloca); */
/*                 break; */

/*             case XJS_CFG_NODE_TYPE_LOAD: */
/*                 ret = xjs_cfg_serialize_load(ctx, node->u.as_load); */
/*                 break; */

/*             case XJS_CFG_NODE_TYPE_UNARY_OP: */
/*                 ret = xjs_cfg_serialize_unary_op(ctx, node->u.as_unary_op); */
/*                 break; */

/*             case XJS_CFG_NODE_TYPE_BINARY_OP: */
/*                 ret = xjs_cfg_serialize_binary_op(ctx, node->u.as_binary_op); */
/*                 break; */

/*             case XJS_CFG_NODE_TYPE_HALT: */
/*                 ret = xjs_cfg_serialize_halt(ctx); */
/*                 break; */

/*             case XJS_CFG_NODE_TYPE_JUMP: */
/*                 ret = xjs_cfg_serialize_jump(ctx, node->u.as_jump); */
/*                 break; */

/*             case XJS_CFG_NODE_TYPE_BLOCK: */
/*                 ret = xjs_cfg_serialize_block(ctx, node->u.as_block, node->comment); */
/*                 break; */

/*             case XJS_CFG_NODE_TYPE_DECLVAR: */
/*                 ret = xjs_cfg_serialize_declvar(ctx, node->u.as_declvar); */
/*                 break; */

/*             case XJS_CFG_NODE_TYPE_STORE: */
/*                 ret = xjs_cfg_serialize_store(ctx, node->u.as_store); */
/*                 break; */

/*             case XJS_CFG_NODE_TYPE_DROP: */
/*                 return -1; */

/*             case XJS_CFG_NODE_TYPE_BRANCH: */
/*                 ret = xjs_cfg_serialize_branch(ctx, node->u.as_branch); */
/*                 break; */
/*         } */
/*         ctx->node_wrote++; */
/*     } */
/*     else */
/*     { */
/*         /1* Impossible that a node has been recorded more than once *1/ */
/*         ret = -1; */
/*     } */

/*     return ret; */
/* } */

static int xjs_cfg_serialize_body( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref top_level)
{
    int ret = 0;
    xjs_cfg_node_stack *walk_stack = NULL;
    xjs_visited_node_set *visited_nodes = NULL;

    /* Record visited nodes */
    visited_nodes = ect_set_new(xjs_visited_node_set);

    /* Build the walk stack */
    walk_stack = ect_stack_new(xjs_cfg_node_stack);

    /* Push the entry node */
    xjs_cfg_node_stack_push(walk_stack, top_level);

    while (!xjs_cfg_node_stack_empty(walk_stack))
    {
        xjs_cfg_node_ref node = xjs_cfg_node_stack_top(walk_stack);
        xjs_cfg_node_stack_pop(walk_stack);

        if (ect_set_count(xjs_visited_node_set, visited_nodes, node) != 0)
        { continue; }
        ect_set_insert(xjs_visited_node_set, visited_nodes, node);

        switch (node->type)
        {
            case XJS_CFG_NODE_TYPE_UNKNOWN:
                return -1;

            case XJS_CFG_NODE_TYPE_LITERAL:
                ret = xjs_cfg_serialize_literal(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_ALLOCA:
                ret = xjs_cfg_serialize_alloca(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_LOAD:
                ret = xjs_cfg_serialize_load(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_UNARY_OP:
                ret = xjs_cfg_serialize_unary_op(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_BINARY_OP:
                ret = xjs_cfg_serialize_binary_op(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_HALT:
                ret = xjs_cfg_serialize_halt(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_JUMP:
                ret = xjs_cfg_serialize_jump(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_BLOCK:
                ret = xjs_cfg_serialize_block(ctx, node, node->comment);
                break;

            case XJS_CFG_NODE_TYPE_DECLVAR:
                ret = xjs_cfg_serialize_declvar(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_STORE:
                ret = xjs_cfg_serialize_store(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_OBJECT_SET:
                ret = xjs_cfg_serialize_object_set(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_OBJECT_GET:
                ret = xjs_cfg_serialize_object_get(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_ARRAY_PUSH:
                ret = xjs_cfg_serialize_array_push(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_DROP:
                return -1;

            case XJS_CFG_NODE_TYPE_BRANCH:
                ret = xjs_cfg_serialize_branch(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_MERGE:
                ret = xjs_cfg_serialize_merge(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_MAKE_FUNCTION:
                ret = xjs_cfg_serialize_make_function(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_MAKE_ARROW_FUNCTION:
                ret = xjs_cfg_serialize_make_arrow_function(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_INSPECT:
                ret = xjs_cfg_serialize_inspect(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_RETURN:
                ret = xjs_cfg_serialize_return(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_CALL:
                ret = xjs_cfg_serialize_call(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_NEW:
                ret = xjs_cfg_serialize_new(ctx, node);
                break;

            case XJS_CFG_NODE_TYPE_THIS:
                ret = xjs_cfg_serialize_this(ctx, node);
                break;
        }

        if (node->type == XJS_CFG_NODE_TYPE_BLOCK)
        {
            ect_reverse_iterator(xjs_cfg_node_list) it;
            ect_for_reverse(xjs_cfg_node_list, node->u.as_block->items, it)
            {
                xjs_cfg_node_ref item = ect_deref(xjs_cfg_node_ref, it);
                xjs_cfg_node_stack_push(walk_stack, item);
            }
        }
        else if (node->type == XJS_CFG_NODE_TYPE_JUMP)
        {
            xjs_cfg_node_stack_push(walk_stack, node->u.as_jump->dest);
        }
        else if (node->type == XJS_CFG_NODE_TYPE_BRANCH)
        {
            xjs_cfg_node_stack_push(walk_stack, node->u.as_branch->true_branch);
            xjs_cfg_node_stack_push(walk_stack, node->u.as_branch->false_branch);
        }
        else
        {
            /* Nothing to do */
        }
    }

    ec_delete(visited_nodes);
    ec_delete(walk_stack);
    return ret;
}

/* Fill ctx->nodes : which maps node to index */
static int xjs_cfg_collect_nodes( \
        xjs_cfg_serialize_ctx_ref ctx, \
        xjs_cfg_node_ref entry)
{
    int ret = 0;
    xjs_cfg_node_stack *walk_stack = NULL;

    /* Build the walk stack */
    walk_stack = ect_stack_new(xjs_cfg_node_stack);

    /* Push the entry node */
    xjs_cfg_node_stack_push(walk_stack, entry);

    while (!xjs_cfg_node_stack_empty(walk_stack))
    {
        xjs_cfg_node_ref node_cur = xjs_cfg_node_stack_top(walk_stack);
        xjs_cfg_node_stack_pop(walk_stack);

        if (ect_map_count(xjs_cfg_node_map, ctx->nodes, node_cur) != 0)
        { continue; }
        ect_map_insert(xjs_cfg_node_map, ctx->nodes, node_cur, ctx->node_index++);

        if (node_cur->type == XJS_CFG_NODE_TYPE_BLOCK)
        {
            ect_reverse_iterator(xjs_cfg_node_list) it;
            ect_for_reverse(xjs_cfg_node_list, node_cur->u.as_block->items, it)
            {
                xjs_cfg_node_ref item = ect_deref(xjs_cfg_node_ref, it);
                xjs_cfg_node_stack_push(walk_stack, item);
            }
        }
        else if (node_cur->type == XJS_CFG_NODE_TYPE_JUMP)
        {
            xjs_cfg_node_stack_push(walk_stack, node_cur->u.as_jump->dest);
        }
        else if (node_cur->type == XJS_CFG_NODE_TYPE_BRANCH)
        {
            xjs_cfg_node_stack_push(walk_stack, node_cur->u.as_branch->true_branch);
            xjs_cfg_node_stack_push(walk_stack, node_cur->u.as_branch->false_branch);
        }
        else
        {
            /* Nothing to do */
        }
    }

    ec_delete(walk_stack);
    return ret;
}

char *xjs_cfg_serialize(xjs_cfg_ref cfg)
{
    xjs_cfg_serialize_ctx ctx;
    char *encoded_result = NULL;
    ec_size_t encoded_result_len;
    ec_string *result = NULL;
    xjs_cfg_node_map_ref nodes = NULL;
    xjs_cfg_function_map_ref functions = NULL;
    ec_bool first;
    int function_id = 0;

    if ((result = ec_string_new()) == NULL) { return NULL; }
    ctx.result = result;

    if ((functions = ect_map_new(xjs_cfg_function_map)) == NULL)
    { goto fail; }
    ctx.functions = functions;

    {
        ect_iterator(xjs_cfg_function_list) it;
        ect_for(xjs_cfg_function_list, cfg->functions, it)
        {
            xjs_cfg_function_ref func = ect_deref(xjs_cfg_function_ref, it);
            ect_map_insert(xjs_cfg_function_map, functions, \
                    func, function_id++);
        }
    }

    first = ec_true;
    ec_string_append_c_str(result, "{");
    ec_string_append_c_str(result, "\"functions\":[");
    {
        ect_iterator(xjs_cfg_function_list) it;
        ect_for(xjs_cfg_function_list, cfg->functions, it)
        {
            xjs_cfg_function_ref func = ect_deref(xjs_cfg_function_ref, it);
            xjs_cfg_node_ref entry = func->body;

            if ((nodes = xjs_cfg_node_map_new()) == NULL) 
            { goto fail; }

            ctx.nodes = nodes;
            ctx.node_index = 0;
            ctx.first = ec_true;
            xjs_cfg_collect_nodes(&ctx, entry);

            if (first == ec_true) first = ec_false; else ec_string_append_c_str(result, ",");

            ec_string_append_c_str(result, "{");
            {
                /* Id */
                ec_string_append_c_str(result, "\"id\":");
                SERIALIZE_APPEND_INT(result, \
                        xjs_cfg_function_map_get(functions, func));

                /* Extract nodes */
                ec_string_append_c_str(result, ",\"nodes\":[");
                {
                    if (xjs_cfg_serialize_body(&ctx, entry) != 0)
                    { goto fail; }
                }
                ec_string_append_c_str(result, "]");

                /* Entry point */
                if (ctx.node_index > 0)
                {
                    ec_string_append_c_str(result, ",\"entry\":");
                    SERIALIZE_APPEND_INT(result, \
                            xjs_cfg_node_map_get(ctx.nodes, entry));
                }
            }

            ec_delete(nodes);
            ec_string_append_c_str(result, "}");
        }
    }
    ec_string_append_c_str(result, "]");
    if (ect_list_size(xjs_cfg_export_symbol_list, cfg->export_symbols) != 0)
    {
        ec_string_append_c_str(result, ",\"exported\":[");
        {
            first = ec_true;
            ect_iterator(xjs_cfg_export_symbol_list) it;
            ect_for(xjs_cfg_export_symbol_list, cfg->export_symbols, it)
            {
                xjs_cfg_export_symbol_ref export_symbol = ect_deref(xjs_cfg_export_symbol_ref, it);
                if (first == ec_true) first = ec_false; else ec_string_append_c_str(result, ",");
                ec_string_append_c_str(result, "{\"exported\":\"");
                ec_string_append(result, export_symbol->exported);
                ec_string_append_c_str(result, "\",\"local\":\"");
                ec_string_append(result, export_symbol->local);
                ec_string_append_c_str(result, "\"}");
            }
        }
        ec_string_append_c_str(result, "]");
    }
    if (ect_list_size(xjs_cfg_import_symbol_list, cfg->import_symbols) != 0)
    {
        ec_string_append_c_str(result, ",\"imported\":[");
        {
            first = ec_true;
            ect_iterator(xjs_cfg_import_symbol_list) it;
            ect_for(xjs_cfg_import_symbol_list, cfg->import_symbols, it)
            {
                xjs_cfg_import_symbol_ref import_symbol = ect_deref(xjs_cfg_import_symbol_ref, it);
                if (first == ec_true) first = ec_false; else ec_string_append_c_str(result, ",");
                ec_string_append_c_str(result, "{\"local\":\"");
                ec_string_append(result, import_symbol->local);
                ec_string_append_c_str(result, "\",\"imported\":\"");
                ec_string_append(result, import_symbol->imported);
                ec_string_append_c_str(result, "\",\"source\":\"");
                ec_string_append(result, import_symbol->source);
                ec_string_append_c_str(result, "\"}");
            }
        }
    ec_string_append_c_str(result, "]");
    }
    ec_string_append_c_str(result, ",\"toplevel\":");
    SERIALIZE_APPEND_INT(result, \
            xjs_cfg_function_map_get(functions, cfg->top_level));
    ec_string_append_c_str(result, "}");

    /* Encode as UTF-8 */
    {
        ec_encoding_t enc;
        ec_encoding_utf8_init(&enc);
        ec_encoding_encode(&enc, (ec_byte_t **)&encoded_result, &encoded_result_len, result);
    }

fail:
    ec_delete(result);
    ec_delete(functions);
    return encoded_result;
}

