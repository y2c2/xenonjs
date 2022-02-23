/* XenonJS : IR
 * Copyright(c) 2017 y2c2 */

#include <ec_algorithm.h>
#include <ec_string.h>
#include <ec_libc.h>
#include "xjs_ir.h"

static void xjs_ir_var_node_dtor(xjs_ir_var node)
{
    (void)node;
}

ect_list_define_declared(xjs_ir_var_list, \
        xjs_ir_var, xjs_ir_var_node_dtor);

xjs_ir_var_list_ref xjs_ir_var_list_clone(xjs_ir_var_list_ref r)
{
    xjs_ir_var_list_ref new_list = ect_list_new(xjs_ir_var_list);
    if (new_list == NULL) return NULL;
    {
        ect_iterator(xjs_ir_var_list) it;
        ect_for(xjs_ir_var_list, r, it)
        {
            ect_list_push_back(xjs_ir_var_list, new_list, ect_deref(xjs_ir_var, it));
        }
    }
    return new_list;
}

/* IR : Data */

static void xjs_ir_data_item_string_ctor(void *data_item)
{
    xjs_ir_data_item_ref r = data_item;
    r->type = xjs_ir_data_item_type_string; 
    r->u.as_string.value = NULL;
}

static void xjs_ir_data_item_string_dtor(void *data_item)
{
    xjs_ir_data_item_ref r = data_item;
    ec_delete(r->u.as_string.value);
}

xjs_ir_data_item_ref xjs_ir_data_item_string_new(xjs_ir_dataid dataid, ec_string *value)
{
    xjs_ir_data_item_ref r = ec_newcd(xjs_ir_data_item, \
            xjs_ir_data_item_string_ctor, \
            xjs_ir_data_item_string_dtor);
    r->dataid = dataid;
    r->u.as_string.value = value;
    return r;
}

static void xjs_ir_data_item_symbol_ctor(void *data_item)
{
    xjs_ir_data_item_ref r = data_item;
    r->type = xjs_ir_data_item_type_symbol; 
    r->u.as_symbol.value = NULL;
}

static void xjs_ir_data_item_symbol_dtor(void *data_item)
{
    xjs_ir_data_item_ref r = data_item;
    ec_delete(r->u.as_symbol.value);
}

xjs_ir_data_item_ref xjs_ir_data_item_symbol_new(xjs_ir_dataid dataid, ec_string *value)
{
    xjs_ir_data_item_ref r = ec_newcd(xjs_ir_data_item, \
            xjs_ir_data_item_symbol_ctor, \
            xjs_ir_data_item_symbol_dtor);
    r->dataid = dataid;
    r->u.as_symbol.value = value;
    return r;
}

static void xjs_ir_data_item_f32_ctor(void *data_item)
{
    xjs_ir_data_item_ref r = data_item;
    r->type = xjs_ir_data_item_type_f32; 
    r->u.as_f32.value = 0.0;
}

xjs_ir_data_item_ref xjs_ir_data_item_f32_new(xjs_ir_dataid dataid, xjs_f32 value)
{
    xjs_ir_data_item_ref r = ec_newcd(xjs_ir_data_item, \
            xjs_ir_data_item_f32_ctor, \
            NULL);
    r->dataid = dataid;
    r->u.as_f32.value = value;
    return r;
}

static void xjs_ir_data_item_f64_ctor(void *data_item)
{
    xjs_ir_data_item_ref r = data_item;
    r->type = xjs_ir_data_item_type_f64; 
    r->u.as_f64.value = 0.0;
}

xjs_ir_data_item_ref xjs_ir_data_item_f64_new(xjs_ir_dataid dataid, xjs_f64 value)
{
    xjs_ir_data_item_ref r = ec_newcd(xjs_ir_data_item, \
            xjs_ir_data_item_f64_ctor, \
            NULL);
    r->dataid = dataid;
    r->u.as_f64.value = value;
    return r;
}

xjs_ir_data_item_ref xjs_ir_data_item_clone(xjs_ir_data_item_ref r)
{
    xjs_ir_data_item_ref new_item = NULL;
    switch (r->type)
    {
        case xjs_ir_data_item_type_f32:
            new_item = xjs_ir_data_item_f32_new(r->dataid, r->u.as_f32.value);
            break;
        case xjs_ir_data_item_type_f64:
            new_item = xjs_ir_data_item_f64_new(r->dataid, r->u.as_f64.value);
            break;
        case xjs_ir_data_item_type_string:
            new_item = xjs_ir_data_item_string_new(r->dataid, ec_string_clone(r->u.as_string.value));
            break;
        case xjs_ir_data_item_type_symbol:
            new_item = xjs_ir_data_item_symbol_new(r->dataid, ec_string_clone(r->u.as_symbol.value));
            break;
    }
    return new_item;
}

static void xjs_ir_data_item_list_node_dtor(xjs_ir_data_item_ref node)
{
    ec_delete(node);
}


ect_list_define_declared(xjs_ir_data_item_list, \
        xjs_ir_data_item_ref, xjs_ir_data_item_list_node_dtor);

static void xjs_ir_data_ctor(void *data)
{
    xjs_ir_data_ref r = data;
    r->items = xjs_ir_data_item_list_new();
}

static void xjs_ir_data_dtor(void *data)
{
    xjs_ir_data_ref r = data;
    ec_delete(r->items);
}

