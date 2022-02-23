/* XenonJS : Runtime Time System : Library : Buffer
 * Copyright(c) 2017 y2c2 */

#include "xjr_lib_buffer.h"

typedef struct
{

    xjr_size_t size;
} buffer;

static void buffer_alloc(xjr_native_fn_args *args)
{
    (void)args;
}

int xjr_lib_buffer(xjr_lib_ctx *ctx)
{
    int ret = 0;
    xjr_val_properties *properties = xjr_val_as_object_property_get(ctx->mp, ctx->exports);

    /* 'buffer' object */
    {
        xjr_val obj_buffer = xjr_val_make_object(ctx->mp);
        {
            xjr_val_properties *props_obj_buffer = xjr_val_as_object_property_get(ctx->mp, obj_buffer);

            /* log */
            xjr_val func_alloc = xjr_val_make_native_function( \
                    ctx->mp, ctx->env, buffer_alloc);
            xjr_val_properties_set_by_name(ctx->mp, props_obj_buffer, \
            XJR_VAL_PROPERTY_TYPE_NORMAL, \
                    "alloc", 5, func_alloc);
        }
        xjr_val_properties_set_by_name(ctx->vm->rts.rheap.mp, properties, \
            XJR_VAL_PROPERTY_TYPE_NORMAL, \
            "buffer", 6, obj_buffer);
    }

    return ret;
}

