/* XenonJS : Runtime Time System
 * Copyright(c) 2017 y2c2 */

#include <stdarg.h>
#include "xjr_vm.h"
#include "xjr_env.h"
#include "xjr_val.h"
#include "xjr_gc.h"
#include "xjr.h"

static void xjr_vm_crash_handle(xjr_vm *vm)
{
    /* vm->rts.pc */
    if (vm->crash.cb != xjr_nullptr)
    {
        xjr_crash_ctx ctx;
        ctx.vm = vm;
        vm->crash.cb(&ctx);
    }
}

int xjr_vm_start(xjr_vm *vm)
{
    int ret = 0;
    int to_halt = 0;
    int dirty = 0;

    while (to_halt == 0)
    {
        xjr_bool step = xjr_true;

        if (vm->debugger.cb != xjr_nullptr)
        {
            xjr_debugger_ctx ctx;
            ctx.vm = vm;
            ctx.stub = vm->debugger.data;
            ctx.step = xjr_false;
            vm->debugger.cb(&ctx);
            step = ctx.step;
        }

        if (step)
        {
            if (xjr_vm_step(vm, &to_halt) != 0)
            {
                /* Crashed */
                xjr_vm_crash_handle(vm);
                goto fail;
            }
            dirty = 1;
        }

        if (dirty && !(vm->opts & XJR_VM_OPTIONS_NOGC))
        {
            if ((vm->gc.before_cb == xjr_nullptr) || \
                    ((vm->gc.before_cb != xjr_nullptr) && \
                     (vm->gc.before_cb(vm, vm->gc.trigger_stub) == xjr_true)))
            {
                xjr_gc(vm);
                if (vm->gc.after_cb != xjr_nullptr)
                { vm->gc.after_cb(vm, vm->gc.trigger_stub); }
            }
        }

        vm->profile.step++;
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

int xjr_vm_call(xjr_vm *vm, \
        xjr_val *v_dst, \
        xjr_val v_callee, \
        xjr_val v_this, \
        const int argc, \
        xjr_val *argv)
{
    int ret = 0;
    xjr_val_function *func = xjr_val_as_function_extract(vm->rts.rheap.mp, v_callee);

    switch (func->type)
    {
        case xjr_val_function_type_native:
            /* TODO: Support native function */
            goto fail;

        case xjr_val_function_type_normal:
            /* Depth checking */
            if (vm->misc.external_call_depth == XJR_VM_EXTERNAL_CALL_STACK_DEPTH_MAX)
            { XJR_ERR_UPDATE(&vm->err, XJR_ERR_SOF); return -1; }

            {
                xjr_urid_t new_env;
                if ((new_env = xjr_vm_env_new(vm->rts.rheap.mp, func->env)) == XJR_URID_INVALID)
                { XJR_ERR_UPDATE(&vm->err, XJR_ERR_OOM); return -1; }

                if (xjr_vm_stack_init_on_call(vm, new_env, v_callee, v_this, argc, argv) != 0)
                { XJR_ERR_UPDATE(&vm->err, XJR_ERR_INTERNAL); return -1; }
            }

            vm->rts.pc = func->u.as_normal.offset;

            {
                int retval;
                vm->misc.external_calls.frames[vm->misc.external_call_depth].sp = vm->rts.rstack.sp;
                vm->misc.external_calls.frames[vm->misc.external_call_depth].extern_call_dst = v_dst;
                vm->misc.external_call_depth++;
                {
                    retval = xjr_vm_start(vm);
                }
                vm->misc.external_call_depth--;
                if (retval != 0) { goto fail; }
            }

            break;
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

int xjr_vm_call2(xjr_vm *vm, \
        void *heap_data, \
        xjr_heap_malloc_callback cb_malloc, \
        xjr_heap_free_callback cb_free, \
        xjr_val *v_dst, \
        xjr_val v_callee, \
        xjr_val v_this, \
        const int argc, \
        ...)
{
    int ret = 0;
    xjr_val *argv = xjr_nullptr;

    if (argc < 0) return -1;

    if (argc != 0)
    {
        va_list ap;
        int i;

        if ((argv = (xjr_val *)cb_malloc(heap_data, sizeof(xjr_val) * (xjr_size_t)argc)) == xjr_nullptr)
        { return -1; }
        va_start(ap, argc);
        for (i = 0; i < argc; i++)
        {
            argv[i] = va_arg(ap, xjr_val);
        }
        va_end(ap);
    }

    ret = xjr_vm_call(vm, v_dst, v_callee, v_this, argc, argv);

    if (argv != xjr_nullptr) { cb_free(heap_data, argv); }
    return ret;
}


/* GC */
void xjr_vm_set_gc_step_trigger(xjr_vm *vm, \
        void *trigger_stub, \
        xjr_gc_trigger_before_cb before_cb, \
        xjr_gc_trigger_after_cb after_cb)
{
    vm->gc.trigger_stub = trigger_stub;
    vm->gc.before_cb = before_cb;
    vm->gc.after_cb = after_cb;
}

