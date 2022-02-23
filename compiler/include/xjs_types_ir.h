/* XenonJS : Types : IR
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_TYPES_IR_H
#define XJS_TYPES_IR_H

#include <ec_string.h>
#include <ec_list.h>
#include <ec_map.h>
#include "xjs_dt.h"

typedef xjs_u8 xjs_ir_reg;
typedef xjs_u32 xjs_ir_offset;

#ifndef DECL_STRUCT_REF
#define DECL_STRUCT_REF(name) \
    struct xjs_opaque##name; \
    typedef struct xjs_opaque##name xjs##name; \
    typedef struct xjs_opaque##name *xjs##name##_ref
#endif

DECL_STRUCT_REF(_ir_data_item);
DECL_STRUCT_REF(_ir_data);
DECL_STRUCT_REF(_ir_text_item);
DECL_STRUCT_REF(_ir_text);
DECL_STRUCT_REF(_ir_export_item);
DECL_STRUCT_REF(_ir_import_item);
DECL_STRUCT_REF(_ir_parameter);
DECL_STRUCT_REF(_ir_function);
DECL_STRUCT_REF(_ir);

typedef xjs_u32 xjs_ir_var;
typedef xjs_u32 xjs_ir_variable;
typedef xjs_u32 xjs_ir_label;
typedef xjs_u32 xjs_ir_dataid;
typedef xjs_u32 xjs_ir_functionid;

/* IR Variable List */

ect_list_declare(xjs_ir_var_list, xjs_ir_var);
typedef xjs_ir_var_list *xjs_ir_var_list_ref;

typedef enum
{
    xjs_ir_data_item_type_string,
    xjs_ir_data_item_type_symbol,
    xjs_ir_data_item_type_f32,
    xjs_ir_data_item_type_f64,
} xjs_ir_data_item_type;

/* IR Data */
struct xjs_opaque_ir_data_item
{
    xjs_ir_data_item_type type;
    xjs_ir_dataid dataid;
    union
    {
        struct
        {
            ec_string *value;
        } as_string;
        struct
        {
            ec_string *value;
        } as_symbol;
        union
        {
            xjs_f32 value;
            xjs_u8 bytes[4];
        } as_f32;
        union
        {
            xjs_f64 value;
            xjs_u8 bytes[8];
        } as_f64;
    } u;
};

ect_list_declare(xjs_ir_data_item_list, xjs_ir_data_item_ref);
typedef xjs_ir_data_item_list *xjs_ir_data_item_list_ref;

struct xjs_opaque_ir_data
{
    xjs_ir_data_item_list_ref items;
};

/* Location */
struct xjs_opaque_ir_loc
{
    ec_string *filename;
    struct
    {
         int ln, col;
    } start, end;
};
typedef struct xjs_opaque_ir_loc xjs_ir_loc;
typedef struct xjs_opaque_ir_loc *xjs_ir_loc_ref;

/* Range */
struct xjs_opaque_ir_range
{
    int start, end;
};
typedef struct xjs_opaque_ir_range xjs_ir_range;
typedef struct xjs_opaque_ir_range *xjs_ir_range_ref;

/* Function */

