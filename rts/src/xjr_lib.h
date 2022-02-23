/* XenonJS : Runtime Time System : Library Development Header
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_LIB_H
#define XJR_LIB_H

#include "xjr_dt.h"
#include "xjr_env.h"
#include "xjr_nativefn.h"
#include "xjr_val.h"
#include "xjr_vm.h"
#include "xjr_vm_inspect.h"
#include "xjr_bltin_helper.h"

#define xjr_lib_export_default() \
    xjr_val obj_default; \
    obj_default = xjr_val_make_object(ctx->vm->rts.rheap.mp); \
    xjr_val_properties *props_exports = xjr_val_as_object_property_get(ctx->vm->rts.rheap.mp, ctx->exports); \
    xjr_val_properties *props_default = xjr_val_as_object_property_get(ctx->vm->rts.rheap.mp, obj_default); \
    xjr_val_properties_set_by_name(ctx->vm->rts.rheap.mp, props_exports, \
            XJR_VAL_PROPERTY_TYPE_NORMAL, "default", 7, obj_default); \

#define xjr_lib_export_default_item(_name, _name_len, _value) \
    do { \
        xjr_val v = (_value); \
        xjr_val_properties_set_by_name(ctx->vm->rts.rheap.mp, props_default, \
                XJR_VAL_PROPERTY_TYPE_NORMAL, _name, _name_len, v); \
        xjr_val_properties_set_by_name(ctx->vm->rts.rheap.mp, props_exports, \
                XJR_VAL_PROPERTY_TYPE_NORMAL, _name, _name_len, v); \
    } while (0)

#endif