static xjs_ir_data_ref xjs_ir_data_new(void)
{
    xjs_ir_data_ref r = ec_newcd(xjs_ir_data, xjs_ir_data_ctor, xjs_ir_data_dtor);
    return r;
}


/* Loc */

void xjs_ir_text_item_set_loc_filename(xjs_ir_text_item_ref ti, ec_string *filename)
{
    ti->loc.filename = filename;
}

void xjs_ir_text_item_set_loc_start(xjs_ir_text_item_ref ti, const int start_ln, const int start_col)
{
    ti->loc.start.ln = start_ln;
    ti->loc.start.col = start_col;
}

void xjs_ir_text_item_set_loc_end(xjs_ir_text_item_ref ti, const int end_ln, const int end_col)
{
    ti->loc.end.ln = end_ln;
    ti->loc.end.col = end_col;
}

static void xjs_ir_text_item_loc_init(xjs_ir_text_item_ref ti)
{
    ti->loc.filename = NULL;
    xjs_ir_text_item_set_loc_start(ti, -1, -1);
    xjs_ir_text_item_set_loc_end(ti, -1, -1);
}

/* Range */

void xjs_ir_text_item_set_range_start(xjs_ir_text_item_ref ti, const int start)
{
    ti->range.start = start;
}

void xjs_ir_text_item_set_range_end(xjs_ir_text_item_ref ti, const int end)
{
    ti->range.end = end;
}

static void xjs_ir_text_item_range_init(xjs_ir_text_item_ref ti)
{
    ti->range.start = -1;
    ti->range.end = -1;
}

static void xjs_ir_text_item_loc_range_init(xjs_ir_text_item_ref ti)
{
    xjs_ir_text_item_loc_init(ti);
    xjs_ir_text_item_range_init(ti);
}

/* Text Items */


/* nop */

static void xjs_ir_text_item_nop_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_nop;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_nop_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_nop_new(void)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_nop_ctor, \
            xjs_ir_text_item_nop_dtor);
    return r;
}


/* halt */

static void xjs_ir_text_item_halt_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_halt;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_halt_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_halt_new(void)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_halt_ctor, \
            xjs_ir_text_item_halt_dtor);
    return r;
}

/* dynlib */

static void xjs_ir_text_item_dynlib_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_dynlib;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_dynlib_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_dynlib_new( \
        xjs_ir_var exports, \
        xjs_ir_var module_name)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_dynlib_ctor, \
            xjs_ir_text_item_dynlib_dtor);
    r->u.as_dynlib.exports = exports;
    r->u.as_dynlib.module_name = module_name;
    return r;
}

/* label */

static void xjs_ir_text_item_label_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_label;
    r->u.as_label.lbl = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_label_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_label_new(xjs_ir_label lbl)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_label_ctor, \
            xjs_ir_text_item_label_dtor);
    r->u.as_label.lbl = lbl;
    return r;
}


/* br */

static void xjs_ir_text_item_br_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_br;
    r->u.as_br.dest = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_br_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_br_new(xjs_ir_label dest)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_br_ctor, \
            xjs_ir_text_item_br_dtor);
    r->u.as_br.dest = dest;
    return r;
}


/* br cond */

static void xjs_ir_text_item_br_cond_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_br_cond;
    r->u.as_br_cond.cond = 0;
    r->u.as_br_cond.dest = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_br_cond_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_br_cond_new(xjs_ir_var cond, xjs_ir_label dest)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_br_cond_ctor, \
            xjs_ir_text_item_br_cond_dtor);
    r->u.as_br_cond.cond = cond;
    r->u.as_br_cond.dest = dest;
    return r;
}


/* merge */

static void xjs_ir_text_item_merge_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_merge;
    r->u.as_merge.test = 0;
    r->u.as_merge.consequent = 0;
    r->u.as_merge.alternate = 0;
    r->u.as_merge.dst = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_merge_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_merge_new( \
        xjs_ir_var test, \
        xjs_ir_var consequent, \
        xjs_ir_var alternate, \
        xjs_ir_var dst)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_merge_ctor, \
            xjs_ir_text_item_merge_dtor);
    r->u.as_merge.test = test;
    r->u.as_merge.consequent = consequent;
    r->u.as_merge.alternate = alternate;
    r->u.as_merge.dst = dst;
    return r;
}


/* alloca */

static void xjs_ir_text_item_alloca_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_alloca;
    r->u.as_alloca.var = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_alloca_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_alloca_new(xjs_ir_var var)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_alloca_ctor, \
            xjs_ir_text_item_alloca_dtor);
    r->u.as_alloca.var = var;
    return r;
}


/* load_undefined */

static void xjs_ir_text_item_load_undefined_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_load_undefined;
    r->u.as_load_undefined.var = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_load_undefined_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_load_undefined_new(xjs_ir_var dest)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_load_undefined_ctor, \
            xjs_ir_text_item_load_undefined_dtor);
    r->u.as_load_undefined.var = dest;
    return r;
}


/* load_null */

static void xjs_ir_text_item_load_null_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_load_null;
    r->u.as_load_null.var = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_load_null_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_load_null_new(xjs_ir_var dest)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_load_null_ctor, \
            xjs_ir_text_item_load_null_dtor);
    r->u.as_load_null.var = dest;
    return r;
}


