/* XenonJS : IR Builder
 * Copyright(c) 2017 y2c2 */

#ifdef __clang__
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif

#include <ec_algorithm.h>
#include <ec_set.h>
#include <ec_map.h>
#include "xjs_ir.h"
#include "xjs_irbuilder.h"

/* CFG Node Set */

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

ect_set_define_declared(xjs_cfg_node_ref_set, xjs_cfg_node_ref, \
        xjs_cfg_node_set_node_ctor, \
        xjs_cfg_node_set_node_dtor, \
        xjs_cfg_node_set_node_cmp);

/* Symbol Map & String Map */

static int xjs_ir_string_map_key_ctor(ec_string **detta_key, ec_string **key)
{
    *detta_key = *key;
    return 0;
}

static void xjs_ir_string_map_key_dtor(ec_string **detta_key)
{
    ec_delete(*detta_key);
}

static int xjs_ir_string_map_key_cmp(ec_string **a, ec_string **b)
{
    return ec_string_cmp(*a, *b);
}

static int xjs_ir_string_map_value_ctor(xjs_ir_dataid *detta_key, xjs_ir_dataid *key)
{
    *detta_key = *key;
    return 0;
}

ect_map_define_declared(xjs_ir_string_ir_dataid_map, \
        ec_string *, xjs_ir_string_map_key_ctor, xjs_ir_string_map_key_dtor, \
        xjs_ir_dataid, xjs_ir_string_map_value_ctor, NULL, \
        xjs_ir_string_map_key_cmp);

static int xjs_ir_symbol_map_key_ctor(ec_string **detta_key, ec_string **key)
{
    *detta_key = *key;
    return 0;
}

static void xjs_ir_symbol_map_key_dtor(ec_string **detta_key)
{
    ec_delete(*detta_key);
}

static int xjs_ir_symbol_map_key_cmp(ec_string **a, ec_string **b)
{
    return ec_string_cmp(*a, *b);
}

static int xjs_ir_symbol_map_value_ctor(xjs_ir_dataid *detta_value, xjs_ir_dataid *value)
{
    *detta_value = *value;
    return 0;
}

ect_map_define_declared(xjs_ir_symbol_ir_dataid_map, \
        ec_string *, xjs_ir_symbol_map_key_ctor, xjs_ir_symbol_map_key_dtor, \
        xjs_ir_dataid, xjs_ir_symbol_map_value_ctor, NULL, \
        xjs_ir_symbol_map_key_cmp);

/* IR dataid Remap */

