/* XenonJS : Runtime Time System : Library : Host
 * Copyright(c) 2017 y2c2 */

#include "xjr_lib.h"
#include "xjr_host.h"

static int console_log_cb( \
        void *data, char *s, xjr_size_t len)
{
    xjr_native_fn_args *args = data;
    args->extn->cb_write(s, len);
    return 0;
}

/* console.log(obj1 [, obj2, ..., objN]); */
/* console.log(msg [, subst1, ..., substN]); */
/* https://developer.mozilla.org/en-US/docs/Web/API/Console/log */
static void console_log(xjr_native_fn_args *args)
{
    xjr_val v = XJR_VAL_MAKE_UNDEFINED();

    if (args->argc >= 1) { v = args->argv[0]; }

    xjr_vm_inspect(args->mp, 
            args, console_log_cb, \
            v, \
            XJR_VM_INSPECT_MODE_JSON_PRINT);

    args->extn->cb_write("\n", 1);
}

int xjr_host_bindings_init(xjr_vm *vm)
{
    int ret = 0;
    xjr_urid_t env = vm->env;
    xjr_mp_t *mp = vm->rts.rheap.mp;

    /* 'console' object */
    {
        xjr_val obj_console;

        /* 'console' */
        XJR_BLTIN_CREATE_OBJ(obj_console, xjr_val_make_object(mp), \
                vm->fundamental.global_object_prototype, XJR_VAL_MAKE_UNDEFINED());

        XJR_BLTIN_METHOD(obj_console, "log", 3, console_log);

        /* global.console */
        V_OOM(xjr_vm_env_set_var(mp, env, "console", 7, obj_console) == 0);
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