/* load_bool */

static void xjs_ir_text_item_load_bool_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_load_bool;
    r->u.as_load_bool.var = 0;
    r->u.as_load_bool.value = xjs_false;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_load_bool_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_load_bool_new(xjs_ir_var var, xjs_bool value)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_load_bool_ctor, \
            xjs_ir_text_item_load_bool_dtor);
    r->u.as_load_bool.var = var;
    r->u.as_load_bool.value = value;
    return r;
}


/* load_string */

static void xjs_ir_text_item_load_string_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_load_string;
    r->u.as_load_string.var = 0;
    r->u.as_load_string.dataid = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_load_string_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_load_string_new(xjs_ir_var var, xjs_ir_dataid dataid)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_load_string_ctor, \
            xjs_ir_text_item_load_string_dtor);
    r->u.as_load_string.var = var;
    r->u.as_load_string.dataid = dataid;
    return r;
}


/* load_number */

static void xjs_ir_text_item_load_number_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_load_number;
    r->u.as_load_number.var = 0;
    r->u.as_load_number.value = 0.0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_load_number_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_load_number_new(xjs_ir_var var, double value)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_load_number_ctor, \
            xjs_ir_text_item_load_number_dtor);
    r->u.as_load_number.var = var;
    r->u.as_load_number.value = value;
    return r;
}

/* load_object */

static void xjs_ir_text_item_load_object_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_load_object;
    r->u.as_load_object.var = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_load_object_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_load_object_new(xjs_ir_var var)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_load_object_ctor, \
            xjs_ir_text_item_load_object_dtor);
    r->u.as_load_object.var = var;
    return r;
}

/* load array */

static void xjs_ir_text_item_load_array_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_load_array;
    r->u.as_load_array.var = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_load_array_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_load_array_new(xjs_ir_var var)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_load_array_ctor, \
            xjs_ir_text_item_load_array_dtor);
    r->u.as_load_array.var = var;
    return r;
}

/* declvar */

static void xjs_ir_text_item_load_declvar_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_declvar;
    r->u.as_declvar.variable = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_load_declvar_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_declvar_new(xjs_ir_dataid variable)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_load_declvar_ctor, \
            xjs_ir_text_item_load_declvar_dtor);
    r->u.as_declvar.variable = variable;
    return r;
}


/* load */

static void xjs_ir_text_item_load_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_load;
    r->u.as_load.var = 0;
    r->u.as_load.variable = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_load_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_load_new(xjs_ir_var var, xjs_ir_dataid variable)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, xjs_ir_text_item_load_ctor, xjs_ir_text_item_load_dtor);
    r->u.as_load.var = var;
    r->u.as_load.variable = variable;
    return r;
}


/* store */

static void xjs_ir_text_item_store_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_store;
    r->u.as_store.variable = 0;
    r->u.as_store.var = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_store_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_store_new(xjs_ir_dataid variable, xjs_ir_var var)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_store_ctor, xjs_ir_text_item_store_dtor);
    r->u.as_store.variable = variable;
    r->u.as_store.var = var;
    return r;
}

/* object set */

static void xjs_ir_text_item_object_set_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_object_set;
    r->u.as_object_set.obj = 0;
    r->u.as_object_set.member = 0;
    r->u.as_object_set.src = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_object_set_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_object_set_new(xjs_ir_var dst, xjs_ir_var object, xjs_ir_var member, xjs_ir_var src)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_object_set_ctor, \
            xjs_ir_text_item_object_set_dtor);
    r->u.as_object_set.dst = dst;
    r->u.as_object_set.obj = object;
    r->u.as_object_set.member = member;
    r->u.as_object_set.src = src;
    return r;
}

/* object get */

static void xjs_ir_text_item_object_get_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_object_get;
    r->u.as_object_get.dst = 0;
    r->u.as_object_get.obj = 0;
    r->u.as_object_get.member = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_object_get_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_object_get_new(xjs_ir_var dst, xjs_ir_var object, xjs_ir_var member)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_object_get_ctor, \
            xjs_ir_text_item_object_get_dtor);
    r->u.as_object_get.dst = dst;
    r->u.as_object_get.obj = object;
    r->u.as_object_get.member = member;
    return r;
}

/* array push */

static void xjs_ir_text_item_array_push_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_array_push;
    r->u.as_array_push.arr = 0;
    r->u.as_array_push.elem = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_array_push_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_array_push_new(xjs_ir_var arr, xjs_ir_var elem)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_array_push_ctor, \
            xjs_ir_text_item_array_push_dtor);
    r->u.as_array_push.arr = arr;
    r->u.as_array_push.elem = elem;
    return r;
}

/* make function */

static void xjs_ir_text_item_make_function_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_make_function;
    r->u.as_make_function.var = 0;
    r->u.as_make_function.func = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_make_function_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_make_function_new(xjs_ir_var var, \
        xjs_ir_functionid func)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_make_function_ctor, \
            xjs_ir_text_item_make_function_dtor);
    r->u.as_make_function.var = var;
    r->u.as_make_function.func = func;
    return r;
}

/* make arrow function */

