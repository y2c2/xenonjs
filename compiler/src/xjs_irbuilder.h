/* XenonJS : IR Builder
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_IRBUILDER_H
#define XJS_IRBUILDER_H

#include <ec_string.h>
#include <ec_set.h>
#include <ec_map.h>
#include "xjs_types.h"

struct xjs_opaque_irbuilder;
typedef struct xjs_opaque_irbuilder xjs_irbuilder;
typedef struct xjs_opaque_irbuilder *xjs_irbuilder_ref;

struct xjs_opaque_irbuilder_function;
typedef struct xjs_opaque_irbuilder_function xjs_irbuilder_function;
typedef struct xjs_opaque_irbuilder_function *xjs_irbuilder_function_ref;

/* CFG Node Set */
ect_set_declare(xjs_cfg_node_ref_set, xjs_cfg_node_ref);

/* Symbol Map */
ect_map_declare(xjs_ir_symbol_ir_dataid_map, ec_string *, xjs_ir_dataid);

/* String Map */
ect_map_declare(xjs_ir_string_ir_dataid_map, ec_string *, xjs_ir_dataid);

/* IR data id remap */
ect_map_declare(xjs_ir_dataid_map, xjs_ir_dataid, xjs_ir_dataid);

/* IR var Remap */
ect_map_declare(xjs_ir_var_map, xjs_ir_var, xjs_ir_var);

/* IR label Remap */
ect_map_declare(xjs_ir_label_map, xjs_ir_label, xjs_ir_label);

/* CFG var -> IR var Mapping */
ect_map_declare(xjs_cfg_var_ir_var_map, xjs_cfg_var, xjs_ir_var);

/* CFG node -> IR label Mapping */
ect_map_declare(xjs_cfg_node_ir_label_map, xjs_cfg_node_ref, xjs_ir_label);

/* IR Function Builder */

struct xjs_opaque_irbuilder_function
{
    xjs_ir_function_ref f;
    xjs_ir_var var_pool;
    xjs_ir_label label_pool;
    xjs_cfg_var_ir_var_map *var_lookup_tbl;
    xjs_cfg_node_ir_label_map *label_lookup_tbl;
};

/* Create Function Builder */
xjs_irbuilder_function_ref xjs_irbuilder_function_new(void);
int xjs_irbuilder_function_push_back_parameter( \
        xjs_irbuilder_function_ref irfb, \
        xjs_ir_dataid parameter_dataid);
int xjs_irbuilder_function_push_back_parameter_with_loc_range( \
        xjs_irbuilder_function_ref irfb, \
        xjs_ir_dataid parameter_dataid, \
        int start_ln, int start_col, \
        int end_ln, int end_col, \
        int range_start, \
        int range_end);

/* Allocate a label */
xjs_ir_label xjs_irbuilder_function_allocate_label(xjs_irbuilder_function_ref irbf);

/* Map a CFG Node to an IR label */
void xjs_irbuilder_function_map_label( \
        xjs_irbuilder_function_ref irbf, \
        xjs_cfg_node_ref node, xjs_ir_label ir_label);

/* Find IR label by CFG node */
xjs_ir_label xjs_irbuilder_function_find_label( \
        xjs_irbuilder_function_ref irbf, \
        xjs_cfg_node_ref node);

/* Allocate a variable */
xjs_ir_var xjs_irbuilder_function_allocate_var(xjs_irbuilder_function_ref irbf);

/* If a CFG has been mapped */
ec_bool xjs_irbuilder_function_mapped( \
        xjs_irbuilder_function_ref irbf, \
        xjs_cfg_var cfg_var);

/* Map a CFG var to an IR var */
void xjs_irbuilder_function_map_var( \
        xjs_irbuilder_function_ref irbf, \
        xjs_cfg_var cfg_var, xjs_ir_var ir_var);

/* Find IR var by CFG var */
xjs_ir_var xjs_irbuilder_function_find_var( \
        xjs_irbuilder_function_ref irbf, \
        xjs_cfg_var cfg_var);

/* Push text item into function  */
void xjs_irbuilder_function_push_back_text_item( \
        xjs_irbuilder_function_ref irbf, \
        xjs_ir_text_item_ref text_item);

/* Generate Function */
xjs_ir_function_ref xjs_irbuilder_function_generate(xjs_irbuilder_function_ref irbf);

/* IR Builder */

struct xjs_opaque_irbuilder
{
    xjs_ir_symbol_ir_dataid_map *symbol_map;
    xjs_ir_string_ir_dataid_map *string_map;
    xjs_ir_dataid dataid_pool;
    xjs_ir_ref ir;
};

/* Create Builder */
xjs_irbuilder_ref xjs_irbuilder_new(void);

/* Append a new function */
void xjs_irbuilder_append_function( \
        xjs_irbuilder_ref irb, \
        xjs_ir_function_ref func);

/* Append an exported symbol */
void xjs_irbuilder_append_exported_symbol( \
        xjs_irbuilder_ref irb, \
        xjs_ir_dataid exported, \
        xjs_ir_dataid local);

/* Append an imported symbol */
void xjs_irbuilder_append_imported_symbol( \
        xjs_irbuilder_ref irb, \
        xjs_ir_dataid local, \
        xjs_ir_dataid imported, \
        xjs_ir_dataid source);

/* Append a new data item */
void xjs_irbuilder_push_back_data_item( \
        xjs_irbuilder_ref irb, \
        xjs_ir_data_item_ref data_item);

/* Set toplevel function */
void xjs_irbuilder_set_toplevel_function( \
        xjs_irbuilder_ref irb, \
        xjs_ir_function_ref func);


/* Generate IR */
xjs_ir_ref xjs_irbuilder_generate_ir(xjs_irbuilder_ref irb);

#endif

