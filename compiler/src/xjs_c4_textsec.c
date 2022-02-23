/* XenonJS : C4 : Text Section
 * Copyright(c) 2017 y2c2 */

#include <ec_algorithm.h>
#include <ec_map.h>
#include <ec_list.h>
#include "xjs_helper.h"
#include "xjs_xformat.h"
#include "xjs_c4_helper.h"
#include "xjs_c4_regalloc.h"
#include "xjs_c4_ctx.h"
#include "xjs_c4_textsec_inst.h"
#include "xjs_c4_textsec.h"

static int text_item_instrument_write(xjs_c4_ctx *ctx, \
        annotated_text_section_item_ref ti)
{
    int ret = 0;

#define WRITE_S8(_x) \
    do { xjs_bitwriter_write_s8(ctx->bw_text, (xjs_s8)(_x)); } while (0)
#define WRITE_S16(_x) \
    do { xjs_bitwriter_write_s16(ctx->bw_text, (xjs_s16)(_x)); } while (0)
#define WRITE_S32(_x) \
    do { xjs_bitwriter_write_s32(ctx->bw_text, (xjs_s32)(_x)); } while (0)
#define WRITE_U8(_x) \
    do { xjs_bitwriter_write_u8(ctx->bw_text, (xjs_u8)(_x)); } while (0)
#define WRITE_U16(_x) \
    do { xjs_bitwriter_write_u16(ctx->bw_text, (xjs_u16)(_x)); } while (0)
#define WRITE_U32(_x) \
    do { xjs_bitwriter_write_u32(ctx->bw_text, (xjs_u32)(_x)); } while (0)
#define WRITE_F64(_x) \
    do { xjs_bitwriter_write_f64(ctx->bw_text, (xjs_f64)(_x)); } while (0)

    /* Opcode */
    WRITE_U8(ti->opcode);

    /* Operands */
    switch (ti->opcode)
    {
        case XJS_BC_OP_NOP:
        case XJS_BC_OP_HALT:
            break;

        case XJS_BC_OP_INSPECT:
            WRITE_U8(ti->operand.as_inspect.reg);
            break;

        case XJS_BC_OP_DYNLIB:
            WRITE_U8(ti->operand.as_dynlib.exports);
            WRITE_U8(ti->operand.as_dynlib.module_name);
            break;

        case XJS_BC_OP_ALLOCREG:
            WRITE_U8(ti->operand.as_allocreg.cnt);
            break;

        case XJS_BC_OP_ARG:
            WRITE_U32(ti->operand.as_arg.offset_data);
            WRITE_U8(ti->operand.as_arg.idx);
            break;

        case XJS_BC_OP_BR:
            WRITE_U32(ti->operand.as_br.dest);
            break;

        case XJS_BC_OP_BRC:
            WRITE_U8(ti->operand.as_br_cond.cond);
            WRITE_U32(ti->operand.as_br_cond.dest);
            break;

        case XJS_BC_OP_MERGE:
            WRITE_U8(ti->operand.as_merge.dst);
            WRITE_U8(ti->operand.as_merge.test);
            WRITE_U8(ti->operand.as_merge.consequent);
            WRITE_U8(ti->operand.as_merge.alternate);
            break;

        case XJS_BC_OP_THIS:
            WRITE_U8(ti->operand.as_this.dst);
            break;

        case XJS_BC_OP_CALL:
            WRITE_U8(ti->operand.as_call.dst);
            WRITE_U8(ti->operand.as_call.callee);
            WRITE_U8((xjs_u8)ti->operand.as_call.argc);
            {
                ec_size_t i;
                for (i = 0; i != ti->operand.as_call.argc; i++)
                {
                    WRITE_U8((xjs_u8)ti->operand.as_call.argv[i]);
                }
            }
            break;

        case XJS_BC_OP_CALLT:
            WRITE_U8(ti->operand.as_callt.dst);
            WRITE_U8(ti->operand.as_callt.callee);
            WRITE_U8(ti->operand.as_callt._this);
            WRITE_U8((xjs_u8)ti->operand.as_callt.argc);
            {
                ec_size_t i;
                for (i = 0; i != ti->operand.as_callt.argc; i++)
                {
                    WRITE_U8((xjs_u8)ti->operand.as_callt.argv[i]);
                }
            }
            break;

        case XJS_BC_OP_NEW:
            WRITE_U8(ti->operand.as_new.dst);
            WRITE_U8(ti->operand.as_new.callee);
            WRITE_U8((xjs_u8)ti->operand.as_new.argc);
            {
                ec_size_t i;
                for (i = 0; i != ti->operand.as_new.argc; i++)
                {
                    WRITE_U8((xjs_u8)ti->operand.as_new.argv[i]);
                }
            }
            break;

        case XJS_BC_OP_RET:
            WRITE_U8(ti->operand.as_ret.reg);
            break;

        case XJS_BC_OP_DECLVAR:
            WRITE_S32(ti->operand.as_declvar.offset_data);
            break;
        case XJS_BC_OP_LOAD:
            WRITE_U8(ti->operand.as_load.reg);
            WRITE_S32(ti->operand.as_load.offset_data);
            break;
        case XJS_BC_OP_STORE:
            WRITE_U8(ti->operand.as_store.reg);
            WRITE_S32(ti->operand.as_store.offset_data);
            break;

        case XJS_BC_OP_OBJSET:
            WRITE_U8(ti->operand.as_objset.dst);
            WRITE_U8(ti->operand.as_objset.obj);
            WRITE_U8(ti->operand.as_objset.member);
            WRITE_U8(ti->operand.as_objset.src);
            break;

        case XJS_BC_OP_OBJGET:
            WRITE_U8(ti->operand.as_objget.dst);
            WRITE_U8(ti->operand.as_objget.obj);
            WRITE_U8(ti->operand.as_objget.member);
            break;

        case XJS_BC_OP_ARRPUSH:
            WRITE_U8(ti->operand.as_arrpush.arr);
            WRITE_U8(ti->operand.as_arrpush.elem);
            break;

        case XJS_BC_OP_LOAD_UNDEFINED:
        case XJS_BC_OP_LOAD_NULL:
        case XJS_BC_OP_LOAD_FALSE:
        case XJS_BC_OP_LOAD_TRUE:
            WRITE_U8(ti->operand.as_load_literal.reg);
            break;

        case XJS_BC_OP_LOAD_OBJECT:
            WRITE_U8(ti->operand.as_load_object.reg);
            break;

        case XJS_BC_OP_LOAD_ARRAY:
            WRITE_U8(ti->operand.as_load_array.reg);
            break;

        case XJS_BC_OP_LOAD_NUMBER_S8:
            WRITE_U8(ti->operand.as_load_literal.reg);
            WRITE_S8(ti->operand.as_load_literal.u.as_s8);
            break;

        case XJS_BC_OP_LOAD_NUMBER_S16:
            WRITE_U8(ti->operand.as_load_literal.reg);
            WRITE_S16(ti->operand.as_load_literal.u.as_s16);
            break;

        case XJS_BC_OP_LOAD_NUMBER_S32:
            WRITE_U8(ti->operand.as_load_literal.reg);
            WRITE_S32(ti->operand.as_load_literal.u.as_s32);
            break;

        case XJS_BC_OP_LOAD_NUMBER_U8:
            WRITE_U8(ti->operand.as_load_literal.reg);
            WRITE_U8(ti->operand.as_load_literal.u.as_u8);
            break;

        case XJS_BC_OP_LOAD_NUMBER_U16:
            WRITE_U8(ti->operand.as_load_literal.reg);
            WRITE_U16(ti->operand.as_load_literal.u.as_u16);
            break;

        case XJS_BC_OP_LOAD_NUMBER_U32:
            WRITE_U8(ti->operand.as_load_literal.reg);
            WRITE_U32(ti->operand.as_load_literal.u.as_u32);
            break;

        case XJS_BC_OP_LOAD_NUMBER_F32:
            XJS_ERROR_INTERNAL(ctx->err);
            break;

        case XJS_BC_OP_LOAD_NUMBER_F64:
            WRITE_U8(ti->operand.as_load_literal.reg);
            WRITE_F64(ti->operand.as_load_literal.u.as_f64);
            break;

        case XJS_BC_OP_LOAD_STRING_EMPTY:
            WRITE_U8(ti->operand.as_load_string.reg);
            break;
        case XJS_BC_OP_LOAD_STRING:
            WRITE_U8(ti->operand.as_load_string.reg);
            WRITE_S32(ti->operand.as_load_string.offset_data);
            break;

        case XJS_BC_OP_LOAD_FUNCTION:
            WRITE_U8(ti->operand.as_load_function.reg);
            WRITE_U32(ti->operand.as_load_function.offset_text);
            break;

        case XJS_BC_OP_BINADD:
        case XJS_BC_OP_BINSUB:
        case XJS_BC_OP_BINMUL:
        case XJS_BC_OP_BINDIV:
        case XJS_BC_OP_BINMOD:
        case XJS_BC_OP_BINE2:
        case XJS_BC_OP_BINNE2:
        case XJS_BC_OP_BINE3:
        case XJS_BC_OP_BINNE3:
        case XJS_BC_OP_BINL:
        case XJS_BC_OP_BINLE:
        case XJS_BC_OP_BING:
        case XJS_BC_OP_BINGE:
        case XJS_BC_OP_BINAND:
        case XJS_BC_OP_BINOR:
            WRITE_U8(ti->operand.as_binary_op.dst);
            WRITE_U8(ti->operand.as_binary_op.lhs);
            WRITE_U8(ti->operand.as_binary_op.rhs);
            break;

        case XJS_BC_OP_UNADD:
        case XJS_BC_OP_UNSUB:
        case XJS_BC_OP_UNNOT:
        case XJS_BC_OP_UNBNOT:
            WRITE_U8(ti->operand.as_unary_op.dst);
            WRITE_U8(ti->operand.as_unary_op.src);
            break;

        default:
            XJS_ERROR_INTERNAL(ctx->err);
            break;
    }

#undef WRITE_U8
#undef WRITE_U16
#undef WRITE_U32
#undef WRITE_S8
#undef WRITE_S16
#undef WRITE_S32

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

int xjs_c4_write_text(xjs_c4_ctx *ctx)
{
    int ret = 0;

    /* Section signature */
    xjs_bitwriter_append(ctx->bw_text, \
            XJS_BC_SECTION_SIGNATURE_TEXT, XJS_BC_SECTION_SIGNATURE_LEN);

    /* Functions */
    {
        ect_iterator(annotated_function_list) it_func;
        ect_for(annotated_function_list, ctx->annotated_functions, it_func)
        {
            annotated_function_ref func = ect_deref(annotated_function_ref, it_func);
            ect_iterator(annotated_text_section_item_list) it_ti;
            ect_for(annotated_text_section_item_list, func->items, it_ti)
            {
                annotated_text_section_item_ref ti = ect_deref(annotated_text_section_item_ref, it_ti);
                if (text_item_instrument_write(ctx, ti) != 0)
                {
                    goto fail;
                }
            }
        }
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