static void xjs_ir_text_item_make_arrow_function_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_make_arrow_function;
    r->u.as_make_arrow_function.var = 0;
    r->u.as_make_arrow_function.func = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_make_arrow_function_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_make_arrow_function_new(xjs_ir_var var, xjs_ir_functionid func)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_make_arrow_function_ctor, \
            xjs_ir_text_item_make_arrow_function_dtor);
    r->u.as_make_arrow_function.var = var;
    r->u.as_make_arrow_function.func = func;
    return r;
}

/* inspect */

static void xjs_ir_text_item_inspect_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_inspect;
    r->u.as_ret.var = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_inspect_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_inspect_new(xjs_ir_var var)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_inspect_ctor, \
            xjs_ir_text_item_inspect_dtor);
    r->u.as_inspect.var = var;
    return r;
}

/* ret */

static void xjs_ir_text_item_ret_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_ret;
    r->u.as_ret.var = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_ret_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_ret_new(xjs_ir_var var)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_ret_ctor, \
            xjs_ir_text_item_ret_dtor);
    r->u.as_ret.var = var;
    return r;
}

/* call */

static void xjs_ir_text_item_call_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_call;
    r->u.as_call.dst = 0;
    r->u.as_call.callee = 0;
    r->u.as_call.arguments = NULL;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_call_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->u.as_call.arguments);
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_call_new(xjs_ir_var dst, xjs_ir_var callee, xjs_ir_var_list_ref arguments)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, xjs_ir_text_item_call_ctor, xjs_ir_text_item_call_dtor);
    r->u.as_call.dst = dst;
    r->u.as_call.callee = callee;
    r->u.as_call.arguments = arguments;
    r->u.as_call.bound_this.enabled = xjs_false;
    return r;
}

xjs_ir_text_item_ref xjs_ir_text_item_call_bound_this_new(xjs_ir_var dst, xjs_ir_var callee, xjs_ir_var_list_ref arguments, xjs_ir_var _this)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, xjs_ir_text_item_call_ctor, xjs_ir_text_item_call_dtor);
    r->u.as_call.dst = dst;
    r->u.as_call.callee = callee;
    r->u.as_call.arguments = arguments;
    r->u.as_call.bound_this.enabled = xjs_true;
    r->u.as_call.bound_this._this = _this;
    return r;
}

/* new */

static void xjs_ir_text_item_new_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_new;
    r->u.as_new.dst = 0;
    r->u.as_new.callee = 0;
    r->u.as_new.arguments = NULL;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_new_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
    ec_delete(r->u.as_new.arguments);
}

xjs_ir_text_item_ref xjs_ir_text_item_new_new(xjs_ir_var dst, xjs_ir_var callee, xjs_ir_var_list_ref arguments)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, xjs_ir_text_item_new_ctor, xjs_ir_text_item_new_dtor);
    r->u.as_new.dst = dst;
    r->u.as_new.callee = callee;
    r->u.as_new.arguments = arguments;
    return r;
}

/* this */

static void xjs_ir_text_item_this_ctor(void *data)
{
    xjs_ir_text_item_ref r = data;
    r->type = xjs_ir_text_item_type_this;
    r->u.as_this.dst = 0;
    xjs_ir_text_item_loc_range_init(r);
}

static void xjs_ir_text_item_this_dtor(void *data)
{
    xjs_ir_text_item_ref r = data;
    ec_delete(r->loc.filename);
}

xjs_ir_text_item_ref xjs_ir_text_item_this_new(xjs_ir_var dst)
{
    xjs_ir_text_item_ref r = ec_newcd( \
            xjs_ir_text_item, \
            xjs_ir_text_item_this_ctor, \
            xjs_ir_text_item_this_dtor);
    r->u.as_this.dst = dst;
    return r;
}

/* binary_op */

#define DEFINE_BINARY_OP(name) \
static void xjs_ir_text_item_##name##_ctor(void *data) { \
    xjs_ir_text_item_ref r = data; \
    r->type = xjs_ir_text_item_type_##name; \
    r->u.as_binary_op.dst = r->u.as_binary_op.lhs = r->u.as_binary_op.rhs = 0; \
    xjs_ir_text_item_loc_range_init(r); \
} \
static void xjs_ir_text_item_##name##_dtor(void *data) { \
    xjs_ir_text_item_ref r = data; \
    ec_delete(r->loc.filename); \
} \
xjs_ir_text_item_ref xjs_ir_text_item_##name##_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs) { \
    xjs_ir_text_item_ref r = ec_newcd(xjs_ir_text_item, \
            xjs_ir_text_item_##name##_ctor, \
            xjs_ir_text_item_##name##_dtor); \
    r->u.as_binary_op.dst = dst; \
    r->u.as_binary_op.lhs = lhs; \
    r->u.as_binary_op.rhs = rhs; \
    return r; } \
xjs_ir_text_item_ref xjs_ir_text_item_##name##_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs)

DEFINE_BINARY_OP(binary_add);
DEFINE_BINARY_OP(binary_sub);
DEFINE_BINARY_OP(binary_mul);
DEFINE_BINARY_OP(binary_div);
DEFINE_BINARY_OP(binary_mod);
DEFINE_BINARY_OP(binary_e2);
DEFINE_BINARY_OP(binary_ne2);
DEFINE_BINARY_OP(binary_e3);
DEFINE_BINARY_OP(binary_ne3);
DEFINE_BINARY_OP(binary_l);
DEFINE_BINARY_OP(binary_le);
DEFINE_BINARY_OP(binary_g);
DEFINE_BINARY_OP(binary_ge);
DEFINE_BINARY_OP(binary_and);
DEFINE_BINARY_OP(binary_or);

