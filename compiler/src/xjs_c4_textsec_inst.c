/* XenonJS : C4 : Text Section : Instruction Translate
 * Copyright(c) 2017 y2c2 */

#include <ec_algorithm.h>
#include <ec_list.h>
#include "xjs_types.h"
#include "xjs_helper.h"
#include "xjs_xformat.h"
#include "xjs_c4_ctx.h"
#include "xjs_c4_textsec_inst.h"


/* Helper Macros */

#define XJS_C4_TSA_DEF_HDLR(_name, _func_ctx, _text_item) \
static int _name(xjs_c4_func_ctx *_func_ctx, xjs_ir_text_item_ref _text_item)

#define XJS_C4_TSA_UNUSED(_x) (void)(_x)

#define XJS_C4_TSA_BEGIN() \
    int ret = 0; (void)func_ctx; (void)text_item

#define XJS_C4_TSA_END() \
    goto done; fail: ret = -1; done: return ret

#define XJS_C4_TSA_FAIL() \
    goto fail;

#define XJS_C4_TSA_DECL_ITEM(_item) \
    annotated_text_section_item_ref _item

#define XJS_C4_TSA_NEW_ITEM(_item) \
    do { \
        XJS_VNZ_ERROR_MEM(_item = annotated_text_section_item_new(), func_ctx->global_ctx->err); \
    } while (0)

#define XJS_C4_TSA_PUSH_ITEM(_item) \
    do { \
        annotated_text_section_item_list_push_back(func_ctx->annotated_function->items, _item); \
    } while (0)

#define XJS_C4_TSA_LBL(_lbl) \
    (ect_map_get(xjs_ir_label_offset_map, \
                 func_ctx->annotated_function->map_ir_label_to_offset, \
                 _lbl))

#define XJS_C4_TSA_VREG(_var) \
    ((xjs_ir_reg)xjs_regsched_alloc(func_ctx->regsched, \
        func_ctx->point, _var))

#define XJS_C4_TSA_DATAID(_dataid) \
    (ect_map_get( \
            xjs_ir_dataid_offset_map, \
            func_ctx->global_ctx->map_ir_dataid_to_offset, \
            _dataid))

#define XJS_C4_TSA_FUNCID(_funcid) \
    (ect_map_get( \
            xjs_ir_functionid_offset_map, \
            func_ctx->global_ctx->map_ir_functionid_to_offset, \
            _funcid))

#define XJS_C4_LOC_CLONE(_dst, _src) \
    do { \
        (_dst)->loc.start.ln = (_src)->loc.start.ln; \
        (_dst)->loc.start.col = (_src)->loc.start.col; \
        (_dst)->loc.end.ln = (_src)->loc.end.ln; \
        (_dst)->loc.end.col = (_src)->loc.end.col; \
    } while (0)

#define XJS_C4_RANGE_CLONE(_dst, _src) \
    do { \
        (_dst)->range.start = (_src)->range.start; \
        (_dst)->range.end = (_src)->range.end; \
    } while (0)

#define XJS_C4_LOC_RANGE_CLONE(_dst, _src) \
    do { \
        if ((_src)->loc.filename != NULL) { \
            (_dst)->filename = ec_string_clone((_src)->loc.filename); \
        } \
        XJS_C4_LOC_CLONE(_dst, _src); \
        XJS_C4_RANGE_CLONE(_dst, _src); \
    } while (0)

