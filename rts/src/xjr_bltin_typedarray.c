/* XenonJS : Runtime Time System : Builtin Functions : TypedArray
 * Copyright(c) 2017 y2c2 */

#include "xjr_bltin_typedarray.h"

typedef struct
{
    xjr_u8 *body;
    xjr_size_t length;
} xjr_dt_uint8array_t;

/*
static xjr_dt_uint8array_t *xjr_dt_uint8array_new(xjr_mp_t *mp, int array_length)
{
    xjr_urid_t u = xjr_mp_malloc_managed(mp, sizeof(xjr_dt_uint8array_t));
    xjr_dt_uint8array_t *new_arr = xjr_mp_get_ptr(mp, u);

    (void)array_length;

    if (new_arr == xjr_nullptr) return xjr_nullptr;

    return xjr_nullptr;
}

static void xjr_dt_uint8array_destroy(xjr_dt_uint8array_t *arr)
{
    (void)arr;
}
*/

/* new Uint8Array(); // new in ES2017
 * new Uint8Array(length);
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Uint8Array */
static void xjr_bltin_uint8array(xjr_native_fn_args *args)
{
    int array_length = 0;

    if (args->argc == 0)
    {
        array_length = 0;
    }
    else if (args->argc == 1)
    {
        if (XJR_VAL_IS_INTEGER(args->argv[0]))
        {
            array_length = XJR_VAL_AS_INTEGER_UNTAG(args->argv[0]);
            if (array_length < 0) { return; }
        }
        else { return; }
    }
    else { return; }

    /* TODO: Create the Uint8Array with specified length */
    (void)array_length;
}

int xjr_bltin_typedarray_init(xjr_vm *vm, xjr_urid_t env)
{
    xjr_mp_t *mp = vm->rts.rheap.mp;
    int ret = 0;

    /* JSON */
    {
        xjr_val obj_uint8array;

        XJR_BLTIN_CREATE_OBJ(obj_uint8array, xjr_val_make_native_function(mp, env, xjr_bltin_uint8array), \
                vm->fundamental.global_uint8array_prototype, XJR_VAL_MAKE_UNDEFINED());

        /* global.Uint8Array */
        V_OOM(xjr_vm_env_set_var(mp, env, "Uint8Array", 10, obj_uint8array) == 0);
    }

fail:
    return ret;
}