/* unary_op */

#define DEFINE_UNARY_OP(name) \
static void xjs_ir_text_item_##name##_ctor(void *data) { \
    xjs_ir_text_item_ref r = data; \
    r->type = xjs_ir_text_item_type_##name; \
    r->u.as_unary_op.dst = 0; \
    r->u.as_unary_op.src = 0; \
    xjs_ir_text_item_loc_range_init(r); } \
static void xjs_ir_text_item_##name##_dtor(void *data) { \
    xjs_ir_text_item_ref r = data; \
    ec_delete(r->loc.filename); \
} \
xjs_ir_text_item_ref xjs_ir_text_item_##name##_new(xjs_ir_var dst, xjs_ir_var src) { \
    xjs_ir_text_item_ref r = ec_newcd(xjs_ir_text_item, \
            xjs_ir_text_item_##name##_ctor, \
            xjs_ir_text_item_##name##_dtor); \
    r->u.as_unary_op.dst = dst; \
    r->u.as_unary_op.src = src; \
    return r; } \
xjs_ir_text_item_ref xjs_ir_text_item_##name##_new(xjs_ir_var dst, xjs_ir_var src)

DEFINE_UNARY_OP(unary_not);
DEFINE_UNARY_OP(unary_bnot);
DEFINE_UNARY_OP(unary_add);
DEFINE_UNARY_OP(unary_sub);

static void xjs_ir_text_item_list_node_dtor(xjs_ir_text_item_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_ir_text_item_list, \
        xjs_ir_text_item_ref, xjs_ir_text_item_list_node_dtor);