/* Handlers */

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_unsupported, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_FAIL();
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_do_nothing, func_ctx, text_item)
{
    XJS_C4_TSA_UNUSED(func_ctx);
    XJS_C4_TSA_UNUSED(text_item);
    return 0;
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_nop, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_NOP;
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_halt, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_HALT;
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_dynlib, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_DYNLIB;
        inst->operand.as_dynlib.exports = XJS_C4_TSA_VREG(text_item->u.as_dynlib.exports);
        inst->operand.as_dynlib.module_name = XJS_C4_TSA_VREG(text_item->u.as_dynlib.module_name);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_br, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_BR;
        inst->operand.as_br.dest = XJS_C4_TSA_LBL(text_item->u.as_br.dest);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_br_cond, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_BRC;
        inst->operand.as_br_cond.dest = XJS_C4_TSA_LBL(text_item->u.as_br_cond.dest);
        inst->operand.as_br_cond.cond = XJS_C4_TSA_VREG(text_item->u.as_br_cond.cond);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_merge, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_MERGE;
        inst->operand.as_merge.dst = XJS_C4_TSA_VREG(text_item->u.as_merge.dst);
        inst->operand.as_merge.test = XJS_C4_TSA_VREG(text_item->u.as_merge.test);
        inst->operand.as_merge.consequent = XJS_C4_TSA_VREG(text_item->u.as_merge.consequent);
        inst->operand.as_merge.alternate = XJS_C4_TSA_VREG(text_item->u.as_merge.alternate);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_inspect, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_INSPECT;
        inst->operand.as_inspect.reg = XJS_C4_TSA_VREG(text_item->u.as_inspect.var);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_ret, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_RET;
        inst->operand.as_ret.reg = XJS_C4_TSA_VREG(text_item->u.as_ret.var);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_make_function, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_LOAD_FUNCTION;
        inst->operand.as_load_function.offset_text = (xjs_ir_offset)XJS_C4_TSA_FUNCID(text_item->u.as_make_function.func);
        inst->operand.as_load_function.reg = XJS_C4_TSA_VREG(text_item->u.as_make_function.var);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_call, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        ec_size_t i = 0;
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        if (text_item->u.as_call.bound_this.enabled == xjs_false)
        {
            inst->opcode = XJS_BC_OP_CALL;
            inst->operand.as_call.dst = XJS_C4_TSA_VREG(text_item->u.as_call.dst);
            inst->operand.as_call.callee = XJS_C4_TSA_VREG(text_item->u.as_call.callee);
            inst->operand.as_call.argc = ect_list_size(xjs_ir_var_list, text_item->u.as_call.arguments);
            if (inst->operand.as_call.argc > XJS_BC_ARGC_MAX)
            { XJS_ERROR_INTERNAL(func_ctx->global_ctx->err); goto fail; }

            ect_iterator(xjs_ir_var_list) it_arg;
            ect_for(xjs_ir_var_list, text_item->u.as_call.arguments, it_arg)
            {
                xjs_ir_var arg = ect_deref(xjs_ir_var, it_arg);
                inst->operand.as_call.argv[i] = XJS_C4_TSA_VREG(arg);
                i++;
            }
        }
        else
        {
            inst->opcode = XJS_BC_OP_CALLT;
            inst->operand.as_callt.dst = XJS_C4_TSA_VREG(text_item->u.as_call.dst);
            inst->operand.as_callt.callee = XJS_C4_TSA_VREG(text_item->u.as_call.callee);
            inst->operand.as_callt._this = XJS_C4_TSA_VREG(text_item->u.as_call.bound_this._this);
            inst->operand.as_callt.argc = ect_list_size(xjs_ir_var_list, text_item->u.as_call.arguments);
            if (inst->operand.as_callt.argc > XJS_BC_ARGC_MAX)
            { XJS_ERROR_INTERNAL(func_ctx->global_ctx->err); goto fail; }

            ect_iterator(xjs_ir_var_list) it_arg;
            ect_for(xjs_ir_var_list, text_item->u.as_call.arguments, it_arg)
            {
                xjs_ir_var arg = ect_deref(xjs_ir_var, it_arg);
                inst->operand.as_callt.argv[i] = XJS_C4_TSA_VREG(arg);
                i++;
            }
        }
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_new, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_NEW;
        inst->operand.as_new.dst = XJS_C4_TSA_VREG(text_item->u.as_new.dst);
        inst->operand.as_new.callee = XJS_C4_TSA_VREG(text_item->u.as_new.callee);
        inst->operand.as_new.argc = ect_list_size(xjs_ir_var_list, text_item->u.as_new.arguments);
        if (inst->operand.as_new.argc > XJS_BC_ARGC_MAX)
        { XJS_ERROR_INTERNAL(func_ctx->global_ctx->err); goto fail; }
        {
            ec_size_t i = 0;
            ect_iterator(xjs_ir_var_list) it_arg;
            ect_for(xjs_ir_var_list, text_item->u.as_new.arguments, it_arg)
            {
                xjs_ir_var arg = ect_deref(xjs_ir_var, it_arg);
                inst->operand.as_new.argv[i] = XJS_C4_TSA_VREG(arg);
                i++;
            }
        }
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_this, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_THIS;
        inst->operand.as_this.dst = XJS_C4_TSA_VREG(text_item->u.as_this.dst);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_declvar, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_DECLVAR;
        inst->operand.as_declvar.offset_data = XJS_C4_TSA_DATAID(text_item->u.as_declvar.variable);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_store, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_STORE;
        inst->operand.as_store.offset_data = XJS_C4_TSA_DATAID(text_item->u.as_store.variable);
        inst->operand.as_store.reg = XJS_C4_TSA_VREG(text_item->u.as_store.var);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_load, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_LOAD;
        inst->operand.as_load.offset_data = XJS_C4_TSA_DATAID(text_item->u.as_load.variable);
        inst->operand.as_load.reg = XJS_C4_TSA_VREG(text_item->u.as_load.var);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_load_undefined, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_LOAD_UNDEFINED;
        inst->operand.as_load_literal.reg = XJS_C4_TSA_VREG(text_item->u.as_load_undefined.var);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_load_null, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_LOAD_NULL;
        inst->operand.as_load_literal.reg = XJS_C4_TSA_VREG(text_item->u.as_load_null.var);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_load_bool, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->operand.as_load_literal.reg = XJS_C4_TSA_VREG(text_item->u.as_load_bool.var);
        if (text_item->u.as_load_bool.value == xjs_false)
        { inst->opcode = XJS_BC_OP_LOAD_FALSE; }
        else
        { inst->opcode = XJS_BC_OP_LOAD_TRUE; }
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_load_number, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        double value = text_item->u.as_load_number.value;

        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);

        inst->operand.as_load_literal.reg = XJS_C4_TSA_VREG(text_item->u.as_load_number.var);

