/* XenonJS : IR
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_IR_H
#define XJS_IR_H

#include <ec_string.h>
#include "xjs_types.h"

xjs_ir_var_list_ref xjs_ir_var_list_clone(xjs_ir_var_list_ref r);

/* Data Items */
xjs_ir_data_item_ref xjs_ir_data_item_string_new(xjs_ir_dataid dataid, ec_string *value);
xjs_ir_data_item_ref xjs_ir_data_item_symbol_new(xjs_ir_dataid dataid, ec_string *value);
xjs_ir_data_item_ref xjs_ir_data_item_f32_new(xjs_ir_dataid dataid, xjs_f32 value);
xjs_ir_data_item_ref xjs_ir_data_item_f64_new(xjs_ir_dataid dataid, xjs_f64 value);
xjs_ir_data_item_ref xjs_ir_data_item_clone(xjs_ir_data_item_ref r);

/* Loc */
void xjs_ir_text_item_set_loc_filename(xjs_ir_text_item_ref ti, ec_string *filename);
void xjs_ir_text_item_set_loc_start(xjs_ir_text_item_ref ti, const int start_ln, const int start_col);
void xjs_ir_text_item_set_loc_end(xjs_ir_text_item_ref ti, const int end_ln, const int end_col);

/* Range */
void xjs_ir_text_item_set_range_start(xjs_ir_text_item_ref ti, const int start);
void xjs_ir_text_item_set_range_end(xjs_ir_text_item_ref ti, const int end);

/* Text Items */
xjs_ir_text_item_ref xjs_ir_text_item_nop_new(void);
xjs_ir_text_item_ref xjs_ir_text_item_halt_new(void);
xjs_ir_text_item_ref xjs_ir_text_item_dynlib_new( \
        xjs_ir_var exports, \
        xjs_ir_var module_name);

xjs_ir_text_item_ref xjs_ir_text_item_label_new(xjs_ir_label lbl);
xjs_ir_text_item_ref xjs_ir_text_item_br_new(xjs_ir_label dest);
xjs_ir_text_item_ref xjs_ir_text_item_br_cond_new(xjs_ir_var cond, xjs_ir_label dest);
xjs_ir_text_item_ref xjs_ir_text_item_merge_new( \
        xjs_ir_var test, \
        xjs_ir_var consequent, \
        xjs_ir_var alternate, \
        xjs_ir_var dst);

xjs_ir_text_item_ref xjs_ir_text_item_alloca_new(xjs_ir_var var);
xjs_ir_text_item_ref xjs_ir_text_item_load_undefined_new(xjs_ir_var var);
xjs_ir_text_item_ref xjs_ir_text_item_load_null_new(xjs_ir_var var);
xjs_ir_text_item_ref xjs_ir_text_item_load_bool_new(xjs_ir_var var, xjs_bool value);
xjs_ir_text_item_ref xjs_ir_text_item_load_string_new(xjs_ir_var var, xjs_ir_dataid dataid);
xjs_ir_text_item_ref xjs_ir_text_item_load_number_new(xjs_ir_var var, double value);
xjs_ir_text_item_ref xjs_ir_text_item_load_object_new(xjs_ir_var var);
xjs_ir_text_item_ref xjs_ir_text_item_load_array_new(xjs_ir_var var);

xjs_ir_text_item_ref xjs_ir_text_item_declvar_new(xjs_ir_dataid variable);
xjs_ir_text_item_ref xjs_ir_text_item_load_new(xjs_ir_var var, xjs_ir_dataid variable);
xjs_ir_text_item_ref xjs_ir_text_item_store_new(xjs_ir_dataid variable, xjs_ir_var var);
xjs_ir_text_item_ref xjs_ir_text_item_object_set_new(xjs_ir_var dst, xjs_ir_var object, xjs_ir_var member, xjs_ir_var src);
xjs_ir_text_item_ref xjs_ir_text_item_object_get_new(xjs_ir_var dst, xjs_ir_var object, xjs_ir_var name);
xjs_ir_text_item_ref xjs_ir_text_item_array_push_new(xjs_ir_var arr, xjs_ir_var elem);
xjs_ir_text_item_ref xjs_ir_text_item_make_function_new(xjs_ir_var var, xjs_ir_functionid func);
xjs_ir_text_item_ref xjs_ir_text_item_make_arrow_function_new(xjs_ir_var var, xjs_ir_functionid func);
xjs_ir_text_item_ref xjs_ir_text_item_inspect_new(xjs_ir_var var);
xjs_ir_text_item_ref xjs_ir_text_item_ret_new(xjs_ir_var var);
xjs_ir_text_item_ref xjs_ir_text_item_call_new(xjs_ir_var dst, xjs_ir_var callee, xjs_ir_var_list_ref arguments);
xjs_ir_text_item_ref xjs_ir_text_item_call_bound_this_new(xjs_ir_var dst, xjs_ir_var callee, xjs_ir_var_list_ref arguments, xjs_ir_var _this);
xjs_ir_text_item_ref xjs_ir_text_item_new_new(xjs_ir_var dst, xjs_ir_var callee, xjs_ir_var_list_ref arguments);
xjs_ir_text_item_ref xjs_ir_text_item_this_new(xjs_ir_var dst);