xjs_ir_text_item_ref xjs_ir_text_item_clone(xjs_ir_text_item_ref ti)
{
    xjs_ir_text_item_ref new_ti = NULL;

    switch (ti->type)
    {
        case xjs_ir_text_item_type_nop:
            new_ti = xjs_ir_text_item_nop_new();
            break;
        case xjs_ir_text_item_type_halt:
            new_ti = xjs_ir_text_item_halt_new();
            break;
        case xjs_ir_text_item_type_inspect:
            new_ti = xjs_ir_text_item_inspect_new(ti->u.as_inspect.var);
            break;
        case xjs_ir_text_item_type_dynlib:
            new_ti = xjs_ir_text_item_dynlib_new(ti->u.as_dynlib.exports, ti->u.as_dynlib.module_name);
            break;
        case xjs_ir_text_item_type_push_scope:
        case xjs_ir_text_item_type_pop_scope:
            break;
        case xjs_ir_text_item_type_label:
            new_ti = xjs_ir_text_item_label_new(ti->u.as_label.lbl);
            break;
        case xjs_ir_text_item_type_br:
            new_ti = xjs_ir_text_item_br_new(ti->u.as_br.dest);
            break;
        case xjs_ir_text_item_type_br_cond:
            new_ti = xjs_ir_text_item_br_cond_new( \
                    ti->u.as_br_cond.cond, ti->u.as_br_cond.dest);
            break;
        case xjs_ir_text_item_type_merge:
            new_ti = xjs_ir_text_item_merge_new( \
                    ti->u.as_merge.test, \
                    ti->u.as_merge.consequent, \
                    ti->u.as_merge.alternate, \
                    ti->u.as_merge.dst);
            break;
        case xjs_ir_text_item_type_alloca:
            new_ti = xjs_ir_text_item_alloca_new( \
                    ti->u.as_alloca.var);
            break;
        case xjs_ir_text_item_type_load_undefined:
            new_ti = xjs_ir_text_item_load_undefined_new(ti->u.as_load_undefined.var);
            break;
        case xjs_ir_text_item_type_load_null:
            new_ti = xjs_ir_text_item_load_null_new(ti->u.as_load_null.var);
            break;
        case xjs_ir_text_item_type_load_bool:
            new_ti = xjs_ir_text_item_load_bool_new( \
                    ti->u.as_load_bool.var, ti->u.as_load_bool.value);
            break;
        case xjs_ir_text_item_type_load_string:
            new_ti = xjs_ir_text_item_load_string_new( \
                    ti->u.as_load_string.var, ti->u.as_load_string.dataid);
            break;
        case xjs_ir_text_item_type_load_number:
            new_ti = xjs_ir_text_item_load_number_new( \
                    ti->u.as_load_number.var, ti->u.as_load_number.value);
            break;
        case xjs_ir_text_item_type_load_object:
            new_ti = xjs_ir_text_item_load_object_new( \
                    ti->u.as_load_object.var);
            break;
        case xjs_ir_text_item_type_load_array:
            new_ti = xjs_ir_text_item_load_array_new( \
                    ti->u.as_load_array.var);
            break;
        case xjs_ir_text_item_type_declvar:
            new_ti = xjs_ir_text_item_declvar_new( \
                    ti->u.as_declvar.variable);
            break;
        case xjs_ir_text_item_type_load:
            new_ti = xjs_ir_text_item_load_new( \
                    ti->u.as_load.var, ti->u.as_load.variable);
            break;
        case xjs_ir_text_item_type_store:
            new_ti = xjs_ir_text_item_store_new( \
                    ti->u.as_store.variable, ti->u.as_store.var);
            break;
        case xjs_ir_text_item_type_object_set:
            new_ti = xjs_ir_text_item_object_set_new( \
                    ti->u.as_object_set.dst, \
                    ti->u.as_object_set.obj, \
                    ti->u.as_object_set.member, \
                    ti->u.as_object_set.src);
            break;
        case xjs_ir_text_item_type_object_get:
            new_ti = xjs_ir_text_item_object_get_new( \
                    ti->u.as_object_get.dst, \
                    ti->u.as_object_get.obj, \
                    ti->u.as_object_get.member);
            break;
        case xjs_ir_text_item_type_array_push:
            new_ti = xjs_ir_text_item_array_push_new( \
                    ti->u.as_array_push.arr, ti->u.as_array_push.elem);
            break;
        case xjs_ir_text_item_type_make_function:
            new_ti = xjs_ir_text_item_make_function_new( \
                    ti->u.as_make_function.var, ti->u.as_make_function.func);
            break;
        case xjs_ir_text_item_type_make_arrow_function:
            new_ti = xjs_ir_text_item_make_arrow_function_new( \
                    ti->u.as_make_arrow_function.var, ti->u.as_make_arrow_function.func);
            break;
        case xjs_ir_text_item_type_ret:
            new_ti = xjs_ir_text_item_ret_new(ti->u.as_ret.var);
            break;
        case xjs_ir_text_item_type_call:
            if (ti->u.as_call.bound_this.enabled == xjs_false)
            {
                new_ti = xjs_ir_text_item_call_new( \
                        ti->u.as_call.dst, \
                        ti->u.as_call.callee, \
                        xjs_ir_var_list_clone(ti->u.as_call.arguments));
            }
            else
            {
                new_ti = xjs_ir_text_item_call_bound_this_new( \
                        ti->u.as_call.dst, \
                        ti->u.as_call.callee, \
                        xjs_ir_var_list_clone(ti->u.as_call.arguments), \
                        ti->u.as_call.bound_this._this);
            }
            break;
        case xjs_ir_text_item_type_new:
            new_ti = xjs_ir_text_item_new_new( \
                    ti->u.as_new.dst, \
                    ti->u.as_new.callee, \
                    xjs_ir_var_list_clone(ti->u.as_call.arguments));
            break;
        case xjs_ir_text_item_type_this:
            new_ti = xjs_ir_text_item_this_new(ti->u.as_this.dst);
            break;
        case xjs_ir_text_item_type_binary_add:
            new_ti = xjs_ir_text_item_binary_add_new( \
                    ti->u.as_binary_op.dst, ti->u.as_binary_op.lhs, ti->u.as_binary_op.rhs);
            break;
        case xjs_ir_text_item_type_binary_sub:
            new_ti = xjs_ir_text_item_binary_sub_new( \
                    ti->u.as_binary_op.dst, ti->u.as_binary_op.lhs, ti->u.as_binary_op.rhs);
            break;
        case xjs_ir_text_item_type_binary_mul:
            new_ti = xjs_ir_text_item_binary_mul_new( \
                    ti->u.as_binary_op.dst, ti->u.as_binary_op.lhs, ti->u.as_binary_op.rhs);
            break;
        case xjs_ir_text_item_type_binary_div:
            new_ti = xjs_ir_text_item_binary_div_new( \
                    ti->u.as_binary_op.dst, ti->u.as_binary_op.lhs, ti->u.as_binary_op.rhs);
            break;
        case xjs_ir_text_item_type_binary_mod:
            new_ti = xjs_ir_text_item_binary_mod_new( \
                    ti->u.as_binary_op.dst, ti->u.as_binary_op.lhs, ti->u.as_binary_op.rhs);
            break;
        case xjs_ir_text_item_type_binary_e2:
            new_ti = xjs_ir_text_item_binary_e2_new( \
                    ti->u.as_binary_op.dst, ti->u.as_binary_op.lhs, ti->u.as_binary_op.rhs);
            break;
        case xjs_ir_text_item_type_binary_ne2:
            new_ti = xjs_ir_text_item_binary_ne2_new( \
                    ti->u.as_binary_op.dst, ti->u.as_binary_op.lhs, ti->u.as_binary_op.rhs);
            break;
        case xjs_ir_text_item_type_binary_e3:
            new_ti = xjs_ir_text_item_binary_e3_new( \
                    ti->u.as_binary_op.dst, ti->u.as_binary_op.lhs, ti->u.as_binary_op.rhs);
            break;
        case xjs_ir_text_item_type_binary_ne3:
            new_ti = xjs_ir_text_item_binary_ne3_new( \
                    ti->u.as_binary_op.dst, ti->u.as_binary_op.lhs, ti->u.as_binary_op.rhs);
            break;
        case xjs_ir_text_item_type_binary_l:
            new_ti = xjs_ir_text_item_binary_l_new( \
                    ti->u.as_binary_op.dst, ti->u.as_binary_op.lhs, ti->u.as_binary_op.rhs);
            break;
        case xjs_ir_text_item_type_binary_le:
            new_ti = xjs_ir_text_item_binary_le_new( \
                    ti->u.as_binary_op.dst, ti->u.as_binary_op.lhs, ti->u.as_binary_op.rhs);
            break;
        case xjs_ir_text_item_type_binary_g:
            new_ti = xjs_ir_text_item_binary_g_new( \
                    ti->u.as_binary_op.dst, ti->u.as_binary_op.lhs, ti->u.as_binary_op.rhs);
            break;
        case xjs_ir_text_item_type_binary_ge:
            new_ti = xjs_ir_text_item_binary_ge_new( \
                    ti->u.as_binary_op.dst, ti->u.as_binary_op.lhs, ti->u.as_binary_op.rhs);
            break;
        case xjs_ir_text_item_type_binary_and:
            new_ti = xjs_ir_text_item_binary_and_new( \
                    ti->u.as_binary_op.dst, ti->u.as_binary_op.lhs, ti->u.as_binary_op.rhs);
            break;
        case xjs_ir_text_item_type_binary_or:
            new_ti = xjs_ir_text_item_binary_or_new( \
                    ti->u.as_binary_op.dst, ti->u.as_binary_op.lhs, ti->u.as_binary_op.rhs);
            break;
        case xjs_ir_text_item_type_unary_not:
            new_ti = xjs_ir_text_item_unary_not_new( \
                    ti->u.as_unary_op.dst, ti->u.as_unary_op.src);
            break;
        case xjs_ir_text_item_type_unary_bnot:
            new_ti = xjs_ir_text_item_unary_bnot_new( \
                    ti->u.as_unary_op.dst, ti->u.as_unary_op.src);
            break;
        case xjs_ir_text_item_type_unary_add:
            new_ti = xjs_ir_text_item_unary_add_new( \
                    ti->u.as_unary_op.dst, ti->u.as_unary_op.src);
            break;
        case xjs_ir_text_item_type_unary_sub:
            new_ti = xjs_ir_text_item_unary_sub_new( \
                    ti->u.as_unary_op.dst, ti->u.as_unary_op.src);
            break;
    }
    if (new_ti != NULL)
    {
        if (ti->loc.filename != NULL)
        {
            new_ti->loc.filename = ec_string_clone(ti->loc.filename);
        }
        else
        {
            new_ti->loc.filename = NULL;
        }
        new_ti->loc.start.ln = ti->loc.start.ln;
        new_ti->loc.start.col = ti->loc.start.col;
        new_ti->loc.end.ln = ti->loc.end.ln;
        new_ti->loc.end.col = ti->loc.end.col;
        new_ti->range.start = ti->range.start;
        new_ti->range.end = ti->range.end;
    }

    return new_ti;
}