typedef enum
{
    xjs_ir_text_item_type_nop, /* nop */
    xjs_ir_text_item_type_halt, /* halt */
    xjs_ir_text_item_type_inspect, /* inspect */
    xjs_ir_text_item_type_dynlib, /* dynamic library */

    xjs_ir_text_item_type_push_scope,
    xjs_ir_text_item_type_pop_scope,

    xjs_ir_text_item_type_label, /* label(lbl) */
    xjs_ir_text_item_type_br, /* br(dest) */
    xjs_ir_text_item_type_br_cond, /* br_cond(cond,dest) */
    xjs_ir_text_item_type_merge, /* merge(test,consequent,alternate,dst) */

    xjs_ir_text_item_type_alloca, /* var = alloca() */
    xjs_ir_text_item_type_load_undefined, /* var = load_undefined() */
    xjs_ir_text_item_type_load_null, /* var = load_null() */
    xjs_ir_text_item_type_load_bool, /* var = load_bool(value) */
    xjs_ir_text_item_type_load_string, /* var = load_string(dataid) */
    xjs_ir_text_item_type_load_number, /* var = load_number(value) */
    xjs_ir_text_item_type_load_object, /* var = load_object() */
    xjs_ir_text_item_type_load_array, /* var = load_array() */

    xjs_ir_text_item_type_declvar, /* declvar(variable) */
    xjs_ir_text_item_type_load, /* var = load(variable) */
    xjs_ir_text_item_type_store, /* store(variable, var) */
    xjs_ir_text_item_type_object_set, /* object_set(object, name, value) */
    xjs_ir_text_item_type_object_get, /* dst = object_get(object, name) */
    xjs_ir_text_item_type_array_push, /* dst = array_push(arr, elem) */
    xjs_ir_text_item_type_make_function, /* var = make_function */
    xjs_ir_text_item_type_make_arrow_function, /* var = make_arrow_function */
    xjs_ir_text_item_type_ret, /* ret(var) */
    xjs_ir_text_item_type_call, /* call(callee, arguments...) */
    xjs_ir_text_item_type_new, /* new(callee, arguments...) */
    xjs_ir_text_item_type_this, /* this() */

    xjs_ir_text_item_type_binary_add, /* dst = binary_add(lhs, rhs) */
    xjs_ir_text_item_type_binary_sub, /* dst = binary_sub(lhs, rhs) */
    xjs_ir_text_item_type_binary_mul, /* dst = binary_mul(lhs, rhs) */
    xjs_ir_text_item_type_binary_div, /* dst = binary_div(lhs, rhs) */
    xjs_ir_text_item_type_binary_mod, /* dst = binary_mod(lhs, rhs) */
    xjs_ir_text_item_type_binary_e2, /* dst = binary_e2(lhs, rhs) */
    xjs_ir_text_item_type_binary_ne2, /* dst = binary_ne2(lhs, rhs) */
    xjs_ir_text_item_type_binary_e3, /* dst = binary_e3(lhs, rhs) */
    xjs_ir_text_item_type_binary_ne3, /* dst = binary_ne3(lhs, rhs) */
    xjs_ir_text_item_type_binary_l, /* dst = binary_l(lhs, rhs) */
    xjs_ir_text_item_type_binary_le, /* dst = binary_le(lhs, rhs) */
    xjs_ir_text_item_type_binary_g, /* dst = binary_g(lhs, rhs) */
    xjs_ir_text_item_type_binary_ge, /* dst = binary_ge(lhs, rhs) */
    xjs_ir_text_item_type_binary_and, /* dst = binary_and(lhs, rhs) */
    xjs_ir_text_item_type_binary_or, /* dst = binary_or(lhs, rhs) */

    xjs_ir_text_item_type_unary_not, /* dst = unary_not(src) */
    xjs_ir_text_item_type_unary_bnot, /* dst = unary_bnot(src) */
    xjs_ir_text_item_type_unary_add, /* dst = unary_add(src) */
    xjs_ir_text_item_type_unary_sub, /* dst = unary_sub(src) */
} xjs_ir_text_item_type;

struct xjs_opaque_ir_text_item
{
    xjs_ir_text_item_type type;
    union
    {
        struct { xjs_ir_var var; } as_inspect;
        struct { xjs_ir_var exports; xjs_ir_var module_name; } as_dynlib;
        struct { xjs_ir_label lbl; } as_label;
        struct { xjs_ir_label dest; } as_br;
        struct { xjs_ir_var cond; xjs_ir_label dest; } as_br_cond;
        struct { xjs_ir_var test, consequent, alternate, dst; } as_merge;

