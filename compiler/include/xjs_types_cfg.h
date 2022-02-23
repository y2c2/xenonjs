/* XenonJS : Types : Control Flow
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_TYPES_CFG_H
#define XJS_TYPES_CFG_H

#include <ec_string.h>
#include <ec_list.h>
#include "xjs_dt.h"
#include "xjs_types.h"

typedef xjs_size_t xjs_cfg_var;

#ifndef DECL_STRUCT_REF_CFG
#define DECL_STRUCT_REF_CFG(name) \
    struct xjs_opaque_cfg##name; \
    typedef struct xjs_opaque_cfg##name xjs_cfg##name; \
    typedef struct xjs_opaque_cfg##name *xjs_cfg##name##_ref
#endif

DECL_STRUCT_REF_CFG(_alloca);

DECL_STRUCT_REF_CFG(_literal);
DECL_STRUCT_REF_CFG(_load);
DECL_STRUCT_REF_CFG(_store);

DECL_STRUCT_REF_CFG(_object_set);
DECL_STRUCT_REF_CFG(_object_get);
DECL_STRUCT_REF_CFG(_array_push);

DECL_STRUCT_REF_CFG(_unary_op);
DECL_STRUCT_REF_CFG(_binary_op);
DECL_STRUCT_REF_CFG(_drop);
DECL_STRUCT_REF_CFG(_half);
DECL_STRUCT_REF_CFG(_jump);
DECL_STRUCT_REF_CFG(_branch);
DECL_STRUCT_REF_CFG(_merge);
DECL_STRUCT_REF_CFG(_block);
DECL_STRUCT_REF_CFG(_declvar);
DECL_STRUCT_REF_CFG(_parameter);
DECL_STRUCT_REF_CFG(_make_function);
DECL_STRUCT_REF_CFG(_make_arrow_function);
DECL_STRUCT_REF_CFG(_inspect);
DECL_STRUCT_REF_CFG(_return);
DECL_STRUCT_REF_CFG(_call);
DECL_STRUCT_REF_CFG(_new);
DECL_STRUCT_REF_CFG(_this);
DECL_STRUCT_REF_CFG(_node);
DECL_STRUCT_REF_CFG(_function);
DECL_STRUCT_REF_CFG(_export_symbol);
DECL_STRUCT_REF_CFG(_import_symbol);

struct xjs_cfg_node_list_container;
typedef struct xjs_cfg_node_list_container *xjs_cfg_node_list_ref;

struct xjs_opaque_cfg;
typedef struct xjs_opaque_cfg xjs_cfg;
typedef struct xjs_opaque_cfg *xjs_cfg_ref;

/* CFG var list */

ect_list_declare(xjs_cfg_var_list, xjs_cfg_var);
typedef xjs_cfg_var_list *xjs_cfg_var_list_ref;

/* Location */
struct xjs_opaque_cfg_loc
{
    struct
    {
         int ln, col;
    } start, end;
};
typedef struct xjs_opaque_cfg_loc xjs_cfg_loc;
typedef struct xjs_opaque_cfg_loc *xjs_cfg_loc_ref;

/* Range */
struct xjs_opaque_cfg_range
{
    int start, end;
};
typedef struct xjs_opaque_cfg_range xjs_cfg_range;
typedef struct xjs_opaque_cfg_range *xjs_cfg_range_ref;

/* Node: Alloca */

struct xjs_opaque_cfg_alloca
{
    xjs_cfg_var var;
};

/* Node : Literal */

typedef enum
{
    XJS_CFG_LITERAL_TYPE_UNKNOWN,
    XJS_CFG_LITERAL_TYPE_UNDEFINED,
    XJS_CFG_LITERAL_TYPE_NULL,
    XJS_CFG_LITERAL_TYPE_BOOL,
    XJS_CFG_LITERAL_TYPE_NUMBER,
    XJS_CFG_LITERAL_TYPE_STRING,
    XJS_CFG_LITERAL_TYPE_OBJECT,
    XJS_CFG_LITERAL_TYPE_ARRAY,
} xjs_cfg_literal_type;

struct xjs_opaque_cfg_literal
{
    xjs_cfg_literal_type type;
    union
    {
        ec_bool as_bool;
        ec_string *as_string;
        double as_number;
    } u;
    xjs_cfg_var var;
};

/* Node : Load */

struct xjs_opaque_cfg_load
{
    ec_string *name;
    xjs_cfg_var var;
};

/* Node : Object Set */

struct xjs_opaque_cfg_object_set
{
    xjs_cfg_var dst;
    xjs_cfg_var obj;
    xjs_cfg_var key;
    xjs_cfg_var value;
};

/* Node : Object Get */
struct xjs_opaque_cfg_object_get
{
    xjs_cfg_var dst;
    xjs_cfg_var obj;
    xjs_cfg_var key;
};

/* Node : Array Push */
struct xjs_opaque_cfg_array_push
{
    xjs_cfg_var arr;
    xjs_cfg_var elem;
};

/* Node: Assign */

struct xjs_opaque_cfg_store
{
    ec_string *name;
    xjs_cfg_var var;
};