xjs_ir_export_item_ref xjs_ir_export_item_new( \
        xjs_ir_dataid exported, \
        xjs_ir_dataid local)
{
    xjs_ir_export_item_ref r = ec_newcd( \
            xjs_ir_export_item, NULL, NULL);
    r->local = local;
    r->exported = exported;
    return r;
}

xjs_ir_import_item_ref xjs_ir_import_item_new( \
        xjs_ir_dataid local, \
        xjs_ir_dataid imported, \
        xjs_ir_dataid source)
{
    xjs_ir_import_item_ref r = ec_newcd( \
            xjs_ir_import_item, NULL, NULL);
    r->local = local;
    r->imported = imported;
    r->source = source;
    return r;
}

static void xjs_ir_export_list_node_dtor(xjs_ir_export_item_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_ir_export_list, \
        xjs_ir_export_item_ref, xjs_ir_export_list_node_dtor);

static void xjs_ir_import_list_node_dtor(xjs_ir_import_item_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_ir_import_list, \
        xjs_ir_import_item_ref, xjs_ir_import_list_node_dtor);

static void xjs_ir_parameter_list_node_dtor(xjs_ir_parameter_ref node)
{
    ec_delete(node);
}

/* Parameter */

static void xjs_ir_parameter_ctor(void *data)
{
    xjs_ir_parameter_ref r = data;
    r->varname = 0;
    r->loc.start.ln = -1;
    r->loc.start.col = -1;
    r->loc.end.ln = -1;
    r->loc.end.col = -1;
}

xjs_ir_parameter_ref xjs_ir_parameter_new(xjs_ir_dataid varname)
{
    xjs_ir_parameter_ref r = ec_newcd( \
            xjs_ir_parameter, \
            xjs_ir_parameter_ctor, \
            NULL);
    r->varname = varname;
    return r;
}

ect_list_define_declared(xjs_ir_parameter_list, \
        xjs_ir_parameter_ref, xjs_ir_parameter_list_node_dtor);

/* IR : Function */

static void xjs_ir_function_list_node_dtor(xjs_ir_function_ref r)
{
    ec_delete(r);
}

ect_list_define_declared(xjs_ir_function_list, \
        xjs_ir_function_ref, xjs_ir_function_list_node_dtor);

static void xjs_ir_function_ctor(void *data)
{
    xjs_ir_function_ref r = data;
    r->parameters = xjs_ir_parameter_list_new();
    r->text_items = xjs_ir_text_item_list_new();
}

static void xjs_ir_function_dtor(void *data)
{
    xjs_ir_function_ref r = data;
    ec_delete(r->parameters);
    ec_delete(r->text_items);
}

xjs_ir_function_ref xjs_ir_function_new(void)
{
    xjs_ir_function_ref r = ec_newcd( \
            xjs_ir_function, \
            xjs_ir_function_ctor, \
            xjs_ir_function_dtor);
    return r;
}