#define DOUBLE_EPSILON (0.000000000000001)
#define ABS(_x) (((_x) >= 0) ? (_x) : (-(_x)))
#define IS_INTEGER(_x) (ABS(((_x) - (int)(_x))) < DOUBLE_EPSILON)
#define IN_RANGE(_x, _lo, _hi) (((_lo) <= (_x)) && ((_x) < (_hi)))

        if (IS_INTEGER(value))
        {
            /* Integer */
            int value_int = (int)(value);

            if (value_int < 0)
            {
                /* Negative */
                if (IN_RANGE(value_int, -128, 0))
                {
                    inst->opcode = XJS_BC_OP_LOAD_NUMBER_S8;
                    inst->operand.as_load_literal.u.as_s8 = (ec_s8)value_int;
                }
                else if (IN_RANGE(value_int, -32768, -128))
                {
                    inst->opcode = XJS_BC_OP_LOAD_NUMBER_S16;
                    inst->operand.as_load_literal.u.as_s16 = (ec_s16)value_int;
                }
                else /* if (IN_RANGE(value_int, -2147483648, -32768)) */
                {
                    inst->opcode = XJS_BC_OP_LOAD_NUMBER_S32;
                    inst->operand.as_load_literal.u.as_s32 = (ec_s32)value_int;
                }
                /*
                else
                {
                    XJS_ERROR(func_ctx->global_ctx->err, XJS_ERRNO_INTERNAL);
                    goto fail;
                }
                */
            }
            else
            {
                /* Positive */
                if (IN_RANGE(value_int, 0, 128))
                {
                    inst->opcode = XJS_BC_OP_LOAD_NUMBER_S8;
                    inst->operand.as_load_literal.u.as_s8 = (ec_s8)value_int;
                }
                else if (IN_RANGE(value_int, 128, 32768))
                {
                    inst->opcode = XJS_BC_OP_LOAD_NUMBER_S16;
                    inst->operand.as_load_literal.u.as_s16 = (ec_s16)value_int;
                }
                else /* if (IN_RANGE(value_int, 32768, 2147483648)) */
                {
                    inst->opcode = XJS_BC_OP_LOAD_NUMBER_S32;
                    inst->operand.as_load_literal.u.as_s32 = (ec_s32)value_int;
                }
            }
        }
        else
        {
            /* Float-point */
            inst->opcode = XJS_BC_OP_LOAD_NUMBER_F64;
            inst->operand.as_load_literal.u.as_f64 = value;
        }
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_load_string, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_LOAD_STRING;
        inst->operand.as_load_string.reg = XJS_C4_TSA_VREG(text_item->u.as_load_string.var);
        inst->operand.as_load_string.offset_data = XJS_C4_TSA_DATAID(text_item->u.as_load_string.dataid);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_load_object, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_LOAD_OBJECT;
        inst->operand.as_load_object.reg = XJS_C4_TSA_VREG(text_item->u.as_load_object.var);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_load_array, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_LOAD_ARRAY;
        inst->operand.as_load_array.reg = XJS_C4_TSA_VREG(text_item->u.as_load_array.var);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_object_get, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_OBJGET;
        inst->operand.as_objget.dst = XJS_C4_TSA_VREG(text_item->u.as_object_get.dst);
        inst->operand.as_objget.obj = XJS_C4_TSA_VREG(text_item->u.as_object_get.obj);
        inst->operand.as_objget.member = XJS_C4_TSA_VREG(text_item->u.as_object_get.member);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_object_set, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_OBJSET;
        inst->operand.as_objset.dst = XJS_C4_TSA_VREG(text_item->u.as_object_set.dst);
        inst->operand.as_objset.obj = XJS_C4_TSA_VREG(text_item->u.as_object_set.obj);
        inst->operand.as_objset.member = XJS_C4_TSA_VREG(text_item->u.as_object_set.member);
        inst->operand.as_objset.src = XJS_C4_TSA_VREG(text_item->u.as_object_set.src);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_array_push, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        inst->opcode = XJS_BC_OP_ARRPUSH;
        inst->operand.as_arrpush.arr = XJS_C4_TSA_VREG(text_item->u.as_array_push.arr);
        inst->operand.as_arrpush.elem = XJS_C4_TSA_VREG(text_item->u.as_array_push.elem);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }

    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_binary_op, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        if (text_item->type == xjs_ir_text_item_type_binary_add)
        { inst->opcode = XJS_BC_OP_BINADD; }
        else if (text_item->type == xjs_ir_text_item_type_binary_sub)
        { inst->opcode = XJS_BC_OP_BINSUB; }
        else if (text_item->type == xjs_ir_text_item_type_binary_mul)
        { inst->opcode = XJS_BC_OP_BINMUL; }
        else if (text_item->type == xjs_ir_text_item_type_binary_div)
        { inst->opcode = XJS_BC_OP_BINDIV; }
        else if (text_item->type == xjs_ir_text_item_type_binary_mod)
        { inst->opcode = XJS_BC_OP_BINMOD; }
        else if (text_item->type == xjs_ir_text_item_type_binary_e2)
        { inst->opcode = XJS_BC_OP_BINE2; }
        else if (text_item->type == xjs_ir_text_item_type_binary_ne2)
        { inst->opcode = XJS_BC_OP_BINNE2; }
        else if (text_item->type == xjs_ir_text_item_type_binary_e3)
        { inst->opcode = XJS_BC_OP_BINE3; }
        else if (text_item->type == xjs_ir_text_item_type_binary_ne3)
        { inst->opcode = XJS_BC_OP_BINNE3; }
        else if (text_item->type == xjs_ir_text_item_type_binary_l)
        { inst->opcode = XJS_BC_OP_BINL; }
        else if (text_item->type == xjs_ir_text_item_type_binary_le)
        { inst->opcode = XJS_BC_OP_BINLE; }
        else if (text_item->type == xjs_ir_text_item_type_binary_g)
        { inst->opcode = XJS_BC_OP_BING; }
        else if (text_item->type == xjs_ir_text_item_type_binary_ge)
        { inst->opcode = XJS_BC_OP_BINGE; }
        else if (text_item->type == xjs_ir_text_item_type_binary_and)
        { inst->opcode = XJS_BC_OP_BINAND; }
        else if (text_item->type == xjs_ir_text_item_type_binary_or)
        { inst->opcode = XJS_BC_OP_BINOR; }
        inst->operand.as_binary_op.dst = XJS_C4_TSA_VREG(text_item->u.as_binary_op.dst);
        inst->operand.as_binary_op.lhs = XJS_C4_TSA_VREG(text_item->u.as_binary_op.lhs);
        inst->operand.as_binary_op.rhs = XJS_C4_TSA_VREG(text_item->u.as_binary_op.rhs);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }
    XJS_C4_TSA_END();
}