xjs_ir_text_item_ref xjs_ir_text_item_binary_add_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs);
xjs_ir_text_item_ref xjs_ir_text_item_binary_sub_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs);
xjs_ir_text_item_ref xjs_ir_text_item_binary_mul_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs);
xjs_ir_text_item_ref xjs_ir_text_item_binary_div_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs);
xjs_ir_text_item_ref xjs_ir_text_item_binary_mod_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs);
xjs_ir_text_item_ref xjs_ir_text_item_binary_e2_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs);
xjs_ir_text_item_ref xjs_ir_text_item_binary_ne2_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs);
xjs_ir_text_item_ref xjs_ir_text_item_binary_e3_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs);
xjs_ir_text_item_ref xjs_ir_text_item_binary_ne3_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs);
xjs_ir_text_item_ref xjs_ir_text_item_binary_l_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs);
xjs_ir_text_item_ref xjs_ir_text_item_binary_le_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs);
xjs_ir_text_item_ref xjs_ir_text_item_binary_g_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs);
xjs_ir_text_item_ref xjs_ir_text_item_binary_ge_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs);
xjs_ir_text_item_ref xjs_ir_text_item_binary_and_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs);
xjs_ir_text_item_ref xjs_ir_text_item_binary_or_new(xjs_ir_var dst, xjs_ir_var lhs, xjs_ir_var rhs);

xjs_ir_text_item_ref xjs_ir_text_item_unary_not_new(xjs_ir_var dst, xjs_ir_var src);
xjs_ir_text_item_ref xjs_ir_text_item_unary_bnot_new(xjs_ir_var dst, xjs_ir_var src);
xjs_ir_text_item_ref xjs_ir_text_item_unary_add_new(xjs_ir_var dst, xjs_ir_var src);
xjs_ir_text_item_ref xjs_ir_text_item_unary_sub_new(xjs_ir_var dst, xjs_ir_var src);

xjs_ir_text_item_ref xjs_ir_text_item_clone(xjs_ir_text_item_ref ti);

/* Exported Items */
xjs_ir_export_item_ref xjs_ir_export_item_new( \
        xjs_ir_dataid exported, \
        xjs_ir_dataid local);

/* Imported Items */
xjs_ir_import_item_ref xjs_ir_import_item_new( \
        xjs_ir_dataid local, \
        xjs_ir_dataid imported, \
        xjs_ir_dataid source);

/* Parameter */
xjs_ir_parameter_ref xjs_ir_parameter_new(xjs_ir_var varname);

/* Function */
xjs_ir_function_ref xjs_ir_function_new(void);
void xjs_ir_function_append_parameter(xjs_ir_function_ref func, \
        xjs_ir_parameter_ref parameter);
void xjs_ir_function_append_text_item(xjs_ir_function_ref func, \
        xjs_ir_text_item_ref text_item);
xjs_ir_function_ref xjs_ir_function_clone(xjs_ir_function_ref fn);

/* IR */
xjs_ir_ref xjs_ir_new(void);
void xjs_ir_append_data_item(xjs_ir_ref ir, \
        xjs_ir_data_item_ref data_item);
xjs_ir_data_item_ref xjs_ir_get_data_item_by_dataid(xjs_ir_ref ir, \
        xjs_ir_dataid dataid);
xjs_ir_data_item_ref xjs_ir_get_data_item_by_string(xjs_ir_ref ir, \
        ec_string *s);

/* IR Manipulate */
void xjs_ir_module_name_set( \
        xjs_ir_ref ir, const char *name, const xjs_size_t name_len);
void xjs_ir_module_fullpath_set( \
        xjs_ir_ref ir, const char *fullpath, const xjs_size_t fullpath_len);


#endif