void xjs_ir_function_append_parameter(xjs_ir_function_ref func, \
        xjs_ir_parameter_ref parameter)
{
    xjs_ir_parameter_list_push_back(func->parameters, parameter);
}

void xjs_ir_function_append_text_item(xjs_ir_function_ref func, \
        xjs_ir_text_item_ref text_item)
{
    xjs_ir_text_item_list_push_back(func->text_items, text_item);
}

xjs_ir_function_ref xjs_ir_function_clone(xjs_ir_function_ref fn)
{
    xjs_ir_function_ref new_fn = xjs_ir_function_new();
    if (new_fn == NULL) return NULL;

    /* Parameters */
    {
        ect_iterator(xjs_ir_parameter_list) it_param;
        ect_for(xjs_ir_parameter_list, fn->parameters, it_param)
        {
            xjs_ir_parameter_ref param = ect_deref(xjs_ir_parameter_ref, it_param);
            ect_list_push_back(xjs_ir_parameter_list, new_fn->parameters, xjs_ir_parameter_new(param->varname));
        }
    }

    /* Instructions */
    {
        ect_iterator(xjs_ir_text_item_list) it_ti;
        ect_for(xjs_ir_text_item_list, fn->text_items, it_ti)
        {
            xjs_ir_text_item_ref ti = ect_deref(xjs_ir_text_item_ref, it_ti);
            ect_list_push_back(xjs_ir_text_item_list, new_fn->text_items, xjs_ir_text_item_clone(ti));
        }
    }

    return new_fn;
}


/* IR : Text */

static void xjs_ir_text_ctor(void *text)
{
    xjs_ir_text_ref r = text;
    r->functions = xjs_ir_function_list_new();
}

static void xjs_ir_text_dtor(void *text)
{
    xjs_ir_text_ref r = text;
    ec_delete(r->functions);
}

static xjs_ir_text_ref xjs_ir_text_new(void)
{
    xjs_ir_text_ref r = ec_newcd(xjs_ir_text, xjs_ir_text_ctor, xjs_ir_text_dtor);
    return r;
}

/* IR */

static void xjs_ir_ctor(void *data)
{
    xjs_ir_ref r = data;
    r->data = xjs_ir_data_new();
    r->text = xjs_ir_text_new();
    r->exported = xjs_ir_export_list_new();
    r->imported = xjs_ir_import_list_new();
    r->module.name = NULL;
    r->module.fullpath = NULL;
    r->toplevel = NULL;
}

static void xjs_ir_dtor(void *data)
{
    xjs_ir_ref r = data;
    ec_delete(r->data);
    ec_delete(r->text);
    ec_delete(r->exported);
    ec_delete(r->imported);
    if (r->module.name != NULL) ec_free(r->module.name);
    if (r->module.fullpath != NULL) ec_free(r->module.fullpath);
}

xjs_ir_ref xjs_ir_new(void)
{
    xjs_ir_ref r = ec_newcd(xjs_ir, xjs_ir_ctor, xjs_ir_dtor);
    return r;
}

void xjs_ir_append_data_item(xjs_ir_ref ir, \
        xjs_ir_data_item_ref data_item)
{
    xjs_ir_data_item_list_push_back(ir->data->items, data_item);
}

xjs_ir_data_item_ref xjs_ir_get_data_item_by_dataid(xjs_ir_ref ir, \
        xjs_ir_dataid dataid)
{
    ect_iterator(xjs_ir_data_item_list) it;
    ect_for(xjs_ir_data_item_list, ir->data->items, it)
    {
        xjs_ir_data_item_ref item = ect_deref(xjs_ir_data_item_ref, it);
        if (item->dataid == dataid) { return item; }
    }
    return NULL;
}

xjs_ir_data_item_ref xjs_ir_get_data_item_by_string(xjs_ir_ref ir, \
        ec_string *s)
{
    ect_iterator(xjs_ir_data_item_list) it;
    ect_for(xjs_ir_data_item_list, ir->data->items, it)
    {
        xjs_ir_data_item_ref item = ect_deref(xjs_ir_data_item_ref, it);
        if (item->type == xjs_ir_data_item_type_string)
        {
            if (ec_string_cmp(item->u.as_string.value, s) == 0)
            { return item; }
        }
    }
    return NULL;
}

void xjs_ir_module_name_set( \
        xjs_ir_ref ir, const char *name, const xjs_size_t name_len)
{
    if (ir->module.name != NULL)
    { ec_free(ir->module.name); ir->module.name = NULL; }

    if ((ir->module.name = (char *)ec_malloc(sizeof(char) * (name_len + 1))) == NULL)
    { return ; }
    ec_memcpy(ir->module.name, name, name_len);
    ir->module.name[name_len] = '\0';
}

void xjs_ir_module_fullpath_set( \
        xjs_ir_ref ir, const char *fullpath, const xjs_size_t fullpath_len)
{
    if (ir->module.fullpath != NULL)
    { ec_free(ir->module.fullpath); ir->module.fullpath = NULL; }

    if ((ir->module.fullpath = (char *)ec_malloc(sizeof(char) * (fullpath_len + 1))) == NULL)
    { return ; }
    ec_memcpy(ir->module.fullpath, fullpath, fullpath_len);
    ir->module.fullpath[fullpath_len] = '\0';
}