XJS_C4_TSA_DEF_HDLR(xjs_c4_tsa_hdlr_unary_op, func_ctx, text_item)
{
    XJS_C4_TSA_BEGIN();

    XJS_C4_TSA_UNUSED(text_item);
    {
        XJS_C4_TSA_DECL_ITEM(inst);
        XJS_C4_TSA_NEW_ITEM(inst);
        if (text_item->type == xjs_ir_text_item_type_unary_add)
        { inst->opcode = XJS_BC_OP_UNADD; }
        else if (text_item->type == xjs_ir_text_item_type_unary_sub)
        { inst->opcode = XJS_BC_OP_UNSUB; }
        else if (text_item->type == xjs_ir_text_item_type_unary_not)
        { inst->opcode = XJS_BC_OP_UNNOT; }
        else if (text_item->type == xjs_ir_text_item_type_unary_bnot)
        { inst->opcode = XJS_BC_OP_UNBNOT; }
        inst->operand.as_unary_op.dst = XJS_C4_TSA_VREG(text_item->u.as_unary_op.dst);
        inst->operand.as_unary_op.src = XJS_C4_TSA_VREG(text_item->u.as_unary_op.src);
        inst->offset = func_ctx->offset;
        XJS_C4_LOC_RANGE_CLONE(inst, text_item);
        XJS_C4_TSA_PUSH_ITEM(inst);
    }
    XJS_C4_TSA_END();
}

