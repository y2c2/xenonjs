/* XenonJS : Runtime Time System : Builtin Functions : Math
 * Copyright(c) 2017 y2c2 */

#include <math.h>
#include "xjr_bltin_math.h"

#ifndef ABS
#define ABS(x) (((x) > 0) ? (x) : -(x))
#endif


/* Math.abs(x) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/abs */
static void xjr_bltin_math_abs(xjr_native_fn_args *args)
{
    xjr_val x = XJR_VAL_MAKE_UNDEFINED();

    if (args->argc >= 1) { x = args->argv[0]; }

    if (XJR_VAL_IS_INTEGER(x))
    {
        args->ret = XJR_VAL_MAKE_INTEGER(ABS((int)XJR_VAL_AS_INTEGER_UNTAG(x)));
        return;
    }
    else if (XJR_VAL_IS_FLOAT(x))
    {
        xjr_val_float_type type = xjr_val_as_float_type(args->mp, x);
        switch (type)
        {
            case xjr_val_float_type_f32:
                args->ret = xjr_val_make_f32(args->mp, ABS(xjr_val_as_float_extract_f32(args->mp, x)));
                break;
            case xjr_val_float_type_f64:
                args->ret = xjr_val_make_f64(args->mp, ABS(xjr_val_as_float_extract_f64(args->mp, x)));
                break;
        }
    }
    else
    {
        return;
    }
}

/* Math.floor(x) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/floor */
static void xjr_bltin_math_floor(xjr_native_fn_args *args)
{
    xjr_val x = XJR_VAL_MAKE_UNDEFINED();

    if (args->argc >= 1) { x = args->argv[0]; }

    if (XJR_VAL_IS_INTEGER(x))
    {
        args->ret = x;
        return;
    }
    else if (XJR_VAL_IS_FLOAT(x))
    {
        xjr_f64 x_f64 = 0.0;
        int base;
        xjr_val_float_type type = xjr_val_as_float_type(args->mp, x);
        switch (type)
        {
            case xjr_val_float_type_f32:
                x_f64 = (xjr_f64)xjr_val_as_float_extract_f32(args->mp, x);
                break;
            case xjr_val_float_type_f64:
                x_f64 = xjr_val_as_float_extract_f64(args->mp, x);
                break;
        }
        base = (int)x_f64;
        if ((ABS((xjr_f64)(base) - x_f64) > 0.000001) && (base <= 0)) { base -= 1; }
        args->ret = XJR_VAL_MAKE_INTEGER(base);
    }
    else
    {
        return;
    }
}

/* Math.max([value1[, value2[, ...]]]) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/max */
static void xjr_bltin_math_compare(xjr_native_fn_args *args, const int is_max)
{
    xjr_size_t i;
    int is_int = 1;
    union
    {
        xjr_f64 as_f64;
        int as_int;
    } maximum_v, v;

    if (args->argc == 0) { return; }

    for (i = 0; i < args->argc; i++)
    {
        if (XJR_VAL_IS_NUMBER(args->argv[0]))
        {
            if (is_int == 1 && XJR_VAL_IS_FLOAT(args->argv[0]))
            { is_int = 0; }
        }
        else
        { return; }
    }

    if (is_int == 0)
    { maximum_v.as_f64 = xjr_val_extract_f64(args->mp, args->argv[0]); }
    else
    { maximum_v.as_int = XJR_VAL_AS_INTEGER_UNTAG(args->argv[0]); }

    for (i = 1; i < args->argc; i++)
    {
        if (is_int == 0)
        {
            v.as_f64 = xjr_val_extract_f64(args->mp, args->argv[i]);
            if (is_max)
            {
                if (v.as_f64 > maximum_v.as_f64) {
                    maximum_v.as_f64 = v.as_f64;
                }
            }
            else
            {
                if (v.as_f64 < maximum_v.as_f64) {
                    maximum_v.as_f64 = v.as_f64;
                }
            }
        }
        else
        {
            v.as_int = XJR_VAL_AS_INTEGER_UNTAG(args->argv[i]);
            if (is_max)
            {
                if (v.as_int > maximum_v.as_int) {
                    maximum_v.as_int = v.as_int;
                }
            }
            else
            {
                if (v.as_int < maximum_v.as_int) {
                    maximum_v.as_int = v.as_int;
                }
            }
        }
    }

    if (is_int == 0)
    {
        args->ret = xjr_val_make_f64(args->mp, maximum_v.as_f64);
    }
    else
    {
        args->ret = XJR_VAL_MAKE_INTEGER(maximum_v.as_int);
    }
}

/* Math.max([value1[, value2[, ...]]]) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/max */
static void xjr_bltin_math_max(xjr_native_fn_args *args)
{
    xjr_bltin_math_compare(args, 1);
}

static void xjr_bltin_math_min(xjr_native_fn_args *args)
{
    xjr_bltin_math_compare(args, 0);
}

/* Math.sqrt(x) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/sqrt */
static void xjr_bltin_math_sqrt(xjr_native_fn_args *args)
{
    xjr_val x = XJR_VAL_MAKE_UNDEFINED();
    xjr_f64 x_f64 = 0.0;

    if (args->argc >= 1) { x = args->argv[0]; }

    if (XJR_VAL_IS_INTEGER(x))
    {
        x_f64 = (xjr_f64)XJR_VAL_AS_INTEGER_UNTAG(x);
    }
    else if (XJR_VAL_IS_FLOAT(x))
    {
        xjr_val_float_type type = xjr_val_as_float_type(args->mp, x);
        switch (type)
        {
            case xjr_val_float_type_f32:
                x_f64 = (xjr_f64)xjr_val_as_float_extract_f32(args->mp, x);
                break;
            case xjr_val_float_type_f64:
                x_f64 = xjr_val_as_float_extract_f64(args->mp, x);
                break;
        }
    }
    else
    {
        return;
    }

    args->ret = xjr_val_make_f64(args->mp, sqrt(x_f64));
}

int xjr_bltin_math_init(xjr_vm *vm, xjr_urid_t env)
{
    xjr_mp_t *mp = vm->rts.rheap.mp;
    int ret = 0;

    /* Math */
    {
        xjr_val obj_math;

        /* Math */
        XJR_BLTIN_CREATE_OBJ(obj_math, xjr_val_make_object(mp), \
                vm->fundamental.global_object_prototype, XJR_VAL_MAKE_UNDEFINED());

        XJR_BLTIN_METHOD(obj_math, "abs", 3, xjr_bltin_math_abs);
        XJR_BLTIN_METHOD(obj_math, "floor", 5, xjr_bltin_math_floor);
        XJR_BLTIN_METHOD(obj_math, "max", 3, xjr_bltin_math_max);
        XJR_BLTIN_METHOD(obj_math, "min", 3, xjr_bltin_math_min);
        XJR_BLTIN_METHOD(obj_math, "sqrt", 4, xjr_bltin_math_sqrt);

        /* global.JSON */
        V_OOM(xjr_vm_env_set_var(mp, env, "Math", 4, obj_math) == 0);
    }

fail:

    return ret;
}