static int xjs_ir_dataid_map_key_ctor(xjs_ir_dataid *detta_key, xjs_ir_dataid *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_ir_dataid_map_key_cmp(xjs_ir_dataid *a, xjs_ir_dataid *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

ect_map_define_declared(xjs_ir_dataid_map, \
        xjs_ir_dataid, xjs_ir_dataid_map_key_ctor, NULL, \
        xjs_ir_dataid, xjs_ir_dataid_map_key_ctor, NULL, \
        xjs_ir_dataid_map_key_cmp);

/* IR var Remap */

static int xjs_ir_var_map_key_ctor(xjs_ir_var *detta_key, xjs_ir_var *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_ir_var_map_key_cmp(xjs_ir_var *a, xjs_ir_var *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

ect_map_define_declared(xjs_ir_var_map, \
        xjs_ir_var, xjs_ir_var_map_key_ctor, NULL, \
        xjs_ir_var, xjs_ir_var_map_key_ctor, NULL, \
        xjs_ir_var_map_key_cmp);

/* IR label Remap */

static int xjs_ir_label_map_key_ctor(xjs_ir_label *detta_key, xjs_ir_label *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_ir_label_map_key_cmp(xjs_ir_label *a, xjs_ir_label *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

ect_map_define_declared(xjs_ir_label_map, \
        xjs_ir_label, xjs_ir_label_map_key_ctor, NULL, \
        xjs_ir_label, xjs_ir_label_map_key_ctor, NULL, \
        xjs_ir_label_map_key_cmp);

/* CFG var -> IR var Mapping */

static int xjs_cfg_var_ir_var_map_key_ctor(xjs_cfg_var *detta_key, xjs_cfg_var *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_cfg_var_ir_var_map_value_ctor(xjs_ir_var *detta_value, xjs_ir_var *value)
{
    *detta_value = *value;
    return 0;
}

static int xjs_cfg_var_ir_var_map_key_cmp(xjs_cfg_var *a, xjs_cfg_var *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

ect_map_define_declared(xjs_cfg_var_ir_var_map, \
        xjs_cfg_var, xjs_cfg_var_ir_var_map_key_ctor, NULL, \
        xjs_ir_var, xjs_cfg_var_ir_var_map_value_ctor, NULL, \
        xjs_cfg_var_ir_var_map_key_cmp);

/* CFG node -> IR label Mapping */

static int xjs_cfg_node_ir_label_map_key_ctor(xjs_cfg_node_ref *detta_key, xjs_cfg_node_ref *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_cfg_node_ir_label_map_key_cmp(xjs_cfg_node_ref *a, xjs_cfg_node_ref *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

static int xjs_cfg_node_ir_label_map_value_ctor(xjs_ir_label *detta_value, xjs_ir_label *value)
{
    *detta_value = *value;
    return 0;
}

ect_map_define_declared(xjs_cfg_node_ir_label_map, \
        xjs_cfg_node_ref, xjs_cfg_node_ir_label_map_key_ctor, NULL, \
        xjs_ir_label, xjs_cfg_node_ir_label_map_value_ctor, NULL, \
        xjs_cfg_node_ir_label_map_key_cmp);

/* IR Function Builder */

static void xjs_irbuilder_function_ctor(void *data)
{
    xjs_irbuilder_function_ref r = data;
    r->f = xjs_ir_function_new();
    r->var_pool = 0;
    r->label_pool = 0;
    r->var_lookup_tbl = ect_map_new(xjs_cfg_var_ir_var_map);
    r->label_lookup_tbl = ect_map_new(xjs_cfg_node_ir_label_map);
}

static void xjs_irbuilder_function_dtor(void *data)
{
    xjs_irbuilder_function_ref r = data;
    ec_delete(r->f);
    ec_delete(r->var_lookup_tbl);
    ec_delete(r->label_lookup_tbl);
}

xjs_irbuilder_function_ref xjs_irbuilder_function_new(void)
{
    xjs_irbuilder_function_ref r = ec_newcd(xjs_irbuilder_function, \
            xjs_irbuilder_function_ctor, xjs_irbuilder_function_dtor);
    return r;
}

int xjs_irbuilder_function_push_back_parameter( \
        xjs_irbuilder_function_ref irfb, \
        xjs_ir_dataid parameter_dataid)
{
    xjs_ir_parameter_ref new_parameter;

    if ((new_parameter = xjs_ir_parameter_new(parameter_dataid)) == NULL)
    { return -1; }
    xjs_ir_parameter_list_push_back(irfb->f->parameters, new_parameter);

    return 0;
}

int xjs_irbuilder_function_push_back_parameter_with_loc_range( \
        xjs_irbuilder_function_ref irfb, \
        xjs_ir_dataid parameter_dataid, \
        int start_ln, int start_col, \
        int end_ln, int end_col, \
        int range_start, \
        int range_end)
{
    xjs_ir_parameter_ref new_parameter;

    if ((new_parameter = xjs_ir_parameter_new(parameter_dataid)) == NULL)
    { return -1; }
    new_parameter->loc.start.ln = start_ln;
    new_parameter->loc.start.col = start_col;
    new_parameter->loc.end.ln = end_ln;
    new_parameter->loc.end.col = end_col;
    new_parameter->range.start = range_start;
    new_parameter->range.end = range_end;
    xjs_ir_parameter_list_push_back(irfb->f->parameters, new_parameter);

    return 0;
}

/* Allocate a label */
xjs_ir_label xjs_irbuilder_function_allocate_label(xjs_irbuilder_function_ref irbf)
{
    xjs_ir_label label = irbf->label_pool++;
    return label;
}

/* Map a CFG Node to an IR label */
void xjs_irbuilder_function_map_label( \
        xjs_irbuilder_function_ref irbf, \
        xjs_cfg_node_ref node, xjs_ir_label ir_label)
{
    ect_map_insert(xjs_cfg_node_ir_label_map, irbf->label_lookup_tbl, node, ir_label);
}

/* Find IR label by CFG node */
xjs_ir_label xjs_irbuilder_function_find_label( \
        xjs_irbuilder_function_ref irbf, \
        xjs_cfg_node_ref node)
{
    return ect_map_get(xjs_cfg_node_ir_label_map, irbf->label_lookup_tbl, node);
}

/* Allocate a variable */
xjs_ir_var xjs_irbuilder_function_allocate_var(xjs_irbuilder_function_ref irbf)
{
    xjs_ir_var var = irbf->var_pool++;
    return var;
}

/* If a CFG has been mapped */
ec_bool xjs_irbuilder_function_mapped( \
        xjs_irbuilder_function_ref irbf, \
        xjs_cfg_var cfg_var)
{
    return ect_map_count( \
            xjs_cfg_var_ir_var_map, \
            irbf->var_lookup_tbl, \
            cfg_var) == 0 ? ec_false : ec_true;
}

/* Map a CFG var to an IR var */
void xjs_irbuilder_function_map_var( \
        xjs_irbuilder_function_ref irbf, \
        xjs_cfg_var cfg_var, xjs_ir_var ir_var)
{
    ect_map_insert(xjs_cfg_var_ir_var_map, irbf->var_lookup_tbl, cfg_var, ir_var);
}

/* Find IR var by CFG var */
xjs_ir_var xjs_irbuilder_function_find_var( \
        xjs_irbuilder_function_ref irbf, \
        xjs_cfg_var cfg_var)
{
    return ect_map_get(xjs_cfg_var_ir_var_map, irbf->var_lookup_tbl, cfg_var);
}

void xjs_irbuilder_function_push_back_text_item( \
        xjs_irbuilder_function_ref irbf, \
        xjs_ir_text_item_ref text_item)
{
    xjs_ir_text_item_list_push_back(irbf->f->text_items, text_item);
}

/* Generate Function */
xjs_ir_function_ref xjs_irbuilder_function_generate(xjs_irbuilder_function_ref irbf)
{
    xjs_ir_function_ref r = irbf->f;
    irbf->f = NULL;
    return r;
}

/* IR Builder */

static void xjs_irbuilder_ctor(void *data)
{
    xjs_irbuilder_ref r = data;
    r->ir = xjs_ir_new();
    r->dataid_pool = 0;
    r->string_map = xjs_ir_string_ir_dataid_map_new();
    r->symbol_map = xjs_ir_symbol_ir_dataid_map_new();
}

static void xjs_irbuilder_dtor(void *data)
{
    xjs_irbuilder_ref r = data;
    ec_delete(r->ir);
    ec_delete(r->string_map);
    ec_delete(r->symbol_map);
}

xjs_irbuilder_ref xjs_irbuilder_new(void)
{
    xjs_irbuilder_ref r = ec_newcd(xjs_irbuilder, \
            xjs_irbuilder_ctor, xjs_irbuilder_dtor);
    return r;
}

void xjs_irbuilder_append_function( \
        xjs_irbuilder_ref irb, \
        xjs_ir_function_ref func)
{
    xjs_ir_function_list_push_back( \
            irb->ir->text->functions, \
            func);
}

void xjs_irbuilder_append_exported_symbol( \
        xjs_irbuilder_ref irb, \
        xjs_ir_dataid exported, \
        xjs_ir_dataid local)
{
    xjs_ir_export_item_ref r = xjs_ir_export_item_new( \
            exported, local);
    xjs_ir_export_list_push_back( \
            irb->ir->exported, r);
}

void xjs_irbuilder_append_imported_symbol( \
        xjs_irbuilder_ref irb, \
        xjs_ir_dataid local, \
        xjs_ir_dataid imported, \
        xjs_ir_dataid source)
{
    xjs_ir_import_item_ref r = xjs_ir_import_item_new( \
            local, imported, source);
    xjs_ir_import_list_push_back( \
            irb->ir->imported, r);
}

void xjs_irbuilder_push_back_data_item( \
        xjs_irbuilder_ref irb, \
        xjs_ir_data_item_ref data_item)
{
    xjs_ir_data_item_list_push_back(irb->ir->data->items, data_item);
}

/* Set toplevel function */
void xjs_irbuilder_set_toplevel_function( \
        xjs_irbuilder_ref irb, \
        xjs_ir_function_ref func)
{
    irb->ir->toplevel = func;
}

xjs_ir_ref xjs_irbuilder_generate_ir(xjs_irbuilder_ref irb)
{
    xjs_ir_ref ret = irb->ir;
    irb->ir = NULL;
    return ret;
}


