/* XenonJS : IR Printer
 * Copyright(c) 2017 y2c2 */

#include <ec_algorithm.h>
#include <ec_encoding.h>
#include <ec_list.h>
#include <ec_map.h>
#include <ec_string.h>
#include "xjs.h"
#include "xjs_ir.h"
#include "xjs_helper.h"
#include "xjs_aux.h"
#include "xjs_irprinter.h"

ect_map_declare(xjs_ir_function_map, xjs_ir_function_ref, xjs_ir_functionid);

static int xjs_ir_function_map_value_ctor(xjs_ir_functionid *detta_value, xjs_ir_functionid *value)
{ *detta_value = *value; return 0; }

static int xjs_ir_function_map_key_ctor(xjs_ir_function_ref *detta_key, xjs_ir_function_ref *key)
{ *detta_key = *key; return 0; }

static void xjs_ir_function_map_key_dtor(xjs_ir_function_ref *key)
{ (void)key; }

static int xjs_ir_function_map_key_cmp(xjs_ir_function_ref *a, xjs_ir_function_ref *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

ect_map_define_declared(xjs_ir_function_map, \
        xjs_ir_function_ref, xjs_ir_function_map_key_ctor, xjs_ir_function_map_key_dtor, \
        xjs_ir_functionid, xjs_ir_function_map_value_ctor, NULL, \
        xjs_ir_function_map_key_cmp);
typedef xjs_ir_function_map *xjs_ir_function_map_ref;

static void serialize_append_int(ec_string *result, int val)
{
    if (val == 0) { ec_string_append_c_str(result, "0"); return; }
    if (val <= 0) { ec_string_append_c_str(result, "-"); val = -val; }

    {
        ec_string *tmp_s = ec_string_new();
        do
        {
            ec_string_push_front(tmp_s, (ec_char_t)("0123456789"[val % 10]));
            val /= 10;;
        } while (val != 0);
        ec_string_append(result, tmp_s);
        ec_delete(tmp_s);
    }
}

#define SERIALIZE_APPEND_INT(_result, _x) \
    do { serialize_append_int(_result, (int)(_x)); } while (0)

/*
    do { char index_buf[10 + 1]; \
      snprintf(index_buf, 10, "%d", (int)(_x)); \
        ec_string_append_c_str(_result, index_buf); } while (0)
        */

#define SERIALIZE_APPEND_LOC(_result, _loc) \
    do { \
        if (generate_debug_info == xjs_true && \
                ((_loc)->filename != NULL) && \
                ((_loc)->start.ln != -1) && ((_loc)->end.ln != -1)) { \
            ec_string_append_c_str(_result, " // "); \
            ec_string_append(_result, (_loc)->filename); \
            ec_string_append_c_str(_result, ":"); \
            SERIALIZE_APPEND_INT(_result, (_loc)->start.ln); \
            ec_string_append_c_str(_result, ":"); \
            SERIALIZE_APPEND_INT(_result, (_loc)->start.col + 1); \
            ec_string_append_c_str(_result, "-"); \
            SERIALIZE_APPEND_INT(_result, (_loc)->end.ln); \
            ec_string_append_c_str(_result, ":"); \
            SERIALIZE_APPEND_INT(_result, (_loc)->end.col + 1); \
        } \
    } while (0)


static int xjs_irprinter_serialize_append_unescaped_json_string( \
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

static int xjs_irprinter_write_data_section( \
        xjs_error_ref err, \
        ec_string *result, \
        xjs_ir_ref ir)
{
    int ret = 0;
    int written_item = 0;

    (void)err;

    ect_iterator(xjs_ir_data_item_list) it;
    ect_for(xjs_ir_data_item_list, ir->data->items, it)
    {
        xjs_ir_data_item_ref item = ect_deref(xjs_ir_data_item_ref, it);
        switch (item->type)
        {
            case xjs_ir_data_item_type_string:
                ec_string_append_c_str(result, "const ");
                ec_string_append_c_str(result, "data");
                SERIALIZE_APPEND_INT(result, item->dataid);
                ec_string_append_c_str(result, " = \"");
                xjs_irprinter_serialize_append_unescaped_json_string(result, item->u.as_string.value);
                ec_string_append_c_str(result, "\";\n");
                written_item++;
                break;

            case xjs_ir_data_item_type_symbol:
                /*
                ec_string_append_c_str(ctx->result, "{\"id\":");
                SERIALIZE_APPEND_INT(ctx->result, item->dataid);
                ec_string_append_c_str(ctx->result, ",\"type\":\"symbol\",\"value\":\"");
                ec_string_append(ctx->result, item->u.as_symbol.value);
                ec_string_append_c_str(ctx->result, "\"}");
                */
                break;

            case xjs_ir_data_item_type_f32:
                ec_string_append_c_str(result, "const ");
                ec_string_append_c_str(result, "data");
                SERIALIZE_APPEND_INT(result, item->dataid);
                ec_string_append_c_str(result, " = ");
                xjs_aux_serialize_append_number(result, (double)item->u.as_f32.value);
                ec_string_append_c_str(result, ";\n");
                written_item++;
                break;

            case xjs_ir_data_item_type_f64:
                ec_string_append_c_str(result, "const ");
                ec_string_append_c_str(result, "data");
                SERIALIZE_APPEND_INT(result, item->dataid);
                ec_string_append_c_str(result, " = ");
                xjs_aux_serialize_append_number(result, item->u.as_f64.value);
                ec_string_append_c_str(result, ";\n");
                written_item++;
                break;
        }
    }

    if (written_item != 0)
    {
        ec_string_append_c_str(result, "\n");
    }

    return ret;
}

static int xjs_irprinter_write_text_section( \
        xjs_error_ref err, \
        ec_string *result, \
        xjs_ir_ref ir, \
        const xjs_bool generate_debug_info)
{
    int ret = 0;
    xjs_ir_function_map_ref function_id_map = NULL;

    (void)err;

    if ((function_id_map = ect_map_new(xjs_ir_function_map)) == NULL)
    { goto fail; }

    /* Label functions with id */
    {
        xjs_ir_functionid function_id = 0;
        ect_iterator(xjs_ir_function_list) it;
        ect_for(xjs_ir_function_list, ir->text->functions, it)
        {
            xjs_ir_function_ref func = ect_deref(xjs_ir_function_ref, it);
            ect_map_insert(xjs_ir_function_map, function_id_map, \
                    func, function_id++);
        }
    }

    ect_iterator(xjs_ir_function_list) it_func;
    ect_for(xjs_ir_function_list, ir->text->functions, it_func)
    {
        xjs_ir_function_ref func = ect_deref(xjs_ir_function_ref, it_func);
        ec_string_append_c_str(result, "function f");
        SERIALIZE_APPEND_INT(result, \
            ect_map_get(xjs_ir_function_map, function_id_map, func));
        ec_string_append_c_str(result, "(");
        {
            ec_bool first = ec_true;
            ect_iterator(xjs_ir_parameter_list) it_parameter;
            ect_for(xjs_ir_parameter_list, func->parameters, it_parameter)
            {
                xjs_ir_parameter_ref parameter = ect_deref(xjs_ir_parameter_ref, it_parameter);
                if (first == ec_true) first = ec_false; else ec_string_append_c_str(result, ", ");

                {
                    xjs_ir_dataid dataid = parameter->varname;
                    xjs_ir_data_item_ref item = xjs_ir_get_data_item_by_dataid(ir, dataid);
                    if (item->type != xjs_ir_data_item_type_symbol)
                    { goto fail; }
                    ec_string_append(result, item->u.as_symbol.value);
                }
            }
        }
        ec_string_append_c_str(result, ") {\n");

        ect_iterator(xjs_ir_text_item_list) it;
        ect_for(xjs_ir_text_item_list, func->text_items, it)
        {
            xjs_ir_text_item_ref ti = ect_deref(xjs_ir_text_item_ref, it);
            switch (ti->type)
            {
                case xjs_ir_text_item_type_nop:
                    break;

                case xjs_ir_text_item_type_halt:
                    ec_string_append_c_str(result, "  halt();");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_dynlib:
                    ec_string_append_c_str(result, "  dynlib(t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_dynlib.exports);
                    ec_string_append_c_str(result, ", t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_dynlib.module_name);
                    ec_string_append_c_str(result, ");");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_push_scope:
                case xjs_ir_text_item_type_pop_scope:
                    break;

                case xjs_ir_text_item_type_label:
                    ec_string_append_c_str(result, "label");
                    SERIALIZE_APPEND_INT(result, ti->u.as_label.lbl);
                    ec_string_append_c_str(result, ":");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_br:
                    ec_string_append_c_str(result, "  goto label");
                    SERIALIZE_APPEND_INT(result, ti->u.as_br.dest);
                    ec_string_append_c_str(result, ";");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_br_cond:
                    ec_string_append_c_str(result, "  if (t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_br_cond.cond);
                    ec_string_append_c_str(result, ") goto label");
                    SERIALIZE_APPEND_INT(result, ti->u.as_br_cond.dest);
                    ec_string_append_c_str(result, ";");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_merge:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_merge.dst);
                    ec_string_append_c_str(result, " = t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_merge.test);
                    ec_string_append_c_str(result, " ? t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_merge.consequent);
                    ec_string_append_c_str(result, " : t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_merge.alternate);
                    ec_string_append_c_str(result, ";");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_alloca:
                    ec_string_append_c_str(result, "  var t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_alloca.var);
                    ec_string_append_c_str(result, ";");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_load_undefined:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_load_undefined.var);
                    ec_string_append_c_str(result, " = undefined;");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_load_null:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_load_null.var);
                    ec_string_append_c_str(result, " = null;");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_load_bool:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_load_bool.var);
                    if (ti->u.as_load_bool.value == xjs_false)
                    { ec_string_append_c_str(result, " = false;"); }
                    else
                    { ec_string_append_c_str(result, " = true;"); }
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_load_number:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_load_number.var);
                    { ec_string_append_c_str(result, " = "); }
                    if (xjs_aux_serialize_append_number(result, ti->u.as_load_number.value) != 0)
                    { XJS_ERROR_INTERNAL(err); }
                    ec_string_append_c_str(result, ";");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_load_string:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_load_string.var);
                    { ec_string_append_c_str(result, " = "); }
                    {
                        xjs_ir_dataid dataid = ti->u.as_load_string.dataid;
                        xjs_ir_data_item_ref item = xjs_ir_get_data_item_by_dataid(ir, dataid);
                        /* ec_string_append_c_str(result, "data"); */
                        /* SERIALIZE_APPEND_INT(result, dataid); */
                        ec_string_append_c_str(result, "\"");
                        ec_string_append(result, item->u.as_string.value);
                        ec_string_append_c_str(result, "\"");
                    }
                    ec_string_append_c_str(result, ";");
                    /*
                    {
                        xjs_ir_dataid dataid = ti->u.as_load_string.dataid;
                        xjs_ir_data_item_ref item = xjs_ir_get_data_item_by_dataid(ir, dataid);
                        if (item->type != xjs_ir_data_item_type_string)
                        { goto fail; }
                        ec_string_append_c_str(result, " // \"");
                        ec_string_append(result, item->u.as_string.value);
                        ec_string_append_c_str(result, "\"");
                    }
                    */
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_load_object:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_load_object.var);
                    ec_string_append_c_str(result, " = {};");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_load_array:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_load_array.var);
                    ec_string_append_c_str(result, " = [];");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_declvar:
                    ec_string_append_c_str(result, "  var ");
                    {
                        xjs_ir_dataid dataid = ti->u.as_declvar.variable;
                        xjs_ir_data_item_ref item = xjs_ir_get_data_item_by_dataid(ir, dataid);
                        if (item->type != xjs_ir_data_item_type_symbol)
                        { goto fail; }
                        ec_string_append(result, item->u.as_symbol.value);
                    }
                    ec_string_append_c_str(result, ";");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_unary_not:
                case xjs_ir_text_item_type_unary_bnot:
                case xjs_ir_text_item_type_unary_add:
                case xjs_ir_text_item_type_unary_sub:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_unary_op.dst);
                    if (ti->type == xjs_ir_text_item_type_unary_not)
                    { ec_string_append_c_str(result, " = !"); }
                    else if (ti->type == xjs_ir_text_item_type_unary_bnot)
                    { ec_string_append_c_str(result, " = ~"); }
                    else if (ti->type == xjs_ir_text_item_type_unary_add)
                    { ec_string_append_c_str(result, " = +"); }
                    else if (ti->type == xjs_ir_text_item_type_unary_sub)
                    { ec_string_append_c_str(result, " = -"); }
                    ec_string_append_c_str(result, "t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_unary_op.src);
                    ec_string_append_c_str(result, ";");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_binary_add:
                case xjs_ir_text_item_type_binary_sub:
                case xjs_ir_text_item_type_binary_mul:
                case xjs_ir_text_item_type_binary_div:
                case xjs_ir_text_item_type_binary_mod:
                case xjs_ir_text_item_type_binary_e2:
                case xjs_ir_text_item_type_binary_ne2:
                case xjs_ir_text_item_type_binary_e3:
                case xjs_ir_text_item_type_binary_ne3:
                case xjs_ir_text_item_type_binary_l:
                case xjs_ir_text_item_type_binary_le:
                case xjs_ir_text_item_type_binary_g:
                case xjs_ir_text_item_type_binary_ge:
                case xjs_ir_text_item_type_binary_and:
                case xjs_ir_text_item_type_binary_or:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_binary_op.dst);
                    ec_string_append_c_str(result, " = ");

                    ec_string_append_c_str(result, "t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_binary_op.lhs);
                    if (ti->type == xjs_ir_text_item_type_binary_add)
                    { ec_string_append_c_str(result, " + "); }
                    else if (ti->type == xjs_ir_text_item_type_binary_sub)
                    { ec_string_append_c_str(result, " - "); }
                    else if (ti->type == xjs_ir_text_item_type_binary_mul)
                    { ec_string_append_c_str(result, " * "); }
                    else if (ti->type == xjs_ir_text_item_type_binary_div)
                    { ec_string_append_c_str(result, " / "); }
                    else if (ti->type == xjs_ir_text_item_type_binary_mod)
                    { ec_string_append_c_str(result, " % "); }
                    else if (ti->type == xjs_ir_text_item_type_binary_e2)
                    { ec_string_append_c_str(result, " == "); }
                    else if (ti->type == xjs_ir_text_item_type_binary_ne2)
                    { ec_string_append_c_str(result, " != "); }
                    else if (ti->type == xjs_ir_text_item_type_binary_e3)
                    { ec_string_append_c_str(result, " === "); }
                    else if (ti->type == xjs_ir_text_item_type_binary_ne3)
                    { ec_string_append_c_str(result, " !== "); }
                    else if (ti->type == xjs_ir_text_item_type_binary_l)
                    { ec_string_append_c_str(result, " < "); }
                    else if (ti->type == xjs_ir_text_item_type_binary_le)
                    { ec_string_append_c_str(result, " <= "); }
                    else if (ti->type == xjs_ir_text_item_type_binary_g)
                    { ec_string_append_c_str(result, " > "); }
                    else if (ti->type == xjs_ir_text_item_type_binary_ge)
                    { ec_string_append_c_str(result, " >= "); }
                    else if (ti->type == xjs_ir_text_item_type_binary_and)
                    { ec_string_append_c_str(result, " && "); }
                    else if (ti->type == xjs_ir_text_item_type_binary_or)
                    { ec_string_append_c_str(result, " || "); }
                    ec_string_append_c_str(result, "t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_binary_op.rhs);

                    ec_string_append_c_str(result, ";");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_load:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_load.var);
                    ec_string_append_c_str(result, " = ");
                    {
                        xjs_ir_dataid dataid = ti->u.as_load.variable;
                        xjs_ir_data_item_ref item = xjs_ir_get_data_item_by_dataid(ir, dataid);
                        if (item->type != xjs_ir_data_item_type_symbol)
                        {
                            goto fail;
                        }
                        ec_string_append(result, item->u.as_symbol.value);
                    }
                    ec_string_append_c_str(result, ";");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_store:
                    ec_string_append_c_str(result, "  ");
                    {
                        xjs_ir_dataid dataid = ti->u.as_store.variable;
                        xjs_ir_data_item_ref item = xjs_ir_get_data_item_by_dataid(ir, dataid);
                        if (item->type != xjs_ir_data_item_type_symbol)
                        {
                            goto fail;
                        }
                        ec_string_append(result, item->u.as_symbol.value);
                    }
                    ec_string_append_c_str(result, " = t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_store.var);
                    ec_string_append_c_str(result, ";");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_object_set:
                    ec_string_append_c_str(result, "  ");
                    ec_string_append_c_str(result, "t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_object_set.dst);
                    ec_string_append_c_str(result, " = t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_object_set.obj);
                    ec_string_append_c_str(result, "[");
                    ec_string_append_c_str(result, "t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_object_set.member);
                    ec_string_append_c_str(result, "]");
                    ec_string_append_c_str(result, " = t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_object_set.src);
                    ec_string_append_c_str(result, ";");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_object_get:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_object_get.dst);
                    ec_string_append_c_str(result, " = ");
                    ec_string_append_c_str(result, "t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_object_get.obj);
                    ec_string_append_c_str(result, "[");
                    ec_string_append_c_str(result, "t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_object_get.member);
                    ec_string_append_c_str(result, "];");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_array_push:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_array_push.arr);
                    ec_string_append_c_str(result, ".push(t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_array_push.elem);
                    ec_string_append_c_str(result, ");");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_make_function:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_make_function.var);
                    ec_string_append_c_str(result, " = ");
                    ec_string_append_c_str(result, "f");
                    SERIALIZE_APPEND_INT(result, ti->u.as_make_function.func);
                    ec_string_append_c_str(result, ";");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_make_arrow_function:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_make_arrow_function.var);
                    ec_string_append_c_str(result, " = ");
                    ec_string_append_c_str(result, "f");
                    SERIALIZE_APPEND_INT(result, ti->u.as_make_arrow_function.func);
                    ec_string_append_c_str(result, ";");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_inspect:
                    ec_string_append_c_str(result, "  inspect t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_inspect.var);
                    ec_string_append_c_str(result, ";");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_ret:
                    ec_string_append_c_str(result, "  return t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_ret.var);
                    ec_string_append_c_str(result, ";");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_call:
                    if (ti->u.as_call.bound_this.enabled == xjs_false)
                    {
                        ec_string_append_c_str(result, "  t");
                        SERIALIZE_APPEND_INT(result, ti->u.as_call.dst);
                        ec_string_append_c_str(result, " = ");
                        ec_string_append_c_str(result, "t");
                        SERIALIZE_APPEND_INT(result, ti->u.as_call.callee);
                        ec_string_append_c_str(result, "(");
                        {
                            ec_bool first = ec_true;
                            ect_iterator(xjs_ir_var_list) it_arg;
                            ect_for(xjs_ir_var_list, ti->u.as_call.arguments, it_arg)
                            {
                                xjs_ir_var var_arg = ect_deref(xjs_ir_var, it_arg);
                                if (first == ec_true) first = ec_false; else ec_string_append_c_str(result, ", ");
                                ec_string_append_c_str(result, "t");
                                SERIALIZE_APPEND_INT(result, var_arg);
                            }
                        }
                        ec_string_append_c_str(result, ");");
                        SERIALIZE_APPEND_LOC(result, &ti->loc);
                        ec_string_append_c_str(result, "\n");
                    }
                    else
                    {
                        ec_string_append_c_str(result, "  t");
                        SERIALIZE_APPEND_INT(result, ti->u.as_call.dst);
                        ec_string_append_c_str(result, " = ");
                        ec_string_append_c_str(result, "t");
                        SERIALIZE_APPEND_INT(result, ti->u.as_call.callee);
                        ec_string_append_c_str(result, ".call(t");
                        SERIALIZE_APPEND_INT(result, ti->u.as_call.bound_this._this);
                        {
                            ect_iterator(xjs_ir_var_list) it_arg;
                            ect_for(xjs_ir_var_list, ti->u.as_call.arguments, it_arg)
                            {
                                xjs_ir_var var_arg = ect_deref(xjs_ir_var, it_arg);
                                ec_string_append_c_str(result, ", ");
                                ec_string_append_c_str(result, "t");
                                SERIALIZE_APPEND_INT(result, var_arg);
                            }
                        }
                        ec_string_append_c_str(result, ");");
                        SERIALIZE_APPEND_LOC(result, &ti->loc);
                        ec_string_append_c_str(result, "\n");
                    }
                    break;

                case xjs_ir_text_item_type_new:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_new.dst);
                    ec_string_append_c_str(result, " = new ");
                    ec_string_append_c_str(result, "t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_new.callee);
                    ec_string_append_c_str(result, "(");
                    {
                        ec_bool first = ec_true;
                        ect_iterator(xjs_ir_var_list) it_arg;
                        ect_for(xjs_ir_var_list, ti->u.as_new.arguments, it_arg)
                        {
                            xjs_ir_var var_arg = ect_deref(xjs_ir_var, it_arg);
                            if (first == ec_true) first = ec_false; else ec_string_append_c_str(result, ", ");
                            ec_string_append_c_str(result, "t");
                            SERIALIZE_APPEND_INT(result, var_arg);
                        }
                    }
                    ec_string_append_c_str(result, ");");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;

                case xjs_ir_text_item_type_this:
                    ec_string_append_c_str(result, "  t");
                    SERIALIZE_APPEND_INT(result, ti->u.as_this.dst);
                    ec_string_append_c_str(result, " = this;");
                    SERIALIZE_APPEND_LOC(result, &ti->loc);
                    ec_string_append_c_str(result, "\n");
                    break;
            }
        }

        ec_string_append_c_str(result, "}\n\n");
    }

fail:
    ec_delete(function_id_map);
    return ret;
}

static int xjs_irprinter_write_export_section( \
        xjs_error_ref err, \
        ec_string *result, \
        xjs_ir_ref ir)
{
    int ret = 0;
    ec_bool first = ec_true;

    (void)err;

    if (ect_list_size(xjs_ir_export_list, ir->exported) != 0)
    {
        ect_iterator(xjs_ir_export_list) it_export_symbol;

        ec_string_append_c_str(result, "export { ");

        ect_for(xjs_ir_export_list, ir->exported, it_export_symbol)
        {
            xjs_ir_export_item_ref export_symbol = ect_deref(xjs_ir_export_item_ref, it_export_symbol);

            if (first == ec_true) first = ec_false; else ec_string_append_c_str(result, ", ");

            {
                xjs_ir_dataid dataid = export_symbol->local;
                xjs_ir_data_item_ref item = xjs_ir_get_data_item_by_dataid(ir, dataid);
                if (item->type != xjs_ir_data_item_type_symbol)
                { ret = -1; goto fail; }
                ec_string_append(result, item->u.as_symbol.value);
            }

            if (export_symbol->local != export_symbol->exported)
            {
                ec_string_append_c_str(result, " as ");
                {
                    xjs_ir_dataid dataid = export_symbol->exported;
                    xjs_ir_data_item_ref item = xjs_ir_get_data_item_by_dataid(ir, dataid);
                    if (item->type != xjs_ir_data_item_type_symbol)
                    { ret = -1; goto fail; }
                    ec_string_append(result, item->u.as_symbol.value);
                }
            }
        }

        ec_string_append_c_str(result, " };\n\n");
    }

fail:
    return ret;
}

static int xjs_irprinter_write_import_section( \
        xjs_error_ref err, \
        ec_string *result, \
        xjs_ir_ref ir)
{
    int ret = 0;

    (void)err;

    if (ect_list_size(xjs_ir_import_list, ir->imported) != 0)
    {
        ect_iterator(xjs_ir_import_list) it_import_symbol;

        ect_for(xjs_ir_import_list, ir->imported, it_import_symbol)
        {
            xjs_ir_import_item_ref import_symbol = ect_deref(xjs_ir_import_item_ref, it_import_symbol);

            ec_string_append_c_str(result, "import { ");

            {
                xjs_ir_dataid dataid = import_symbol->local;
                xjs_ir_data_item_ref item = xjs_ir_get_data_item_by_dataid(ir, dataid);
                if (item->type != xjs_ir_data_item_type_symbol)
                { ret = -1; goto fail; }
                ec_string_append(result, item->u.as_symbol.value);
            }

            if (import_symbol->local != import_symbol->imported)
            {
                ec_string_append_c_str(result, " as ");
                {
                    xjs_ir_dataid dataid = import_symbol->imported;
                    xjs_ir_data_item_ref item = xjs_ir_get_data_item_by_dataid(ir, dataid);
                    if (item->type != xjs_ir_data_item_type_symbol)
                    { ret = -1; goto fail; }
                    ec_string_append(result, item->u.as_symbol.value);
                }
            }

            ec_string_append_c_str(result, " } from ");
            ec_string_append_c_str(result, "data");
            SERIALIZE_APPEND_INT(result, import_symbol->source);
            ec_string_append_c_str(result, ";\n");
        }
        ec_string_append_c_str(result, "\n");
    }
    
fail:
    return ret;
}

/* IR Printer */
int xjs_irprinter_start_ex( \
        xjs_error_ref err, \
        ec_string **result_out, \
        xjs_ir_ref ir, \
        const xjs_bool generate_debug_info)
{
    int ret = 0;
    ec_string *result = NULL;

    XJS_VNZ_ERROR_MEM(result = ec_string_new(), err);

    if (xjs_irprinter_write_data_section( \
                err, result, ir) != 0)
    { goto fail; }

    if (xjs_irprinter_write_text_section( \
                err, result, ir, generate_debug_info) != 0)
    { goto fail; }

    if (xjs_irprinter_write_export_section( \
                err, result, ir) != 0)
    { goto fail; }

    if (xjs_irprinter_write_import_section( \
                err, result, ir) != 0)
    { goto fail; }

    *result_out = result;

    goto done;
fail:
    ret = -1;
    ec_delete(result); result = NULL;
done:
    return ret;
}

int xjs_irprinter_start( \
        xjs_error_ref err, \
        ec_string **result_out, \
        xjs_ir_ref ir)
{
    return xjs_irprinter_start_ex( \
        err, \
        result_out, \
        ir, xjs_false);
}

