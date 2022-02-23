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
#include "xjs_c4_ant.h"

static int function_boilerplate_instrument_size(xjs_ir_function_ref func)
{
    int ret = 0;

    /* Parameters */
    ret += (XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_ADDR + XJS_BC_SIZE_REG) * \
           ect_list_size(xjs_ir_parameter_list, func->parameters);

    /* Allocate registers (XJS_BC_OP_ALLOCREG)
     * TODO: if no virtual variable required, this is probably not required? */
    /* <opcode> <number of registers> */
    ret += XJS_BC_SIZE_OPCODE + 1;

    return ret;
}


#define FOR_EACH_IR_FUNC_BEGIN(_func) \
        ect_iterator(xjs_ir_function_list) it_func; \
        ect_for(xjs_ir_function_list, ctx->ir->text->functions, it_func) { \
            xjs_ir_function_ref _func = ect_deref(xjs_ir_function_ref, it_func); \

#define FOR_EACH_IR_FUNC_END() }

#define FOR_EACH_IR_ITEM_BEGIN(_func, _item) \
        ect_iterator(xjs_ir_text_item_list) it; \
        ect_for(xjs_ir_text_item_list, _func->text_items, it) { \
            xjs_ir_text_item_ref _item = ect_deref(xjs_ir_text_item_ref, it); \

#define FOR_EACH_IR_ITEM_END() }

/* Extract the offset of each function by walkthough the code */
static xjs_ir_function_offset_map_ref
xjs_c4_function_offset_map_extract(xjs_c4_ctx *ctx)
{
    xjs_ir_function_offset_map_ref map_ir_function_to_offset = NULL;
    xjs_ir_offset offset;

    XJS_VNZ_ERROR_MEM(map_ir_function_to_offset = ect_map_new(xjs_ir_function_offset_map), ctx->err);

    /* Reset the offset */
    offset = 0;

    FOR_EACH_IR_FUNC_BEGIN(func)
    {
        /* Save the beginning offset of block */
        ect_map_insert(xjs_ir_function_offset_map, \
                map_ir_function_to_offset, func, offset);

        /* Top level offset */
        if (func == ctx->ir->toplevel)
        {
            ctx->offset_toplevel = offset;
        }

        offset += (xjs_ir_offset)function_boilerplate_instrument_size(func);

        FOR_EACH_IR_ITEM_BEGIN(func, item)
        {
            int instrument_size = xjs_c4_text_item_instrument_size(ctx, item);
            XJS_V_ERROR_INTERNAL((instrument_size >= 0), ctx->err);
            offset += (xjs_ir_offset)instrument_size;
        }
        FOR_EACH_IR_ITEM_END();
    }
    FOR_EACH_IR_FUNC_END();

    goto done;
fail:
    if (map_ir_function_to_offset != NULL)
    {
        ec_delete(map_ir_function_to_offset);
        map_ir_function_to_offset= NULL;
    }
done:
    return map_ir_function_to_offset;
}

/* Extract the offset of each function by walkthough the code (by function id) */
static xjs_ir_functionid_offset_map_ref
xjs_c4_functionid_offset_map_extract(xjs_c4_ctx *ctx)
{
    xjs_ir_functionid_offset_map_ref map_ir_functionid_to_offset = NULL;
    xjs_ir_offset offset;
    xjs_ir_functionid funcid = 0;

    XJS_VNZ_ERROR_MEM(map_ir_functionid_to_offset = ect_map_new(xjs_ir_functionid_offset_map), ctx->err);

    /* Reset the offset */
    offset = 0;

    FOR_EACH_IR_FUNC_BEGIN(func)
    {
        /* Save the beginning offset of block */
        ect_map_insert(xjs_ir_functionid_offset_map, \
                map_ir_functionid_to_offset, funcid, offset);
        funcid++;

        /* Top level offset */
        if (func == ctx->ir->toplevel)
        {
            ctx->offset_toplevel = offset;
        }

        offset += (xjs_ir_offset)function_boilerplate_instrument_size(func);

        FOR_EACH_IR_ITEM_BEGIN(func, item)
        {
            int instrument_size = xjs_c4_text_item_instrument_size(ctx, item);
            XJS_V_ERROR_INTERNAL((instrument_size >= 0), ctx->err);
            offset += (xjs_ir_offset)instrument_size;
        }
        FOR_EACH_IR_ITEM_END();
    }
    FOR_EACH_IR_FUNC_END();

    goto done;
fail:
    if (map_ir_functionid_to_offset != NULL)
    {
        ec_delete(map_ir_functionid_to_offset);
        map_ir_functionid_to_offset= NULL;
    }
done:
    return map_ir_functionid_to_offset;
}

/* Extract the offset of each label by walkthough the code */
static xjs_ir_label_offset_map_ref
xjs_c4_label_offset_map_extract(xjs_c4_ctx *ctx, xjs_ir_function_ref func_to_extract)
{
    xjs_ir_label_offset_map_ref map_ir_label_to_offset = NULL;
    xjs_ir_offset offset;

    XJS_VNZ_ERROR_MEM(map_ir_label_to_offset = ect_map_new(xjs_ir_label_offset_map), ctx->err);

    /* Reset the offset */
    offset = 0;

    FOR_EACH_IR_FUNC_BEGIN(func)
    {
        /* Some boilerplate instructions in the beginning of each
         * function */ 
        offset += (xjs_ir_offset)function_boilerplate_instrument_size(func);

        FOR_EACH_IR_ITEM_BEGIN(func, item)
        {
            int instrument_size = xjs_c4_text_item_instrument_size(ctx, item);
            XJS_V_ERROR_INTERNAL((instrument_size >= 0), ctx->err);
            if (func == func_to_extract)
            {
                if (item->type == xjs_ir_text_item_type_label)
                {
                    XJS_V_ERROR_INTERNAL(ect_map_count(xjs_ir_label_offset_map, \
                                map_ir_label_to_offset, item->u.as_label.lbl) == 0, ctx->err);

                    /* FIXME: Should use (func, lbl) as key instead of just (lbl) */
                    ect_map_insert(xjs_ir_label_offset_map, \
                            map_ir_label_to_offset, item->u.as_label.lbl, offset);
                }
            }
            offset += (xjs_ir_offset)instrument_size;
        }
        FOR_EACH_IR_ITEM_END();
    }
    FOR_EACH_IR_FUNC_END();

    goto done;
fail:
    if (map_ir_label_to_offset != NULL)
    {
        ec_delete(map_ir_label_to_offset);
        map_ir_label_to_offset= NULL;
    }
done:
    return map_ir_label_to_offset;
}

static xjs_regsched_ref
xjs_regsched_initialize(xjs_c4_ctx *ctx, \
        xjs_ir_function_ref func)
{
    xjs_regsched_ref sched = NULL;

    XJS_VNZ_ERROR_MEM(sched = xjs_regsched_new(ctx->err), ctx->err);
    if (xjs_regsched_set_r1(ctx->err, sched, func) != 0)
    { goto fail; }

    goto done;
fail:
    if (sched != NULL) { ec_delete(sched); sched = NULL; }
done:
    return sched;
}

annotated_function_list_ref
xjs_c4_text_section_annotate(xjs_c4_ctx *ctx)
{
    xjs_ir_function_offset_map_ref map_ir_function_to_offset = NULL;
    xjs_ir_functionid_offset_map_ref map_ir_functionid_to_offset = NULL;
    annotated_function_list_ref annotated_functions = NULL;
    xjs_regsched_ref regsched = NULL;
    xjs_c4_func_ctx func_ctx;
    annotated_function_ref new_annotated_func = NULL;
    ec_size_t point, point_boilerplate;
    xjs_size_t offset = 0;
    xjs_size_t instrument_size = 0;

    XJS_VNZ_ERROR_MEM(annotated_functions = ect_list_new(annotated_function_list), ctx->err);

    /* 1st pass, extract addresses (offset) & points of labels */
    if ((map_ir_function_to_offset = xjs_c4_function_offset_map_extract(ctx)) == NULL) { goto fail; }
    if ((map_ir_functionid_to_offset = xjs_c4_functionid_offset_map_extract(ctx)) == NULL) { goto fail; }
    ctx->map_ir_function_to_offset = map_ir_function_to_offset;
    ctx->map_ir_functionid_to_offset = map_ir_functionid_to_offset;

    /* 2nd, insert annotated items */
    func_ctx.global_ctx = ctx;

    FOR_EACH_IR_FUNC_BEGIN(func)
    {
        /* Reset point */
        point = 0;

        if ((regsched = xjs_regsched_initialize(ctx, func)) == NULL)
        { goto fail; }
        XJS_VNZ_ERROR_MEM(new_annotated_func = annotated_function_new(), ctx->err);

        new_annotated_func->offset = ect_map_get( \
                xjs_ir_function_offset_map, map_ir_function_to_offset, func);
        new_annotated_func->map_ir_label_to_offset = xjs_c4_label_offset_map_extract(ctx, func);

        /* Function regsched */
        func_ctx.regsched = regsched;
        func_ctx.annotated_function = new_annotated_func;

        /* Insert boilerplate */
        {
            point_boilerplate = 0;
            annotated_text_section_item_ref new_item;

            /* Parameters */
            {
                ec_size_t idx = 0;
                instrument_size = (XJS_BC_SIZE_OPCODE + XJS_BC_SIZE_ADDR + XJS_BC_SIZE_REG);
                ect_iterator(xjs_ir_parameter_list) it_param;
                ect_for(xjs_ir_parameter_list, func->parameters, it_param)
                {
                    xjs_ir_parameter_ref param = ect_deref(xjs_ir_parameter_ref, it_param);
                    XJS_VNZ_ERROR_MEM(new_item = annotated_text_section_item_new(), ctx->err);
                    new_item->opcode = XJS_BC_OP_ARG;
                    new_item->operand.as_arg.offset_data = (xjs_ir_offset)(ect_map_get(xjs_ir_dataid_offset_map, \
                                     ctx->map_ir_dataid_to_offset, \
                                     param->varname));
                    new_item->operand.as_arg.idx = (ec_u8)idx;
                    new_item->offset = offset;
                    idx++;
                    annotated_text_section_item_list_push_back(new_annotated_func->items, new_item);
                    point_boilerplate++;
                    offset += instrument_size;
                }
            }

            /* Allocate Registers */
            instrument_size = XJS_BC_SIZE_OPCODE + 1;
            XJS_VNZ_ERROR_MEM(new_item = annotated_text_section_item_new(), ctx->err);
            new_item->opcode = XJS_BC_OP_ALLOCREG;
            new_item->operand.as_allocreg.cnt = xjs_regsched_requirement(regsched);
            new_item->offset = offset;
            annotated_text_section_item_list_push_back(new_annotated_func->items, new_item);
            point_boilerplate++;
            offset += instrument_size;
        }

        /* Instructions of Body */
        FOR_EACH_IR_ITEM_BEGIN(func, item)
        {
            xjs_c4_annotate_handler handler;

            /* Find handler to handle the current text item */
            if ((handler = xjs_c4_text_section_annotate_get_handler(item)) == NULL)
            { XJS_ERROR_INTERNAL(ctx->err); }

            /* Fill missing information */
            func_ctx.point = point;

            func_ctx.offset = offset;

            /* Process the text item */
            if (handler(&func_ctx, item) != 0) { goto fail; }

            instrument_size = (xjs_size_t)xjs_c4_text_item_instrument_size(ctx, item);
            offset += instrument_size;

            /* Move point */
            point++;
        }
        FOR_EACH_IR_ITEM_END();
        annotated_function_list_push_back(annotated_functions, new_annotated_func);
        new_annotated_func = NULL;

        ec_delete(regsched); regsched = NULL;
    }
    FOR_EACH_IR_FUNC_END();

    goto done;
fail:
    if (annotated_functions != NULL)
    {
        ec_delete(annotated_functions);
        annotated_functions = NULL;
    }
done:
    ec_delete(new_annotated_func);
    ec_delete(regsched);
    return annotated_functions;
}