/* Node : Unary Operation */

typedef enum
{
    XJS_CFG_UNARY_OP_TYPE_UNKNOWN,
    XJS_CFG_UNARY_OP_TYPE_NOT, /* ! */
    XJS_CFG_UNARY_OP_TYPE_BNOT, /* ~ */
    XJS_CFG_UNARY_OP_TYPE_ADD, /* + */
    XJS_CFG_UNARY_OP_TYPE_SUB, /* - */
} xjs_cfg_unary_op_type;

struct xjs_opaque_cfg_unary_op
{
    xjs_cfg_unary_op_type type;
    xjs_cfg_var var_dst;
    xjs_cfg_var var_src;
};

/* Node : Binary Operation */

typedef enum
{
    XJS_CFG_BINARY_OP_TYPE_UNKNOWN,
    XJS_CFG_BINARY_OP_TYPE_ADD,
    XJS_CFG_BINARY_OP_TYPE_SUB,
    XJS_CFG_BINARY_OP_TYPE_MUL,
    XJS_CFG_BINARY_OP_TYPE_DIV,
    XJS_CFG_BINARY_OP_TYPE_MOD,
    XJS_CFG_BINARY_OP_TYPE_E2, /* == */
    XJS_CFG_BINARY_OP_TYPE_NE2, /* != */
    XJS_CFG_BINARY_OP_TYPE_E3, /* === */
    XJS_CFG_BINARY_OP_TYPE_NE3, /* !== */
    XJS_CFG_BINARY_OP_TYPE_L, /* < */
    XJS_CFG_BINARY_OP_TYPE_LE, /* <= */
    XJS_CFG_BINARY_OP_TYPE_G, /* > */
    XJS_CFG_BINARY_OP_TYPE_GE, /* >= */
    XJS_CFG_BINARY_OP_TYPE_AND, /* && */
    XJS_CFG_BINARY_OP_TYPE_OR, /* || */
} xjs_cfg_binary_op_type;

struct xjs_opaque_cfg_binary_op
{
    xjs_cfg_binary_op_type type;
    xjs_cfg_var var_dst;
    xjs_cfg_var var_src1, var_src2;
};


/* Node : Function */

struct xjs_opaque_cfg_parameter
{
    ec_string *name;
    xjs_cfg_loc loc;
    xjs_cfg_range range;
};

ect_list_declare(xjs_cfg_parameter_list, xjs_cfg_parameter_ref);
typedef xjs_cfg_parameter_list *xjs_cfg_parameter_list_ref;

/* Node : Drop */

struct xjs_opaque_cfg_drop
{
    xjs_cfg_node_ref value;
};

/* Node : Halt */

/* Node : Branch */

struct xjs_opaque_cfg_jump
{
    xjs_cfg_node_ref dest;
};

/* Node : Branch */

struct xjs_opaque_cfg_branch
{
    xjs_cfg_var cond;
    xjs_cfg_node_ref true_branch;
    xjs_cfg_node_ref false_branch;
};

/* Node : Merge */

struct xjs_opaque_cfg_merge
{
    xjs_cfg_var test;
    xjs_cfg_var dst;
    xjs_cfg_var consequent;
    xjs_cfg_var alternate;
};

/* Node : Block */

struct xjs_opaque_cfg_block
{
    xjs_cfg_node_list_ref items;
};

/* Node : DeclVar */

struct xjs_opaque_cfg_declvar
{
    ec_string *name;
};

/* Node : Make Function */

struct xjs_opaque_cfg_make_function
{
    xjs_cfg_var var_dst;
    xjs_cfg_function_ref func;
};

/* Node : Make Arrow Function */

struct xjs_opaque_cfg_make_arrow_function
{
    xjs_cfg_var var_dst;
    xjs_cfg_function_ref func;
};

/* Node : Inspect */

struct xjs_opaque_cfg_inspect
{
    xjs_cfg_var var;
};

/* Node : Return */

struct xjs_opaque_cfg_return
{
    xjs_cfg_var var;
};

/* Node : Call */

struct xjs_opaque_cfg_call
{
    xjs_cfg_var dst;
    xjs_cfg_var callee;
    xjs_cfg_var_list_ref arguments;

    struct
    {
        xjs_bool enabled;
        xjs_cfg_var _this;
    } bound_this;
};

/* Node : New */

struct xjs_opaque_cfg_new
{
    xjs_cfg_var dst;
    xjs_cfg_var callee;
    xjs_cfg_var_list_ref arguments;
};

/* Node : This */

struct xjs_opaque_cfg_this
{
    xjs_cfg_var dst;
};

/* Node */

