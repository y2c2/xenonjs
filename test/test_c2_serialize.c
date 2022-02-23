#include <ec_algorithm.h>
#include <ec_encoding.h>
#include <ec_string.h>
#include "xjs.h"
#include "test_serialize_helper.h"
#include "test_c2_serialize.h"

struct xjs_opaque_ir_serialize_ctx 
{
    ec_string *result;
    int node_index;
};
typedef struct xjs_opaque_ir_serialize_ctx xjs_ir_serialize_ctx;
typedef struct xjs_opaque_ir_serialize_ctx *xjs_ir_serialize_ctx_ref;

#define SERIALIZE_APPEND_INT(_result, _x) \
    do { char index_buf[10 + 1]; \
      snprintf(index_buf, 10, "%d", (int)(_x)); \
        ec_string_append_c_str(_result, index_buf); } while (0)

static int xjs_ir_serialize_append_unescaped_json_string( \
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

static int xjs_ir_serialize_data( \
        xjs_ir_serialize_ctx_ref ctx, \
        xjs_ir_data_ref data)
{
    ec_bool first = ec_true;
    ec_string_append_c_str(ctx->result, "[");
    {
        ect_iterator(xjs_ir_data_item_list) it;
        ect_for(xjs_ir_data_item_list, data->items, it)
        {
            if (first == ec_true) first = ec_false; else ec_string_append_c_str(ctx->result, ",");

            xjs_ir_data_item_ref item = ect_deref(xjs_ir_data_item_ref, it);
            switch (item->type)
            {
                case xjs_ir_data_item_type_string:
                    ec_string_append_c_str(ctx->result, "{\"id\":");
                    SERIALIZE_APPEND_INT(ctx->result, item->dataid);
                    ec_string_append_c_str(ctx->result, ",\"type\":\"string\",\"value\":\"");
                    xjs_ir_serialize_append_unescaped_json_string(ctx->result, item->u.as_string.value);
                    ec_string_append_c_str(ctx->result, "\"}");
                    break;

                case xjs_ir_data_item_type_symbol:
                    ec_string_append_c_str(ctx->result, "{\"id\":");
                    SERIALIZE_APPEND_INT(ctx->result, item->dataid);
                    ec_string_append_c_str(ctx->result, ",\"type\":\"symbol\",\"value\":\"");
                    ec_string_append(ctx->result, item->u.as_symbol.value);
                    ec_string_append_c_str(ctx->result, "\"}");
                    break;

                case xjs_ir_data_item_type_f32:
                    ec_string_append_c_str(ctx->result, "{\"id\":");
                    SERIALIZE_APPEND_INT(ctx->result, item->dataid);
                    ec_string_append_c_str(ctx->result, ",\"type\":\"f32\",\"value\":\"");
                    xjs_serialize_append_number(ctx->result, (double)item->u.as_f32.value);
                    ec_string_append_c_str(ctx->result, "\"}");
                    break;

                case xjs_ir_data_item_type_f64:
                    ec_string_append_c_str(ctx->result, "{\"id\":");
                    SERIALIZE_APPEND_INT(ctx->result, item->dataid);
                    ec_string_append_c_str(ctx->result, ",\"type\":\"f64\",\"value\":\"");
                    xjs_serialize_append_number(ctx->result, item->u.as_f64.value);
                    ec_string_append_c_str(ctx->result, "\"}");
                    break;
            }
        }
    }
    ec_string_append_c_str(ctx->result, "]");

    return 0;
}

static int xjs_ir_serialize_text( \
        xjs_ir_serialize_ctx_ref ctx, \
        xjs_ir_text_ref text)
{
    ec_bool first = ec_true;
    ec_string_append_c_str(ctx->result, "{\"functions\":[");
    {
        ect_iterator(xjs_ir_function_list) it_func;
        ect_for(xjs_ir_function_list, text->functions, it_func)
        {
            xjs_ir_function_ref func = ect_deref(xjs_ir_function_ref, it_func);

            if (first == ec_true) first = ec_false; else ec_string_append_c_str(ctx->result, ",");

            /* Parameters */
            ec_string_append_c_str(ctx->result, "{\"index\":");
            SERIALIZE_APPEND_INT(ctx->result, ctx->node_index++);
            ec_string_append_c_str(ctx->result, ",\"parameters\":[");
            {
                ec_bool first2 = ec_true;
                ect_iterator(xjs_ir_parameter_list) it;
                ect_for(xjs_ir_parameter_list, func->parameters, it)
                {
                    xjs_ir_parameter_ref parameter = ect_deref(xjs_ir_parameter_ref, it);
                    if (first2 == ec_true) first2 = ec_false; else ec_string_append_c_str(ctx->result, ",");
                    SERIALIZE_APPEND_INT(ctx->result, parameter->varname);
                }
            }
            ec_string_append_c_str(ctx->result, "],\"body\":[");

            /* Text Items */
            {
                ec_bool first2 = ec_true;
                ect_iterator(xjs_ir_text_item_list) it;
                ect_for(xjs_ir_text_item_list, func->text_items, it)
                {
                    xjs_ir_text_item_ref ti = ect_deref(xjs_ir_text_item_ref, it);
                    if (first2 == ec_true)
                        first2 = ec_false;
                    else
                        ec_string_append_c_str(ctx->result, ",");
                    switch (ti->type)
                    {
                        case xjs_ir_text_item_type_nop:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"nop\"}");
                            break;

                        case xjs_ir_text_item_type_halt:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"halt\"}");
                            break;

                        case xjs_ir_text_item_type_dynlib:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"dynlib\",\"exports\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_dynlib.exports);
                            ec_string_append_c_str(ctx->result, ",\"module\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_dynlib.module_name);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_push_scope:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"push_scope\"}");
                            break;

                        case xjs_ir_text_item_type_pop_scope:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"pop_scope\"}");
                            break;

                        case xjs_ir_text_item_type_label:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"label\",\"lbl\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_label.lbl);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_br:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"br\",\"dest\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_br.dest);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_br_cond:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"br_cond\",\"cond\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_br_cond.cond);
                            ec_string_append_c_str(ctx->result, ",\"dest\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_br_cond.dest);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_merge:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"merge\",\"test\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_merge.test);
                            ec_string_append_c_str(ctx->result, ",\"consequent\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_merge.consequent);
                            ec_string_append_c_str(ctx->result, ",\"alternate\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_merge.alternate);
                            ec_string_append_c_str(ctx->result, ",\"dst\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_merge.dst);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_alloca:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"alloca\",\"var\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_alloca.var);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_load_undefined:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"load_undefined\",\"var\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_load_undefined.var);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_load_null:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"load_null\",\"var\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_load_null.var);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_load_bool:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"load_bool\",\"var\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_load_bool.var);
                            ec_string_append_c_str(ctx->result, ",\"value\":");
                            if (ti->u.as_load_bool.value == xjs_false) { ec_string_append_c_str(ctx->result, "false"); }
                            else { ec_string_append_c_str(ctx->result, "true"); }
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_load_string:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"load_string\",\"var\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_load_string.var);
                            ec_string_append_c_str(ctx->result, ",\"dataid\":");
                            SERIALIZE_APPEND_INT(ctx->result, (int)(ti->u.as_load_string.dataid));
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_load_number:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"load_number\",\"var\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_load_number.var);
                            ec_string_append_c_str(ctx->result, ",\"value\":");
                            /* TODO: Serialize the value */
                            xjs_serialize_append_number(ctx->result, ti->u.as_load_number.value);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_load_object:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"load_object\",\"var\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_load_object.var);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_load_array:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"load_array\",\"var\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_load_array.var);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_array_push:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"array_push\",\"array\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_array_push.arr);
                            ec_string_append_c_str(ctx->result, ",\"element\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_array_push.elem);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_declvar:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"declvar\",\"variable\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_declvar.variable);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

#define SERIALIZE_UNARY_OP(name) \
                            do { \
                                ec_string_append_c_str(ctx->result, "{\"type\":\"" name "\",\"dst\":"); \
                                SERIALIZE_APPEND_INT(ctx->result, (int)(ti->u.as_unary_op.dst)); \
                                ec_string_append_c_str(ctx->result, ",\"lhs\":"); \
                                SERIALIZE_APPEND_INT(ctx->result, (int)(ti->u.as_unary_op.src)); \
                                ec_string_append_c_str(ctx->result, "}"); \
                            } while (0)

                        case xjs_ir_text_item_type_unary_not:
                            SERIALIZE_UNARY_OP("unary_not");
                            break;

                        case xjs_ir_text_item_type_unary_bnot:
                            SERIALIZE_UNARY_OP("unary_bnot");
                            break;

                        case xjs_ir_text_item_type_unary_add:
                            SERIALIZE_UNARY_OP("unary_add");
                            break;

                        case xjs_ir_text_item_type_unary_sub:
                            SERIALIZE_UNARY_OP("unary_sub");
                            break;

#define SERIALIZE_BINARY_OP(name) \
                            do { \
                                ec_string_append_c_str(ctx->result, "{\"type\":\"" name "\",\"dst\":"); \
                                SERIALIZE_APPEND_INT(ctx->result, (int)(ti->u.as_binary_op.dst)); \
                                ec_string_append_c_str(ctx->result, ",\"lhs\":"); \
                                SERIALIZE_APPEND_INT(ctx->result, (int)(ti->u.as_binary_op.lhs)); \
                                ec_string_append_c_str(ctx->result, ",\"rhs\":"); \
                                SERIALIZE_APPEND_INT(ctx->result, (int)(ti->u.as_binary_op.rhs)); \
                                ec_string_append_c_str(ctx->result, "}"); \
                            } while (0)

                        case xjs_ir_text_item_type_binary_add:
                            SERIALIZE_BINARY_OP("binary_add");
                            break;

                        case xjs_ir_text_item_type_binary_sub:
                            SERIALIZE_BINARY_OP("binary_sub");
                            break;

                        case xjs_ir_text_item_type_binary_mul:
                            SERIALIZE_BINARY_OP("binary_mul");
                            break;

                        case xjs_ir_text_item_type_binary_div:
                            SERIALIZE_BINARY_OP("binary_div");
                            break;

                        case xjs_ir_text_item_type_binary_mod:
                            SERIALIZE_BINARY_OP("binary_mod");
                            break;

                        case xjs_ir_text_item_type_binary_e2:
                            SERIALIZE_BINARY_OP("binary_e2");
                            break;

                        case xjs_ir_text_item_type_binary_ne2:
                            SERIALIZE_BINARY_OP("binary_ne2");
                            break;

                        case xjs_ir_text_item_type_binary_e3:
                            SERIALIZE_BINARY_OP("binary_e3");
                            break;

                        case xjs_ir_text_item_type_binary_ne3:
                            SERIALIZE_BINARY_OP("binary_ne3");
                            break;

                        case xjs_ir_text_item_type_binary_l:
                            SERIALIZE_BINARY_OP("binary_l");
                            break;

                        case xjs_ir_text_item_type_binary_le:
                            SERIALIZE_BINARY_OP("binary_le");
                            break;

                        case xjs_ir_text_item_type_binary_g:
                            SERIALIZE_BINARY_OP("binary_g");
                            break;

                        case xjs_ir_text_item_type_binary_ge:
                            SERIALIZE_BINARY_OP("binary_ge");
                            break;

                        case xjs_ir_text_item_type_binary_and:
                            SERIALIZE_BINARY_OP("binary_and");
                            break;

                        case xjs_ir_text_item_type_binary_or:
                            SERIALIZE_BINARY_OP("binary_or");
                            break;

                        case xjs_ir_text_item_type_load:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"load\",\"var\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_load.var);
                            ec_string_append_c_str(ctx->result, ",\"variable\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_load.variable);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_store:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"store\",\"variable\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_store.variable);
                            ec_string_append_c_str(ctx->result, ",\"var\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_store.var);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_object_set:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"object_set\",\"obj\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_object_set.obj);
                            ec_string_append_c_str(ctx->result, ",\"name\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_object_set.src);
                            ec_string_append_c_str(ctx->result, ",\"value\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_object_set.member);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_object_get:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"object_get\",\"obj\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_object_get.dst);
                            ec_string_append_c_str(ctx->result, ",\"value\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_object_get.obj);
                            ec_string_append_c_str(ctx->result, ",\"name\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_object_get.member);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_inspect:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"inspect\",\"var\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_inspect.var);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_ret:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"ret\",\"var\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_ret.var);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_call:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"call\",\"dst\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_call.dst);
                            ec_string_append_c_str(ctx->result, ",\"callee\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_call.callee);
                            ec_string_append_c_str(ctx->result, ",\"arguments\":[");
                            {
                                ec_bool first3 = ec_true;
                                ect_iterator(xjs_ir_var_list) it_arg;
                                ect_for(xjs_ir_var_list, ti->u.as_call.arguments, it_arg)
                                {
                                    xjs_ir_var var_arg = ect_deref(xjs_ir_var, it_arg);
                                    if (first3 == ec_true) first3 = ec_false; else ec_string_append_c_str(ctx->result, ",");
                                    SERIALIZE_APPEND_INT(ctx->result, var_arg);
                                }
                            }
                            ec_string_append_c_str(ctx->result, "]}");
                            break;

                        case xjs_ir_text_item_type_new:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"new\",\"dst\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_new.dst);
                            ec_string_append_c_str(ctx->result, ",\"callee\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_new.callee);
                            ec_string_append_c_str(ctx->result, ",\"arguments\":[");
                            {
                                ec_bool first3 = ec_true;
                                ect_iterator(xjs_ir_var_list) it_arg;
                                ect_for(xjs_ir_var_list, ti->u.as_new.arguments, it_arg)
                                {
                                    xjs_ir_var var_arg = ect_deref(xjs_ir_var, it_arg);
                                    if (first3 == ec_true) first3 = ec_false; else ec_string_append_c_str(ctx->result, ",");
                                    SERIALIZE_APPEND_INT(ctx->result, var_arg);
                                }
                            }
                            ec_string_append_c_str(ctx->result, "]}");
                            break;

                        case xjs_ir_text_item_type_this:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"this\",\"dst\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_this.dst);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_make_function:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"make_function\",\"var\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_make_function.var);
                            ec_string_append_c_str(ctx->result, "}");
                            break;

                        case xjs_ir_text_item_type_make_arrow_function:
                            ec_string_append_c_str(ctx->result, "{\"type\":\"make_arrow_function\",\"var\":");
                            SERIALIZE_APPEND_INT(ctx->result, ti->u.as_make_arrow_function.var);
                            ec_string_append_c_str(ctx->result, "}");
                            break;
                    }
                }
            }
            ec_string_append_c_str(ctx->result, "]}");
        }
    }
    ec_string_append_c_str(ctx->result, "]}");
    return 0;
}

char *xjs_ir_serialize(xjs_ir_ref ir)
{
    char *encoded_result = NULL;
    ec_size_t encoded_result_len;
    ec_string *result = NULL;
    xjs_ir_serialize_ctx ctx;

    if ((result = ec_string_new()) == NULL) { return NULL; }

    ctx.result = result;
    ctx.node_index = 0;

    ec_string_append_c_str(result, "{");

    /* Data */
    ec_string_append_c_str(result, "\"data\":");
    if (xjs_ir_serialize_data(&ctx, ir->data) != 0) { goto fail; }
    /* Text */
    ec_string_append_c_str(result, ",\"text\":");
    if (xjs_ir_serialize_text(&ctx, ir->text) != 0) { goto fail; }
    /* Export */
    {
        if (ect_list_size(xjs_ir_export_list, ir->exported) != 0)
        {
            ec_bool first = ec_true;
            ect_iterator(xjs_ir_export_list) it;
            ec_string_append_c_str(result, ",\"exported\":[");
            ect_for(xjs_ir_export_list, ir->exported, it)
            {
                xjs_ir_export_item_ref export_item = ect_deref(xjs_ir_export_item_ref, it);
                if (first == ec_true) first = ec_false; else ec_string_append_c_str(result, ",");
                ec_string_append_c_str(result, "{\"exported\":");
                SERIALIZE_APPEND_INT(result, export_item->exported);
                ec_string_append_c_str(result, ",\"local\":");
                SERIALIZE_APPEND_INT(result, export_item->local);
                ec_string_append_c_str(result, "}");
            }
            ec_string_append_c_str(result, "]");
        }
    }
    /* Import */
    {
        if (ect_list_size(xjs_ir_import_list, ir->imported) != 0)
        {
            ect_iterator(xjs_ir_import_list) it;
            ec_string_append_c_str(result, ",\"imported\":[");
            ect_for(xjs_ir_import_list, ir->imported, it)
            {
                xjs_ir_import_item_ref import_item = ect_deref(xjs_ir_import_item_ref, it);
                ec_string_append_c_str(result, "{\"local\":");
                SERIALIZE_APPEND_INT(result, import_item->local);
                ec_string_append_c_str(result, ",\"imported\":");
                SERIALIZE_APPEND_INT(result, import_item->imported);
                ec_string_append_c_str(result, ",\"source\":");
                SERIALIZE_APPEND_INT(result, import_item->source);
                ec_string_append_c_str(result, "}");
            }
            ec_string_append_c_str(result, "]");
        }
    }
    /* Entry point */
    if (ctx.node_index > 0)
    {
        ec_string_append_c_str(result, ",\"entry\":");
        SERIALIZE_APPEND_INT(result, 0);
    }
    ec_string_append_c_str(result, "}");

    /* Encode as UTF-8 */
    {
        ec_encoding_t enc;
        ec_encoding_utf8_init(&enc);
        ec_encoding_encode(&enc, (ec_byte_t **)&encoded_result, &encoded_result_len, result);
    }

fail:
    ec_delete(result);
    return encoded_result;
}