        struct { xjs_ir_var var; } as_alloca;
        struct
        {
            xjs_ir_var var;
        } as_load_undefined, as_load_null;
        struct
        {
            xjs_ir_var var;
            xjs_bool value;
        } as_load_bool;
        struct { xjs_ir_var var; xjs_ir_dataid dataid; } as_load_string;
        struct { xjs_ir_var var; double value; } as_load_number;
        struct { xjs_ir_var var; } as_load_object;
        struct { xjs_ir_var var; } as_load_array;

        struct { xjs_ir_dataid variable; } as_declvar;
        struct { xjs_ir_var var; xjs_ir_dataid variable; } as_load;
        struct { xjs_ir_dataid variable; xjs_ir_var var; } as_store;
        struct { xjs_ir_var dst; xjs_ir_var obj; xjs_ir_var member; xjs_ir_var src; } as_object_set;
        struct { xjs_ir_var dst; xjs_ir_var obj; xjs_ir_var member; } as_object_get;
        struct { xjs_ir_var arr; xjs_ir_var elem; } as_array_push;
        struct { xjs_ir_var var; xjs_ir_functionid func; } as_make_function;
        struct { xjs_ir_var var; xjs_ir_functionid func; } as_make_arrow_function;
        struct { xjs_ir_var var; } as_ret;
        struct
        {
            xjs_ir_var dst, callee; xjs_ir_var_list_ref arguments;
            struct
            {
                xjs_bool enabled;
                xjs_ir_var _this;
            } bound_this;
        } as_call;
        struct { xjs_ir_var dst, callee; xjs_ir_var_list_ref arguments; } as_new;
        struct { xjs_ir_var dst; } as_this;

        struct { xjs_ir_var dst, lhs, rhs; } as_binary_op;
        struct { xjs_ir_var dst, src; } as_unary_op;
    } u;

    xjs_ir_loc loc;
    xjs_ir_range range;
};

ect_list_declare(xjs_ir_text_item_list, xjs_ir_text_item_ref);
typedef xjs_ir_text_item_list *xjs_ir_text_item_list_ref;

/* Parameter */
struct xjs_opaque_ir_parameter
{
    xjs_ir_dataid varname;

    xjs_ir_loc loc;
    xjs_ir_range range;
};

ect_list_declare(xjs_ir_parameter_list, xjs_ir_parameter_ref);
typedef xjs_ir_parameter_list *xjs_ir_parameter_list_ref;

/* Function */
struct xjs_opaque_ir_function
{
    /* Parameter */
    xjs_ir_parameter_list_ref parameters;

    /* Body */
    xjs_ir_text_item_list_ref text_items;
};

ect_list_declare(xjs_ir_function_list, xjs_ir_function_ref);
typedef xjs_ir_function_list *xjs_ir_function_list_ref;

/* IR Text */
struct xjs_opaque_ir_text
{
    xjs_ir_function_list_ref functions;
};

/* Export */

struct xjs_opaque_ir_export_item
{
    xjs_ir_dataid local;
    xjs_ir_dataid exported;
};

ect_list_declare(xjs_ir_export_list, xjs_ir_export_item_ref);
typedef xjs_ir_export_list *xjs_ir_export_list_ref;

/* Import */

struct xjs_opaque_ir_import_item
{
    xjs_ir_dataid local;
    xjs_ir_dataid imported;
    xjs_ir_dataid source;
};

ect_list_declare(xjs_ir_import_list, xjs_ir_import_item_ref);
typedef xjs_ir_import_list *xjs_ir_import_list_ref;

/* IR */
struct xjs_opaque_ir
{
    xjs_ir_text_ref text;
    xjs_ir_data_ref data;
    xjs_ir_function_ref toplevel;
    xjs_ir_export_list_ref exported;
    xjs_ir_import_list_ref imported;
    struct
    {
        char *name;
        char *fullpath;
    } module;
};


#endif