typedef struct
{
    xjs_ir_text_item_type type;
    xjs_c4_annotate_handler handler;
} xjs_c4_text_item_handler_map_item_t;

xjs_c4_annotate_handler xjs_c4_text_section_annotate_get_handler(xjs_ir_text_item_ref text_item)
{
    static xjs_c4_text_item_handler_map_item_t map_items[] =
    {
        {xjs_ir_text_item_type_nop, xjs_c4_tsa_hdlr_nop},
        {xjs_ir_text_item_type_halt, xjs_c4_tsa_hdlr_halt},
        {xjs_ir_text_item_type_dynlib, xjs_c4_tsa_hdlr_dynlib},
        {xjs_ir_text_item_type_br, xjs_c4_tsa_hdlr_br},
        {xjs_ir_text_item_type_br_cond, xjs_c4_tsa_hdlr_br_cond},
        {xjs_ir_text_item_type_merge, xjs_c4_tsa_hdlr_merge},
        {xjs_ir_text_item_type_inspect, xjs_c4_tsa_hdlr_inspect},
        {xjs_ir_text_item_type_push_scope, xjs_c4_tsa_hdlr_unsupported},
        {xjs_ir_text_item_type_pop_scope, xjs_c4_tsa_hdlr_unsupported},
        {xjs_ir_text_item_type_alloca, xjs_c4_tsa_hdlr_do_nothing},
        {xjs_ir_text_item_type_ret, xjs_c4_tsa_hdlr_ret},
        {xjs_ir_text_item_type_make_function, xjs_c4_tsa_hdlr_make_function},
        {xjs_ir_text_item_type_call, xjs_c4_tsa_hdlr_call},
        {xjs_ir_text_item_type_new, xjs_c4_tsa_hdlr_new},
        {xjs_ir_text_item_type_this, xjs_c4_tsa_hdlr_this},
        {xjs_ir_text_item_type_label, xjs_c4_tsa_hdlr_do_nothing},
        {xjs_ir_text_item_type_declvar, xjs_c4_tsa_hdlr_declvar},
        {xjs_ir_text_item_type_store, xjs_c4_tsa_hdlr_store},
        {xjs_ir_text_item_type_load, xjs_c4_tsa_hdlr_load},
        {xjs_ir_text_item_type_load_undefined, xjs_c4_tsa_hdlr_load_undefined},
        {xjs_ir_text_item_type_load_null, xjs_c4_tsa_hdlr_load_null},
        {xjs_ir_text_item_type_load_bool, xjs_c4_tsa_hdlr_load_bool},
        {xjs_ir_text_item_type_load_number, xjs_c4_tsa_hdlr_load_number},
        {xjs_ir_text_item_type_load_string, xjs_c4_tsa_hdlr_load_string},
        {xjs_ir_text_item_type_load_object, xjs_c4_tsa_hdlr_load_object},
        {xjs_ir_text_item_type_load_array, xjs_c4_tsa_hdlr_load_array},
        {xjs_ir_text_item_type_object_get, xjs_c4_tsa_hdlr_object_get},
        {xjs_ir_text_item_type_object_set, xjs_c4_tsa_hdlr_object_set},
        {xjs_ir_text_item_type_array_push, xjs_c4_tsa_hdlr_array_push},
        {xjs_ir_text_item_type_binary_add, xjs_c4_tsa_hdlr_binary_op},
        {xjs_ir_text_item_type_binary_sub, xjs_c4_tsa_hdlr_binary_op},
        {xjs_ir_text_item_type_binary_mul, xjs_c4_tsa_hdlr_binary_op},
        {xjs_ir_text_item_type_binary_div, xjs_c4_tsa_hdlr_binary_op},
        {xjs_ir_text_item_type_binary_mod, xjs_c4_tsa_hdlr_binary_op},
        {xjs_ir_text_item_type_binary_e2, xjs_c4_tsa_hdlr_binary_op},
        {xjs_ir_text_item_type_binary_ne2, xjs_c4_tsa_hdlr_binary_op},
        {xjs_ir_text_item_type_binary_e3, xjs_c4_tsa_hdlr_binary_op},
        {xjs_ir_text_item_type_binary_ne3, xjs_c4_tsa_hdlr_binary_op},
        {xjs_ir_text_item_type_binary_l, xjs_c4_tsa_hdlr_binary_op},
        {xjs_ir_text_item_type_binary_le, xjs_c4_tsa_hdlr_binary_op},
        {xjs_ir_text_item_type_binary_g, xjs_c4_tsa_hdlr_binary_op},
        {xjs_ir_text_item_type_binary_ge, xjs_c4_tsa_hdlr_binary_op},
        {xjs_ir_text_item_type_binary_and, xjs_c4_tsa_hdlr_binary_op},
        {xjs_ir_text_item_type_binary_or, xjs_c4_tsa_hdlr_binary_op},
        {xjs_ir_text_item_type_unary_add, xjs_c4_tsa_hdlr_unary_op},
        {xjs_ir_text_item_type_unary_sub, xjs_c4_tsa_hdlr_unary_op},
        {xjs_ir_text_item_type_unary_not, xjs_c4_tsa_hdlr_unary_op},
        {xjs_ir_text_item_type_unary_bnot, xjs_c4_tsa_hdlr_unary_op},
    };
    xjs_c4_text_item_handler_map_item_t *cur = map_items;

    for (;;)
    {
        if (cur->handler == NULL) { break; }
        if (cur->type == text_item->type) { return cur->handler; }

        cur++;
    }

    return NULL;
}