typedef enum
{
    XJS_CFG_NODE_TYPE_UNKNOWN,
    XJS_CFG_NODE_TYPE_ALLOCA,

    XJS_CFG_NODE_TYPE_LITERAL, /* t = <literal> */
    XJS_CFG_NODE_TYPE_LOAD, /* t = variable */
    XJS_CFG_NODE_TYPE_STORE, /* variable = t */

    XJS_CFG_NODE_TYPE_OBJECT_SET,
    XJS_CFG_NODE_TYPE_OBJECT_GET,
    XJS_CFG_NODE_TYPE_ARRAY_PUSH,

    XJS_CFG_NODE_TYPE_UNARY_OP,
    XJS_CFG_NODE_TYPE_BINARY_OP,
    XJS_CFG_NODE_TYPE_DROP,
    XJS_CFG_NODE_TYPE_HALT,
    XJS_CFG_NODE_TYPE_JUMP,
    XJS_CFG_NODE_TYPE_BRANCH,
    XJS_CFG_NODE_TYPE_MERGE,
    XJS_CFG_NODE_TYPE_BLOCK,
    XJS_CFG_NODE_TYPE_DECLVAR,
    XJS_CFG_NODE_TYPE_MAKE_FUNCTION,
    XJS_CFG_NODE_TYPE_MAKE_ARROW_FUNCTION,
    XJS_CFG_NODE_TYPE_INSPECT,
    XJS_CFG_NODE_TYPE_RETURN,
    XJS_CFG_NODE_TYPE_CALL,
    XJS_CFG_NODE_TYPE_NEW,
    XJS_CFG_NODE_TYPE_THIS,
} xjs_cfg_node_type;

struct xjs_opaque_cfg_node
{
    xjs_cfg_node_type type;
    union
    {
        xjs_cfg_alloca_ref as_alloca;

        xjs_cfg_literal_ref as_literal;
        xjs_cfg_load_ref as_load;
        xjs_cfg_store_ref as_store;

        xjs_cfg_object_set_ref as_object_set;
        xjs_cfg_object_get_ref as_object_get;
        xjs_cfg_array_push_ref as_array_push;

        xjs_cfg_unary_op_ref as_unary_op;
        xjs_cfg_binary_op_ref as_binary_op;
        xjs_cfg_drop_ref as_drop;
        xjs_cfg_jump_ref as_jump;
        xjs_cfg_branch_ref as_branch;
        xjs_cfg_merge_ref as_merge;
        xjs_cfg_block_ref as_block;
        xjs_cfg_declvar_ref as_declvar;
        xjs_cfg_make_function_ref as_make_function;
        xjs_cfg_make_arrow_function_ref as_make_arrow_function;
        xjs_cfg_inspect_ref as_inspect;
        xjs_cfg_return_ref as_return;
        xjs_cfg_call_ref as_call;
        xjs_cfg_new_ref as_new;
        xjs_cfg_this_ref as_this;
    } u;

    ec_string *comment;

    xjs_cfg_loc loc;
    xjs_cfg_range range;
};

/* Node List */
ect_list_declare(xjs_cfg_node_list, xjs_cfg_node_ref);

/* Allocated Nodes */
ect_list_declare(xjs_cfg_cfgbuilder_context_node_list, xjs_cfg_node_ref);
typedef xjs_cfg_cfgbuilder_context_node_list *xjs_cfg_cfgbuilder_context_node_list_ref;

/* Function Export */
typedef enum
{
    xjs_cfg_function_export_type_none,

    /* export * from 'foo' */
    xjs_cfg_function_export_type_all,

    /* export default <expression>; */
    xjs_cfg_function_export_type_default,

    /* export { foo }; */
    xjs_cfg_function_export_type_named,
} xjs_cfg_function_export_type;

struct xjs_opaque_cfg_function_export
{
    xjs_cfg_function_export_type type;
    union
    {
        struct
        {
            int dummy;
        } as_default;
    } u;
};
typedef struct xjs_opaque_cfg_function_export xjs_cfg_function_export;

/* Node : Function */
struct xjs_opaque_cfg_function
{
    xjs_cfg_function_export exporting;

    xjs_cfg_parameter_list_ref parameters;
    xjs_cfg_node_ref body;
};

ect_list_declare(xjs_cfg_function_list, xjs_cfg_function_ref);
typedef xjs_cfg_function_list *xjs_cfg_function_list_ref;

/* Exports */
struct xjs_opaque_cfg_export_symbol
{
    ec_string *local;
    ec_string *exported;
};
ect_list_declare(xjs_cfg_export_symbol_list, xjs_cfg_export_symbol_ref);
typedef xjs_cfg_export_symbol_list *xjs_cfg_export_symbol_list_ref;

/* Imports */
struct xjs_opaque_cfg_import_symbol
{
    ec_string *local;
    ec_string *imported;
    ec_string *source;
};
ect_list_declare(xjs_cfg_import_symbol_list, xjs_cfg_import_symbol_ref);
typedef xjs_cfg_import_symbol_list *xjs_cfg_import_symbol_list_ref;

/* CFG */
struct xjs_opaque_cfg
{
    /* Allocated Nodes */
    xjs_cfg_cfgbuilder_context_node_list_ref allocated_nodes;

    xjs_cfg_function_ref top_level;
    xjs_cfg_function_list_ref functions;
    xjs_cfg_export_symbol_list_ref export_symbols;
    xjs_cfg_import_symbol_list_ref import_symbols;
};

#endif