/* Measure the size of each IR text item */
int xjs_c4_text_item_instrument_size( \
        xjs_c4_ctx *ctx, xjs_ir_text_item_ref item)
{
    int ret = 0;

    (void)ctx;

    switch (item->type)
    {
        case xjs_ir_text_item_type_nop:
        case xjs_ir_text_item_type_halt:
            /* No operand instructions */
            ret = XJS_BC_SIZE_OPCODE;
            break;
        case xjs_ir_text_item_type_dynlib:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG + XJS_BC_SIZE_REG;
            break;
        case xjs_ir_text_item_type_push_scope:
        case xjs_ir_text_item_type_pop_scope:
            /* Not supported */
            ret = -1;
            break;
        case xjs_ir_text_item_type_label:
            /* Label doesn't occupies any space */
            ret = 0;
            break;
        case xjs_ir_text_item_type_br:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_ADDR;
            break;
        case xjs_ir_text_item_type_br_cond:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG + XJS_BC_SIZE_ADDR;
            break;
        case xjs_ir_text_item_type_merge:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG * 4;
            break;
        case xjs_ir_text_item_type_alloca:
            /* Alloca semanticly allocates an unused register,
             * but it should be sched by allocator, so no space required */
            ret = 0;
            break;
        case xjs_ir_text_item_type_load_undefined:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG;
            break;
        case xjs_ir_text_item_type_load_null:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG;
            break;
        case xjs_ir_text_item_type_load_bool:
            /* opcode here already contains true or false info */
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG;
            break;
        case xjs_ir_text_item_type_load_string:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG + XJS_BC_SIZE_ADDR;
            break;
        case xjs_ir_text_item_type_load_number:
            /* TODO: 0, 1 or other constants, uses shorter encoding scheme
             * which involves special opcode without argument */
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG;
            {
                double value = item->u.as_load_number.value;
                if (IS_INTEGER(value))
                {
                    int value_int = (int)(value);

                    if (value_int < 0)
                    {
                        if (IN_RANGE(value_int, -128, 0)) ret += 1;
                        else if (IN_RANGE(value_int, -32768, -128)) ret += 2;
                        else ret += 4;
                    }
                    else
                    {
                        if (IN_RANGE(value_int, 0, 128)) ret += 1;
                        else if (IN_RANGE(value_int, 128, 32768)) ret += 2;
                        else ret += 4;
                    }
                }
                else
                {
                    /* TODO: Add 32-bit float-point number */
                    /* Use double-precision currently */
                    ret += 8;
                }
            }
            break;
        case xjs_ir_text_item_type_load_object:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG;
            break;
        case xjs_ir_text_item_type_load_array:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG;
            break;
        case xjs_ir_text_item_type_declvar:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_ADDR;
            break;
        case xjs_ir_text_item_type_load:
        case xjs_ir_text_item_type_store:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG + XJS_BC_SIZE_ADDR;
            break;
        case xjs_ir_text_item_type_object_set:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG + XJS_BC_SIZE_REG + XJS_BC_SIZE_REG + XJS_BC_SIZE_REG;
            break;
        case xjs_ir_text_item_type_object_get:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG + XJS_BC_SIZE_REG + XJS_BC_SIZE_REG;
            break;
        case xjs_ir_text_item_type_array_push:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG + XJS_BC_SIZE_REG;
            break;
        case xjs_ir_text_item_type_make_function:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG + XJS_BC_SIZE_FUNCTIONID;
            break;
        case xjs_ir_text_item_type_make_arrow_function:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG + XJS_BC_SIZE_FUNCTIONID;
            break;
        case xjs_ir_text_item_type_inspect:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG;
            break;
        case xjs_ir_text_item_type_ret:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG;
            break;
        case xjs_ir_text_item_type_call:
            if (item->u.as_call.bound_this.enabled == xjs_false)
            {
                /* <opcode> <dst:reg> <callee:u8> <argc:u8> <argv:[regs]> */
                ret = XJS_BC_SIZE_OPCODE + \
                      XJS_BC_SIZE_REG + \
                      XJS_BC_SIZE_REG + \
                      (int)(1) + \
                      (int)(ect_list_size(xjs_ir_var_list, item->u.as_call.arguments));
            }
            else
            {
                /* <opcode> <dst:reg> <callee:u8> <this:u8> <argc:u8> <argv:[regs]> */
                ret = XJS_BC_SIZE_OPCODE + \
                      XJS_BC_SIZE_REG + \
                      XJS_BC_SIZE_REG + \
                      XJS_BC_SIZE_REG + \
                      (int)(1) + \
                      (int)(ect_list_size(xjs_ir_var_list, item->u.as_call.arguments));
            }
            break;
        case xjs_ir_text_item_type_new:
            /* <opcode> <dst:reg> <callee:u8> <argc:u8> <argv:[regs]> */
            ret = XJS_BC_SIZE_OPCODE + \
                  XJS_BC_SIZE_REG + \
                  XJS_BC_SIZE_REG + \
                  (int)(1) + \
                  (int)(ect_list_size(xjs_ir_var_list, item->u.as_new.arguments));
            break;
        case xjs_ir_text_item_type_this:
            /* <opcode> <dst:reg>  */
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG;
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
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG * 3;
            break;

        case xjs_ir_text_item_type_unary_not:
        case xjs_ir_text_item_type_unary_bnot:
        case xjs_ir_text_item_type_unary_add:
        case xjs_ir_text_item_type_unary_sub:
            ret = XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_REG + XJS_BC_SIZE_REG;
            break;
    }

    /*
    goto done;
fail:
    ret = -1;
done:
*/
    return ret;
}

