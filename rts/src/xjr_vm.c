/* XenonJS : Runtime Time System : VM
 * Copyright(c) 2017 y2c2 */

#include "xjr_dt.h"
#include "xjr_libc.h"
#include "xjr_mp.h"
#include "xjr_val.h"
#include "xjr_env.h"
#include "xjr_xformat.h"
#include "xjr_vm_stkmap.h"
#include "xjr_nativefn.h"
#include "xjr_vm_inspect.h"
#include "xjr_bltin.h"
#include "xjr_vm.h"

/* Error */
/*
static void xjr_vm_error_update(xjr_vm *vm, \
        int err_code, char *filename, int ln)
{
    vm->err.code = err_code;
    vm->err.loc.filename = filename;
    vm->err.loc.ln = ln;
}
*/

/* Load bytecode */

int xjr_vm_load(xjr_vm *vm, char *bytecode, xjr_size_t bytecode_len)
{
    if (xjr_xfile_load(&vm->xf, bytecode, bytecode_len) != 0)
    {
        XJR_ERR_UPDATE(&vm->err, XJR_ERR_XCORRUPT);
        return -1;
    }
    vm->rts.pc = vm->xf.init_pc;
    return 0;
}

static void xjr_vm_op_inspect(xjr_vm *vm, xjr_val v);

/* Helpers */

static int xjr_vm_read_symbol(xjr_vm *vm, \
        char **body_out, xjr_size_t *len_out, \
        xjr_offset_t offset)
{
    int ret = 0;
    ret = xjr_xfile_read_symbol(&vm->xf, body_out, len_out, offset);
    if (ret != 0)
    { XJR_ERR_UPDATE(&vm->err, XJR_ERR_INTERNAL); }
    return ret;
}

static void xjr_vm_stack_init(xjr_vm *vm, \
        xjr_urid_t env, \
        xjr_val callee, \
        xjr_val _this, \
        const int argc, \
        xjr_val *argv)
{
    char *stk = (char *)vm->rts.rstack.data + vm->rts.rstack.sdp;

    /* call type */
    stk[XJR_OFFSET_SHIFT_CALLTYPE] = XJR_CALLTYPE_CALL;
    /* restore point */
    {
        xjr_u32 preserved_pc = vm->rts.pc;
        ((*((xjr_u32 *)(&((xjr_u8 *)vm->rts.rstack.data)[(vm)->rts.rstack.sdp + (XJR_OFFSET_SHIFT_RESTORE_POINT)]))) = (xjr_u32)(preserved_pc));
    }
    /*
    stk[XJR_OFFSET_SHIFT_RESTORE_POINT] = 0;
    stk[XJR_OFFSET_SHIFT_RESTORE_POINT + 1] = 0;
    stk[XJR_OFFSET_SHIFT_RESTORE_POINT + 2] = 0;
    stk[XJR_OFFSET_SHIFT_RESTORE_POINT + 3] = 0;
    */

    /* environment */
    *((xjr_u16 *)(stk + XJR_OFFSET_SHIFT_ENV)) = env;
    /* dst */
    stk[XJR_OFFSET_SHIFT_DST] = 0;
    /* callee */
    *((xjr_u32 *)(stk + XJR_OFFSET_SHIFT_CALLEE)) = callee;
    /* this */
    *((xjr_u32 *)(stk + XJR_OFFSET_SHIFT_THIS)) = _this;
    /* number of arguments */
    stk[XJR_OFFSET_SHIFT_ARGC] = (char)argc;
    /* number of vregs (set as zero) */
    stk[XJR_OFFSET_SHIFT_VREGC] = 0;
    /* arguments */
    {
        int i;
        xjr_val *argv_data_dst = (xjr_val *)(&stk[XJR_OFFSET_SHIFT_ARGV]);
        for (i = 0; i != argc; i++)
        {
            argv_data_dst[i] = argv[i];
        }
    }
}

void xjr_vm_stack_init_on_boot(xjr_vm *vm, xjr_urid_t env, xjr_val global)
{
    vm->rts.rstack.sp = vm->rts.rstack.size;
    vm->rts.rstack.sdp = 0;

    xjr_vm_stack_init(vm, env, \
            XJR_VAL_MAKE_UNDEFINED(), global, 0, xjr_nullptr);
}

int xjr_vm_stack_init_on_call(xjr_vm *vm, xjr_urid_t env, \
        xjr_val callee, \
        xjr_val _this, \
        const int argc, \
        xjr_val *argv)
{
    /* Calling from external place */

    if (vm->misc.external_call_depth == 0)
    {
        vm->rts.rstack.sp = vm->rts.rstack.size;
        vm->rts.rstack.sdp = 0;
        xjr_vm_stack_init(vm, env, callee, _this, argc, argv);
    }
    else
    {
        /* Since the calling depth is larger than 0 which indicates
         * the two native calling stack have an vm stack frame intercepted 
         *
         * <native> <vm> <native>
         *
         * That makes the 'increase SP' process essential */

        xjr_offset_t offset_next_frame = XJR_VM_GET_SF_SIZE(vm);

        /* Must have enough space to contain the stack frame */
        if (vm->rts.rstack.sdp + offset_next_frame + 64 >= vm->rts.rstack.sp)
        { return -1; }

        /* Store the offset of the new stack frame */
        XJR_VM_STACK_INCREASE_SP(vm);

        xjr_vm_stack_init(vm, env, callee, _this, argc, argv);
    }

    return 0;
}

int xjr_vm_boot(xjr_vm *vm)
{
    xjr_urid_t env = XJR_URID_INVALID;
    xjr_val global;
    
    /* Environment */
    env = xjr_vm_env_new(vm->rts.rheap.mp, XJR_URID_INVALID);
    if (env == XJR_URID_INVALID) { XJR_ERR_UPDATE(&vm->err, XJR_ERR_OOM); goto fail; }
    vm->env = env;

    /* Global */
    global = xjr_val_make_object(vm->rts.rheap.mp);
    if (global == XJR_URID_INVALID) { XJR_ERR_UPDATE(&vm->err, XJR_ERR_OOM); goto fail; }

    /* Should be 'global' */
    if (xjr_vm_env_set_var(vm->rts.rheap.mp, env, \
                "global", 6, global) != 0)
    { goto fail; }

    vm->fundamental.global_object_prototype = XJR_VAL_MAKE_UNDEFINED();
    vm->fundamental.global_function_prototype = XJR_VAL_MAKE_UNDEFINED();
    vm->fundamental.global_number_prototype = XJR_VAL_MAKE_UNDEFINED();
    vm->fundamental.global_array_prototype = XJR_VAL_MAKE_UNDEFINED();
    vm->fundamental.global_string_prototype = XJR_VAL_MAKE_UNDEFINED();
    vm->fundamental.global_uint8array_prototype = XJR_VAL_MAKE_UNDEFINED();

    /* Fundamental things */
    if (xjr_bltin_init(vm, env) != 0)
    { goto fail; }

    /* First time booting */
    xjr_vm_stack_init_on_boot(vm, env, global);

    return 0;
fail:
    return -1;
}

/* Initialize */
int xjr_vm_init(xjr_vm *vm)
{
    xjr_xfile_init(&vm->xf);
    vm->inspect.cb_write = xjr_nullptr;
    vm->opts = 0;
    vm->rts.pc = 0;
    vm->rts.rheap.data = xjr_nullptr;
    vm->rts.rheap.cb_malloc = xjr_nullptr;
    vm->rts.rheap.cb_free = xjr_nullptr;
    vm->rts.rheap.mp = xjr_nullptr;
    vm->err.code = 0;
    vm->err.rts.opcode = -1;
    vm->err.rts.pc = -1;
    vm->profile.step = 0;
    vm->libs.count = 0;
    vm->libs.name_p = vm->libs.pool;
    vm->libs.pool_size = 0;
    vm->debugger.cb = xjr_nullptr;
    vm->debugger.data = xjr_nullptr;
    vm->crash.cb = xjr_nullptr;
    xjr_extn_init(&vm->extn);
    vm->gc.trigger_stub = xjr_nullptr;
    vm->gc.before_cb = xjr_nullptr;
    vm->gc.after_cb = xjr_nullptr;
    vm->distribution_data = xjr_nullptr;
    vm->misc.external_vals.size = 0;
    vm->misc.external_call_depth = 0;

    return 0;
}

int xjr_vm_uninit(xjr_vm *vm)
{
    if (vm->rts.rheap.mp != xjr_nullptr) xjr_mp_destroy(vm->rts.rheap.mp);
    return 0;
}

/* Set options */
void xjr_vm_set_options(xjr_vm *vm, xjr_u32 options)
{
    vm->opts |= options;
}

/* Distribution Data */
void *xjr_vm_get_distribution_data(xjr_vm *vm)
{
    return vm->distribution_data;
}

void xjr_vm_set_distribution_data(xjr_vm *vm, void *data)
{
    vm->distribution_data = data;
}

void xjr_vm_setup_stack(xjr_vm *vm, void *data, xjr_size_t size)
{
    vm->rts.rstack.data = data;
    vm->rts.rstack.size = size;
}

int xjr_vm_setup_heap(xjr_vm *vm, \
        void *data, \
        xjr_heap_malloc_callback cb_malloc, \
        xjr_heap_free_callback cb_free)
{
    vm->rts.rheap.data = data;
    vm->rts.rheap.cb_malloc = cb_malloc;
    vm->rts.rheap.cb_free = cb_free;
    if ((vm->rts.rheap.mp = xjr_mp_new_opt(data, cb_malloc, cb_free, \
                    0, XJR_MP_OPTS_NOALIGN)) == xjr_nullptr)
    { return -1; }
    return 0;
}

void xjr_vm_setup_write(xjr_vm *vm, \
        xjr_inspect_write_callback cb_write)
{
    vm->inspect.cb_write = cb_write;
}

void xjr_vm_set_malloc_callback(xjr_vm *vm, xjr_heap_malloc_record_callback cb)
{
    xjr_mp_set_malloc_callback(vm->rts.rheap.mp, cb);
}

void xjr_vm_set_free_callback(xjr_vm *vm, xjr_heap_free_record_callback cb)
{
    xjr_mp_set_free_callback(vm->rts.rheap.mp, cb);
}

xjr_extn *xjr_vm_extn(xjr_vm *vm)
{
    return &vm->extn;
}

int xjr_vm_lib_install(xjr_vm *vm, const char *module_name, xjr_lib_install_cb cb)
{
    xjr_size_t module_name_len;

    if (vm->libs.count == XJR_VM_LIB_CAPACITY) return -1;
    module_name_len = xjr_strlen(module_name);
    if (vm->libs.pool_size + module_name_len > XJR_VM_LIB_POOL_SIZE) return -1;
    {
        xjr_vm_lib *lib = vm->libs.libs + vm->libs.count;
        xjr_memcpy(vm->libs.name_p, module_name, module_name_len);
        vm->libs.name_p[module_name_len] = '\0';
        lib->name = vm->libs.name_p;
        vm->libs.name_p += (module_name_len + 1);
        vm->libs.pool_size += module_name_len;
        lib->cb = cb;
        vm->libs.count++;
    }

    return 0;
}

void xjr_vm_debugger_install(xjr_vm *vm, xjr_debugger_cb cb, void *data)
{
    vm->debugger.cb = cb;
    vm->debugger.data = data;
}

void xjr_vm_crash_handler_install(xjr_vm *vm, xjr_crash_cb cb)
{
    vm->crash.cb = cb;
}

/* Get stub from debugger context */
void *xjr_debugger_stub_get(xjr_debugger_ctx *ctx)
{
    return ctx->stub;
}

#define OFFSET_PC (vm->xf.sections.text + 4 + vm->rts.pc)

#define FETCH_U8(x) \
    do { \
        if (xjr_xfile_read_u8(&vm->xf, x, OFFSET_PC) != 0) return -1; \
        vm->rts.pc++; \
    } while (0)

#define FETCH_S8(x) \
    do { \
        if (xjr_xfile_read_s8(&vm->xf, x, OFFSET_PC) != 0) return -1; \
        vm->rts.pc++; \
    } while (0)

#define FETCH_U16(x) \
    do { \
        if (xjr_xfile_read_u16(&vm->xf, x, OFFSET_PC) != 0) return -1; \
        vm->rts.pc += 2; \
    } while (0)

#define FETCH_S16(x) \
    do { \
        if (xjr_xfile_read_s16(&vm->xf, x, OFFSET_PC) != 0) return -1; \
        vm->rts.pc += 2; \
    } while (0)

#define FETCH_U32(x) \
    do { \
        if (xjr_xfile_read_u32(&vm->xf, x, OFFSET_PC) != 0) return -1; \
        vm->rts.pc += 4; \
    } while (0)

#define FETCH_S32(x) \
    do { \
        if (xjr_xfile_read_s32(&vm->xf, x, OFFSET_PC) != 0) return -1; \
        vm->rts.pc += 4; \
    } while (0)

#define FETCH_F64(x) \
    do { \
        if (xjr_xfile_read_f64(&vm->xf, x, OFFSET_PC) != 0) return -1; \
        vm->rts.pc += 8; \
    } while (0)

#define FETCH_OPCODE(_opcode) FETCH_U8(_opcode)
#define FETCH_REG(_reg) FETCH_U8(_reg)


static xjr_bool xjr_vm_cond_eq2_as_true(xjr_vm *vm, xjr_val v)
{
    if ((XJR_VAL_IS_UNDEFINED(v)) || \
            (XJR_VAL_IS_NULL(v)) || \
            (XJR_VAL_IS_BOOLEAN_FALSE(v)) || \
            (XJR_VAL_IS_INTEGER(v) && (XJR_VAL_AS_INTEGER_UNTAG(v) == 0)))
    { return xjr_false; }

    if (XJR_VAL_IS_STRING(v))
    {
        if (xjr_val_as_string_length(vm->rts.rheap.mp, v) == 0)
        { return xjr_false; }
    }

    return xjr_true;
}

static void xjr_vm_op_allocreg(xjr_vm *vm, xjr_u8 nreg)
{
    XJR_VM_SET_TO_STACK(vm, XJR_OFFSET_SHIFT_VREGC, xjr_u8, nreg);
    {
        xjr_u8 i;
        for (i = 0; i < nreg; i++)
        { XJR_VM_SET_VREGV(vm, i, XJR_VAL_MAKE_UNDEFINED()); }
    }
}

static int xjr_vm_op_call(xjr_vm *vm, \
        xjr_reg dst, \
        xjr_val callee, \
        xjr_val _this, \
        xjr_size_t argc, xjr_reg *argv)
{
    xjr_offset_t offset_next_frame = XJR_VM_GET_SF_SIZE(vm);

    if (vm->rts.rstack.sdp + offset_next_frame + 64 >= vm->rts.rstack.sp)
    {
        XJR_ERR_UPDATE(&vm->err, XJR_ERR_SOF);
        return -1;
    }

    /* Store the offset of the new stack frame */
    XJR_VM_STACK_INCREASE_SP(vm);

    {
        xjr_u8 *stk = vm->rts.rstack.data;
        xjr_val *argv_data_dst;
        xjr_size_t i;

        /* Preserve calling arguments */
        argv_data_dst = (xjr_val *)(&stk[vm->rts.rstack.sdp + offset_next_frame + XJR_OFFSET_SHIFT_ARGV]);
        for (i = 0; i != argc; i++)
        {
            argv_data_dst[i] = XJR_VM_GET_VREGV(vm, argv[i]);
        }
        vm->rts.rstack.sdp += offset_next_frame;

        /* Preserve dst */
        XJR_VM_SET_DST(vm, dst);
        XJR_VM_SET_ARGC(vm, argc);
    }

    /* Make call */
    {
        if (!XJR_VAL_IS_FUNCTION(callee))
        { XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE); return -1; }
        xjr_val_function *f = xjr_val_as_function_extract(vm->rts.rheap.mp, callee);
        /* Call type */
        XJR_VM_SET_CALLTYPE(vm, XJR_CALLTYPE_CALL);
        /* Preserve restore point */
        XJR_VM_SET_RESTORE_POINT(vm, vm->rts.pc);
        /* Preserve environment */
        {
            xjr_urid_t new_env;
            if ((new_env = xjr_vm_env_new(vm->rts.rheap.mp, f->env)) == XJR_URID_INVALID)
            { XJR_ERR_UPDATE(&vm->err, XJR_ERR_OOM); return -1; }
            XJR_VM_SET_ENV(vm, new_env);
        }
        /* Callee */
        XJR_VM_SET_CALLEE(vm, callee);
        /* this */
        XJR_VM_SET_THIS(vm, _this);
        /* Vregs */
        XJR_VM_SET_VREGC(vm, 0);

        switch (f->type)
        {
            case xjr_val_function_type_native:
                {
                    xjr_reg dst_prev;
                    xjr_native_fn_args args;

                    args.ret = XJR_VAL_MAKE_UNDEFINED();
                    args.callee = callee;
                    args.argc = argc;
                    args.argv = XJR_VM_ADDR_FROM_STACK(vm, XJR_OFFSET_SHIFT_ARGV, xjr_val);
                    args.mp = vm->rts.rheap.mp;
                    args.env = f->env;
                    args.extn = &vm->extn;
                    args._this = _this;
                    args.vm = vm;
                    f->u.as_native.cb(&args);

                    dst_prev = XJR_VM_GET_DST(vm);
                    XJR_VM_STACK_DECREASE_SP(vm);
                    XJR_VM_SET_VREGV(vm, dst_prev, args.ret);
                }
                break;

            case xjr_val_function_type_normal:
                vm->rts.pc = f->u.as_normal.offset;
                break;
        }
    }

    return 0;
}

static int xjr_vm_op_new(xjr_vm *vm, \
        xjr_reg dst, \
        xjr_val callee, \
        xjr_val _this, \
        xjr_size_t argc, xjr_reg *argv)
{
    xjr_offset_t offset_next_frame = XJR_VM_GET_SF_SIZE(vm);

    if (vm->rts.rstack.sdp + offset_next_frame + 64 >= vm->rts.rstack.sp)
    { XJR_ERR_UPDATE(&vm->err, XJR_ERR_SOF); return -1; }

    /* Store the offset of the new stack frame */
    XJR_VM_STACK_INCREASE_SP(vm);

    {
        xjr_u8 *stk = vm->rts.rstack.data;
        xjr_val *argv_data_dst;
        xjr_size_t i;

        /* Preserve calling arguments */
        argv_data_dst = (xjr_val *)(&stk[vm->rts.rstack.sdp + offset_next_frame + XJR_OFFSET_SHIFT_ARGV]);
        for (i = 0; i != argc; i++)
        {
            argv_data_dst[i] = XJR_VM_GET_VREGV(vm, argv[i]);
        }
        vm->rts.rstack.sdp += offset_next_frame;

        /* Preserve dst */
        XJR_VM_SET_DST(vm, dst);
        XJR_VM_SET_ARGC(vm, argc);
    }

    /* Make call */
    {
        if (!XJR_VAL_IS_FUNCTION(callee))
        { XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE); return -1; }
        xjr_val_function *f = xjr_val_as_function_extract(vm->rts.rheap.mp, callee);
        /* Call type */
        XJR_VM_SET_CALLTYPE(vm, XJR_CALLTYPE_NEW);
        /* Preserve restore point */
        XJR_VM_SET_RESTORE_POINT(vm, vm->rts.pc);
        /* Preserve environment */
        {
            xjr_urid_t new_env;
            if ((new_env = xjr_vm_env_new(vm->rts.rheap.mp, f->env)) == XJR_URID_INVALID)
            { XJR_ERR_UPDATE(&vm->err, XJR_ERR_OOM); return -1; }
            XJR_VM_SET_ENV(vm, new_env);
        }
        /* Callee */
        XJR_VM_SET_CALLEE(vm, callee);
        /* this */
        XJR_VM_SET_THIS(vm, _this);
        /* Vregs */
        XJR_VM_SET_VREGC(vm, 0);

        switch (f->type)
        {
            case xjr_val_function_type_native:
                XJR_ERR_UPDATE(&vm->err, XJR_ERR_INTERNAL);
                return -1;

            case xjr_val_function_type_normal:
                vm->rts.pc = f->u.as_normal.offset;
                break;
        }
    }

    return 0;
}

static int xjr_vm_op_ret(xjr_vm *vm, xjr_reg dst, int *to_halt)
{
    xjr_reg dst_prev;
    xjr_u32 new_pc;
    xjr_u8 calltype;
    int depth = (int)vm->misc.external_call_depth;

    if ((depth != 0) && \
            (vm->misc.external_calls.frames[depth - 1].sp == vm->rts.rstack.sp))
    {
        /* Reach the top level of stack frame of the external call,
         * going to exit the execution */  
        *to_halt = 1;

        /* Save the return value directly */
        {
            xjr_val *dst_p = vm->misc.external_calls.frames[depth - 1].extern_call_dst;
            if (dst_p != xjr_nullptr)
            {
                *dst_p = XJR_VM_GET_VREGV(vm, dst);
            }
        }

        new_pc = XJR_VM_GET_RESTORE_POINT(vm);

        /* Restore sp */
        /* TODO: Explain it */
        if (depth > 1)
        {
            XJR_VM_STACK_DECREASE_SP(vm);
        }

        /* Restore pc */
        vm->rts.pc = new_pc;

        return 0;
    }

    /* dst on prev frame (the returned value will be store
     * in the 'dst_prev' register on the previous frame) */
    dst_prev = XJR_VM_GET_DST(vm);

    /* Get pc to restore */
    new_pc = XJR_VM_GET_RESTORE_POINT(vm);

    calltype = XJR_VM_GET_CALLTYPE(vm);
    
    if (calltype == XJR_CALLTYPE_CALL)
    {
        xjr_val dst_data = XJR_VM_GET_VREGV(vm, dst);

        /* Restore sp */
        XJR_VM_STACK_DECREASE_SP(vm);

        /* Restore pc */
        vm->rts.pc = new_pc;

        /* Load dst in outter frame */
        XJR_VM_SET_VREGV(vm, dst_prev, dst_data);
    }
    else if (calltype == XJR_CALLTYPE_NEW)
    {
        /* We retrieve returned value and abandon it, instead of using the return value,
         * we returns 'this' */
        xjr_val dst_data = XJR_VM_GET_THIS(vm);

        /* Restore sp */
        XJR_VM_STACK_DECREASE_SP(vm);

        /* Restore pc */
        vm->rts.pc = new_pc;

        /* Load dst in outter frame */
        XJR_VM_SET_VREGV(vm, dst_prev, dst_data);
    }
    else
    {
        XJR_ERR_UPDATE(&vm->err, XJR_ERR_INTERNAL);
        return -1;
    }

    return 0;
}

static int xjr_vm_op_binary_add(xjr_vm *vm, \
        xjr_val *dst_out, xjr_val v_lhs, xjr_val v_rhs)
{
    xjr_mp_t *mp = vm->rts.rheap.mp;

    if (XJR_VAL_IS_INTEGER(v_lhs) && XJR_VAL_IS_INTEGER(v_rhs))
    {
        *dst_out = XJR_VAL_MAKE_INTEGER( \
                XJR_VAL_AS_INTEGER_UNTAG(v_lhs) + XJR_VAL_AS_INTEGER_UNTAG(v_rhs));
    }
    else if (XJR_VAL_IS_NUMBER(v_lhs) && XJR_VAL_IS_NUMBER(v_rhs))
    {
        *dst_out = xjr_val_make_f64(mp, \
                xjr_val_extract_f64(mp, v_lhs) + xjr_val_extract_f64(mp, v_rhs));
    }
    else if (XJR_VAL_IS_STRING(v_lhs) && XJR_VAL_IS_STRING(v_rhs))
    {
        xjr_size_t lhs_len = xjr_val_as_string_length(mp, v_lhs);
        xjr_size_t rhs_len = xjr_val_as_string_length(mp, v_rhs);
        xjr_urid_t u_buf;
        char *buf;
        if ((u_buf = xjr_mp_malloc(mp, lhs_len + rhs_len + 1)) == XJR_URID_INVALID)
        { XJR_ERR_UPDATE(&vm->err, XJR_ERR_OOM); return -1; }
        buf = xjr_mp_get_ptr(mp, u_buf);
        xjr_memcpy(buf, xjr_val_as_string_body(mp, v_lhs), lhs_len);
        xjr_memcpy(buf + lhs_len, xjr_val_as_string_body(mp, v_rhs), rhs_len);
        buf[lhs_len + rhs_len] = '\0';
        {
            xjr_val dst;
            dst = xjr_val_make_string_from_heap(mp, buf, lhs_len + rhs_len);
            xjr_val_properties_install(vm, dst);
            *dst_out = dst;
        }
        xjr_mp_free(mp, u_buf);
    }
    else if (XJR_VAL_IS_STRING(v_lhs) && XJR_VAL_IS_INTEGER(v_rhs))
    {
        xjr_val v_rhs_int = xjr_val_make_string_from_int(mp, XJR_VAL_AS_INTEGER_UNTAG(v_rhs));
        return xjr_vm_op_binary_add(vm, dst_out, v_lhs, v_rhs_int);
    }
    else if (XJR_VAL_IS_INTEGER(v_lhs) && XJR_VAL_IS_STRING(v_rhs))
    {
        xjr_val v_lhs_int = xjr_val_make_string_from_int(mp, XJR_VAL_AS_INTEGER_UNTAG(v_lhs));
        return xjr_vm_op_binary_add(vm, dst_out, v_lhs_int, v_rhs);
    }
    else
    {
        /* Type error */
        XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE);
        return -1;
    }

    return 0;
}

static int xjr_vm_op_binary_sub(xjr_vm *vm, \
        xjr_val *dst_out, xjr_val v_lhs, xjr_val v_rhs)
{
    xjr_mp_t *mp = vm->rts.rheap.mp;

    if (XJR_VAL_IS_INTEGER(v_lhs) && XJR_VAL_IS_INTEGER(v_rhs))
    {
        *dst_out = XJR_VAL_MAKE_INTEGER( \
                XJR_VAL_AS_INTEGER_UNTAG(v_lhs) - XJR_VAL_AS_INTEGER_UNTAG(v_rhs));
    }
    else if (XJR_VAL_IS_NUMBER(v_lhs) && XJR_VAL_IS_NUMBER(v_rhs))
    {
        *dst_out = xjr_val_make_f64(mp, \
                xjr_val_extract_f64(mp, v_lhs) - xjr_val_extract_f64(mp, v_rhs));
    }
    else
    {
        /* Type error */
        XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE);
        return -1;
    }

    return 0;
}

static int xjr_vm_op_binary_mul(xjr_vm *vm, \
        xjr_val *dst_out, xjr_val v_lhs, xjr_val v_rhs)
{
    xjr_mp_t *mp = vm->rts.rheap.mp;

    if (XJR_VAL_IS_INTEGER(v_lhs) && XJR_VAL_IS_INTEGER(v_rhs))
    {
        *dst_out = XJR_VAL_MAKE_INTEGER( \
                XJR_VAL_AS_INTEGER_UNTAG(v_lhs) * XJR_VAL_AS_INTEGER_UNTAG(v_rhs));
    }
    else if (XJR_VAL_IS_NUMBER(v_lhs) && XJR_VAL_IS_NUMBER(v_rhs))
    {
        *dst_out = xjr_val_make_f64(mp, \
                xjr_val_extract_f64(mp, v_lhs) * xjr_val_extract_f64(mp, v_rhs));
    }
    else
    {
        /* Type error */
        XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE);
        return -1;
    }

    return 0;
}

static int xjr_vm_op_binary_div(xjr_vm *vm, \
        xjr_val *dst_out, xjr_val v_lhs, xjr_val v_rhs)
{
    xjr_mp_t *mp = vm->rts.rheap.mp;

    if (XJR_VAL_IS_INTEGER(v_lhs) && XJR_VAL_IS_INTEGER(v_rhs))
    {
        int v_lhs_i = XJR_VAL_AS_INTEGER_UNTAG(v_lhs);
        int v_rhs_i = XJR_VAL_AS_INTEGER_UNTAG(v_rhs);
        if (v_lhs_i % v_rhs_i == 0)
        {
            int v_dst = v_lhs_i / v_rhs_i;
            *dst_out = XJR_VAL_MAKE_INTEGER(v_dst);
        }
        else
        {
            *dst_out = xjr_val_make_f64(mp, (xjr_f64)v_lhs_i / (xjr_f64)v_rhs_i);
        }
    }
    else if (XJR_VAL_IS_NUMBER(v_lhs) && XJR_VAL_IS_NUMBER(v_rhs))
    {
        *dst_out = xjr_val_make_f64(mp, \
                xjr_val_extract_f64(mp, v_lhs) / xjr_val_extract_f64(mp, v_rhs));
    }
    else
    {
        /* Type error */
        XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE);
        return -1;
    }

    return 0;
}

static int xjr_vm_op_binary_mod(xjr_vm *vm, \
        xjr_val *dst, xjr_val lhs, xjr_val rhs)
{
    if (XJR_VAL_IS_INTEGER(lhs) && XJR_VAL_IS_INTEGER(rhs))
    {
        int v_lhs = XJR_VAL_AS_INTEGER_UNTAG(lhs);
        int v_rhs = XJR_VAL_AS_INTEGER_UNTAG(rhs);
        int v_dst = v_lhs % v_rhs;
        *dst = XJR_VAL_MAKE_INTEGER(v_dst);
    }
    else
    {
        /* Type error */
        XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE);
        return -1;
    }

    return 0;
}

static int xjr_vm_op_binary_e2(xjr_vm *vm, \
        xjr_val *dst, xjr_val lhs, xjr_val rhs)
{
    xjr_mp_t *mp = vm->rts.rheap.mp;

    if (XJR_VAL_IS_INTEGER(lhs) && XJR_VAL_IS_INTEGER(rhs))
    {
        int v_lhs = XJR_VAL_AS_INTEGER_UNTAG(lhs);
        int v_rhs = XJR_VAL_AS_INTEGER_UNTAG(rhs);
        *dst = XJR_VAL_MAKE_BOOLEAN(v_lhs == v_rhs);
    }
    else if (XJR_VAL_IS_BOOLEAN_FALSE(lhs) && XJR_VAL_IS_BOOLEAN_FALSE(rhs))
    { *dst = XJR_VAL_MAKE_BOOLEAN_TRUE(); }
    else if (XJR_VAL_IS_BOOLEAN_TRUE(lhs) && XJR_VAL_IS_BOOLEAN_TRUE(rhs))
    { *dst = XJR_VAL_MAKE_BOOLEAN_TRUE(); }
    else if (XJR_VAL_IS_BOOLEAN_TRUE(lhs) && XJR_VAL_IS_BOOLEAN_TRUE(rhs))
    { *dst = XJR_VAL_MAKE_BOOLEAN_TRUE(); }
    else if ((XJR_VAL_IS_NULL(lhs) || XJR_VAL_IS_UNDEFINED(lhs)) && \
            (XJR_VAL_IS_NULL(rhs) || XJR_VAL_IS_UNDEFINED(rhs)))
    { *dst = XJR_VAL_MAKE_BOOLEAN_TRUE(); }
    else if (XJR_VAL_IS_STRING(lhs) && XJR_VAL_IS_STRING(rhs))
    {
        *dst = xjr_val_as_string_e2(mp, lhs, rhs) == xjr_true ? \
               XJR_VAL_MAKE_BOOLEAN_TRUE() : XJR_VAL_MAKE_BOOLEAN_FALSE();
    }
    else if (XJR_VAL_IS_FUNCTION(lhs) && XJR_VAL_IS_FUNCTION(rhs))
    {
        *dst = xjr_val_as_function_e2(mp, lhs, rhs) == xjr_true ? \
               XJR_VAL_MAKE_BOOLEAN_TRUE() : XJR_VAL_MAKE_BOOLEAN_FALSE();
    }
    else if (XJR_VAL_IS_OBJECT(lhs) && XJR_VAL_IS_OBJECT(rhs))
    {
        *dst = lhs == rhs ? \
               XJR_VAL_MAKE_BOOLEAN_TRUE() : XJR_VAL_MAKE_BOOLEAN_FALSE();
    }
    else
    {
        *dst = XJR_VAL_MAKE_BOOLEAN_FALSE();
    }

    return 0;
}

static int xjr_vm_op_binary_ne2(xjr_vm *vm, \
        xjr_val *dst, xjr_val lhs, xjr_val rhs)
{
    int ret = xjr_vm_op_binary_e2(vm, dst, lhs, rhs);

    /* Reverse the result */
    if (XJR_VAL_IS_BOOLEAN_FALSE(*dst))
    { *dst = XJR_VAL_MAKE_BOOLEAN_TRUE(); }
    else
    { *dst = XJR_VAL_MAKE_BOOLEAN_FALSE(); }

    return ret;
}

static int xjr_vm_op_binary_e3(xjr_vm *vm, \
        xjr_val *dst, xjr_val lhs, xjr_val rhs)
{
    xjr_mp_t *mp = vm->rts.rheap.mp;

    if (XJR_VAL_IS_INTEGER(lhs) && XJR_VAL_IS_INTEGER(rhs))
    {
        int v_lhs = XJR_VAL_AS_INTEGER_UNTAG(lhs);
        int v_rhs = XJR_VAL_AS_INTEGER_UNTAG(rhs);
        *dst = XJR_VAL_MAKE_BOOLEAN(v_lhs == v_rhs);
    }
    else if (XJR_VAL_IS_BOOLEAN_FALSE(lhs) && XJR_VAL_IS_BOOLEAN_FALSE(rhs))
    { *dst = XJR_VAL_MAKE_BOOLEAN_TRUE(); }
    else if (XJR_VAL_IS_BOOLEAN_TRUE(lhs) && XJR_VAL_IS_BOOLEAN_TRUE(rhs))
    { *dst = XJR_VAL_MAKE_BOOLEAN_TRUE(); }
    else if (XJR_VAL_IS_BOOLEAN_TRUE(lhs) && XJR_VAL_IS_BOOLEAN_TRUE(rhs))
    { *dst = XJR_VAL_MAKE_BOOLEAN_TRUE(); }
    else if ((XJR_VAL_IS_NULL(lhs) || XJR_VAL_IS_UNDEFINED(lhs)) && \
            (XJR_VAL_IS_NULL(rhs) || XJR_VAL_IS_UNDEFINED(rhs)))
    { *dst = XJR_VAL_MAKE_BOOLEAN_TRUE(); }
    else if (XJR_VAL_IS_STRING(lhs) && XJR_VAL_IS_STRING(rhs))
    {
        *dst = xjr_val_as_string_e3(mp, lhs, rhs) == xjr_true ? \
               XJR_VAL_MAKE_BOOLEAN_TRUE() : XJR_VAL_MAKE_BOOLEAN_FALSE();
    }
    else if (XJR_VAL_IS_FUNCTION(lhs) && XJR_VAL_IS_FUNCTION(rhs))
    {
        *dst = xjr_val_as_function_e3(mp, lhs, rhs) == xjr_true ? \
               XJR_VAL_MAKE_BOOLEAN_TRUE() : XJR_VAL_MAKE_BOOLEAN_FALSE();
    }
    else if (XJR_VAL_IS_OBJECT(lhs) && XJR_VAL_IS_OBJECT(rhs))
    {
        *dst = lhs == rhs ? \
               XJR_VAL_MAKE_BOOLEAN_TRUE() : XJR_VAL_MAKE_BOOLEAN_FALSE();
    }
    else
    {
        *dst = XJR_VAL_MAKE_BOOLEAN_FALSE();
    }

    return 0;
}

static int xjr_vm_op_binary_ne3(xjr_vm *vm, \
        xjr_val *dst, xjr_val lhs, xjr_val rhs)
{
    int ret = xjr_vm_op_binary_e3(vm, dst, lhs, rhs);

    /* Reverse the result */
    if (XJR_VAL_IS_BOOLEAN_FALSE(*dst))
    { *dst = XJR_VAL_MAKE_BOOLEAN_TRUE(); }
    else
    { *dst = XJR_VAL_MAKE_BOOLEAN_FALSE(); }

    return ret;
}

static int xjr_vm_op_binary_l(xjr_vm *vm, \
        xjr_val *dst, xjr_val lhs, xjr_val rhs)
{
    if (XJR_VAL_IS_INTEGER(lhs) && XJR_VAL_IS_INTEGER(rhs))
    {
        int v_lhs = XJR_VAL_AS_INTEGER_UNTAG(lhs);
        int v_rhs = XJR_VAL_AS_INTEGER_UNTAG(rhs);
        *dst = XJR_VAL_MAKE_BOOLEAN(v_lhs < v_rhs);
    }
    else
    { XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE); return -1; }
    return 0;
}

static int xjr_vm_op_binary_le(xjr_vm *vm, \
        xjr_val *dst, xjr_val lhs, xjr_val rhs)
{
    if (XJR_VAL_IS_INTEGER(lhs) && XJR_VAL_IS_INTEGER(rhs))
    {
        int v_lhs = XJR_VAL_AS_INTEGER_UNTAG(lhs);
        int v_rhs = XJR_VAL_AS_INTEGER_UNTAG(rhs);
        *dst = XJR_VAL_MAKE_BOOLEAN(v_lhs <= v_rhs);
    }
    else
    { XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE); return -1; }
    return 0;
}

static int xjr_vm_op_binary_g(xjr_vm *vm, \
        xjr_val *dst, xjr_val lhs, xjr_val rhs)
{
    if (XJR_VAL_IS_INTEGER(lhs) && XJR_VAL_IS_INTEGER(rhs))
    {
        int v_lhs = XJR_VAL_AS_INTEGER_UNTAG(lhs);
        int v_rhs = XJR_VAL_AS_INTEGER_UNTAG(rhs);
        *dst = XJR_VAL_MAKE_BOOLEAN(v_lhs > v_rhs);
    }
    else
    { XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE); return -1; }
    return 0;
}

static int xjr_vm_op_binary_ge(xjr_vm *vm, \
        xjr_val *dst, xjr_val lhs, xjr_val rhs)
{
    if (XJR_VAL_IS_INTEGER(lhs) && XJR_VAL_IS_INTEGER(rhs))
    {
        int v_lhs = XJR_VAL_AS_INTEGER_UNTAG(lhs);
        int v_rhs = XJR_VAL_AS_INTEGER_UNTAG(rhs);
        *dst = XJR_VAL_MAKE_BOOLEAN(v_lhs >= v_rhs);
    }
    else
    { XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE); return -1; }
    return 0;
}

static int xjr_vm_op_binary_and(xjr_vm *vm, \
        xjr_val *dst, xjr_val lhs, xjr_val rhs)
{
    *dst = xjr_vm_cond_eq2_as_true(vm, lhs) == xjr_false ? lhs : rhs;
    return 0;
}

static int xjr_vm_op_binary_or(xjr_vm *vm, \
        xjr_val *dst, xjr_val lhs, xjr_val rhs)
{
    *dst = xjr_vm_cond_eq2_as_true(vm, lhs) ? lhs : rhs;
    return 0;
}

static int xjr_vm_op_unary_add(xjr_vm *vm, \
        xjr_val *dst, xjr_val src)
{
    xjr_mp_t *mp = vm->rts.rheap.mp;
    if (XJR_VAL_IS_INTEGER(src))
    {
        *dst = XJR_VAL_MAKE_INTEGER(XJR_VAL_AS_INTEGER_UNTAG(src));
    }
    else if (XJR_VAL_IS_FLOAT(src))
    {
        xjr_f64 v_src_f64 = xjr_val_extract_f64(mp, src);
        *dst = xjr_val_make_f64(mp, v_src_f64);
    }
    else
    {
        /* Type error */
        XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE);
        return -1;
    }

    return 0;
}

static int xjr_vm_op_unary_sub(xjr_vm *vm, \
        xjr_val *dst, xjr_val src)
{
    xjr_mp_t *mp = vm->rts.rheap.mp;
    if (XJR_VAL_IS_INTEGER(src))
    {
        int v_src = XJR_VAL_AS_INTEGER_UNTAG(src);
        int v_dst = -v_src;
        *dst = XJR_VAL_MAKE_INTEGER(v_dst);
    }
    else if (XJR_VAL_IS_FLOAT(src))
    {
        xjr_f64 v_src_f64 = xjr_val_extract_f64(mp, src);
        *dst = xjr_val_make_f64(mp, -v_src_f64);
    }
    else
    {
        /* Type error */
        XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE);
        return -1;
    }

    return 0;
}

static int xjr_vm_op_unary_bnot(xjr_vm *vm, \
        xjr_val *dst, xjr_val src)
{
    if (XJR_VAL_IS_INTEGER(src))
    {
        int v_src = XJR_VAL_AS_INTEGER_UNTAG(src);
        int v_dst = ~v_src;
        *dst = XJR_VAL_MAKE_INTEGER(v_dst);
    }
    else
    {
        /* Type error */
        XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE);
        return -1;
    }

    return 0;
}

static int xjr_vm_op_unary_not(xjr_vm *vm, \
        xjr_val *dst, xjr_val src)
{
    if (XJR_VAL_IS_BOOLEAN_FALSE(src))
    {
        *dst = XJR_VAL_MAKE_BOOLEAN_TRUE();
    }
    else if (XJR_VAL_IS_BOOLEAN_TRUE(src))
    {
        *dst = XJR_VAL_MAKE_BOOLEAN_FALSE();
    }
    else if (XJR_VAL_IS_INTEGER(src))
    {
        *dst = XJR_VAL_AS_INTEGER_UNTAG(src) == 0 ? \
               XJR_VAL_MAKE_BOOLEAN_TRUE() : 
               XJR_VAL_MAKE_BOOLEAN_FALSE();
    }
    else
    {
        /* Type error */
        XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE);
        return -1;
    }

    return 0;
}

static int xjr_vm_op_inspect_cb( \
        void *data, char *s, xjr_size_t len)
{
    xjr_vm *vm = data;
    vm->inspect.cb_write(s, len);
    return 0;
}

static void xjr_vm_op_inspect(xjr_vm *vm, xjr_val v)
{
    xjr_vm_inspect( \
            vm->rts.rheap.mp, \
            vm, xjr_vm_op_inspect_cb, \
            v, \
            XJR_VM_INSPECT_MODE_JSON_PRINT);
    vm->inspect.cb_write("\n", 1);
}

static void xjr_vm_op_dynlib(xjr_vm *vm, xjr_val exports, xjr_val v_module_name)
{
    char *module_name = xjr_val_as_string_body(vm->rts.rheap.mp, v_module_name);
    xjr_size_t module_name_len = xjr_val_as_string_length(vm->rts.rheap.mp, v_module_name);
    xjr_size_t count = vm->libs.count;
    xjr_vm_lib *lib = vm->libs.libs;
    while (count-- != 0)
    {
        if ((xjr_strlen(lib->name) == module_name_len) && \
                (xjr_strncmp(lib->name, module_name, module_name_len) == 0))
        {
            xjr_lib_ctx ctx;
            ctx.vm = vm;
            ctx.env = XJR_VM_GET_ENV(vm);
            ctx.exports = exports;
            ctx.mp = vm->rts.rheap.mp;
            lib->cb(&ctx);
            break;
        }

        lib++;
    }
}

static int xjr_vm_declvar(xjr_vm *vm, xjr_offset_t offset)
{
    int ret = 0;
    char *name; xjr_size_t len;

    if (xjr_vm_read_symbol(vm, &name, &len, offset) != 0)
    { return -1; }

    /* Body */
    {
        xjr_urid_t env = XJR_VM_GET_ENV(vm);
        if (env == XJR_URID_INVALID)
        { XJR_ERR_UPDATE(&vm->err, XJR_ERR_INTERNAL); return -1; }

        /* Save 'undefined' value */
        if (xjr_vm_env_set_var(vm->rts.rheap.mp, env, \
                    name, len, XJR_VAL_MAKE_UNDEFINED()) != 0)
        { goto fail; }
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int xjr_vm_op_declvar(xjr_vm *vm, xjr_offset_t offset)
{
    return xjr_vm_declvar(vm, offset);
}

static int xjr_vm_op_load(xjr_vm *vm, xjr_val *value_out, xjr_offset_t offset)
{
    char *name; xjr_size_t len;
    xjr_urid_t env = XJR_VM_GET_ENV(vm);;

    if (env == XJR_URID_INVALID)
    { XJR_ERR_UPDATE(&vm->err, XJR_ERR_INTERNAL); return -1; }

    if (xjr_vm_read_symbol(vm, &name, &len, offset) != 0)
    { return -1; }

    do
    {
        xjr_vm_env *e = xjr_mp_get_ptr(vm->rts.rheap.mp, env);
        xjr_urid_t var = e->vars.first;
        while (var != XJR_URID_INVALID)
        {
            xjr_vm_var *v = xjr_mp_get_ptr(vm->rts.rheap.mp, var);
            if (v == xjr_nullptr)
            {
                XJR_ERR_UPDATE(&vm->err, XJR_ERR_MEM_CORRUPTED);
                vm->err.u.as_mem_corrupted.urid = (int)var;
                return -1;
            }
            if ((v->name.len == len) && \
                    (xjr_strncmp(v->name.body, name, len) == 0))
            { *value_out = v->value; return 0; }

            /* Next variable */
            var = v->next;
        }

        /* Outter environment */
        env = e->parent;
    } while (env != XJR_URID_INVALID);

    /* Not found
     * TODO: store on the global scope */
    /* do_something(); */
    XJR_ERR_UPDATE(&vm->err, XJR_ERR_NOT_DEFINED);
    vm->err.u.as_not_defined.name = name;
    vm->err.u.as_not_defined.len = (int)len;
    return -1;
}

static int xjr_vm_op_store(xjr_vm *vm, xjr_val value, xjr_offset_t offset)
{
    char *name; xjr_size_t len;
    xjr_urid_t env = XJR_VM_GET_ENV(vm);;

    if (env == XJR_URID_INVALID)
    { XJR_ERR_UPDATE(&vm->err, XJR_ERR_INTERNAL); return -1; }

    if (xjr_vm_read_symbol(vm, &name, &len, offset) != 0)
    { return -1; }

    do
    {
        xjr_vm_env *e = xjr_mp_get_ptr(vm->rts.rheap.mp, env);
        xjr_urid_t var = e->vars.first;
        while (var != XJR_URID_INVALID)
        {
            xjr_vm_var *v = xjr_mp_get_ptr(vm->rts.rheap.mp, var);
            if ((v->name.len == len) && \
                    (xjr_strncmp(v->name.body, name, len) == 0))
            {
                v->value = value;
                return 0;
            }

            /* Next variable */
            var = v->next;
        }

        /* Outter environment */
        env = e->parent;
    } while (env != XJR_URID_INVALID);

    /* Not found
     * TODO: current throw the error, 
     * to store on the global scope for following the 'non-strict' rule */
    XJR_ERR_UPDATE(&vm->err, XJR_ERR_NOT_DEFINED);
    vm->err.u.as_not_defined.name = name;
    vm->err.u.as_not_defined.len = (int)len;
    return -1;
}

static int xjr_vm_objget(xjr_vm *vm, xjr_reg r_dst, xjr_val obj, xjr_val key)
{
    int ret = 0;
    xjr_mp_t *mp = vm->rts.rheap.mp;
    xjr_val cur = obj;

    if (XJR_VAL_IS_ARRAY(cur))
    {
        if (XJR_VAL_IS_INTEGER(key))
        {
            int key_idx = XJR_VAL_AS_INTEGER_UNTAG(key);
            xjr_val v_dst = xjr_val_as_array_get_by_idx(mp, cur, key_idx);
            xjr_val_properties_install(vm, v_dst);
            XJR_VM_SET_VREGV(vm, r_dst, v_dst);
            return 0;
        }
    }
    else if (XJR_VAL_IS_STRING(cur))
    {
        if (XJR_VAL_IS_INTEGER(key))
        {
            int key_idx = XJR_VAL_AS_INTEGER_UNTAG(key);
            xjr_val v_dst = xjr_val_as_string_get_by_idx(mp, cur, key_idx);
            xjr_val_properties_install(vm, v_dst);
            XJR_VM_SET_VREGV(vm, r_dst, v_dst);
            return 0;
        }
    }

    for (;;)
    {
        if (!XJR_VAL_HAS_PROPERTIES(cur))
        {
            XJR_ERR_UPDATE(&vm->err, XJR_ERR_NOT_DEFINED);
            if (XJR_VAL_IS_STRING(key))
            {
                vm->err.u.as_not_defined.name = xjr_val_as_string_body(mp, key);
                vm->err.u.as_not_defined.len = (int)xjr_val_as_string_length(mp, key);
            }
            else
            {
                vm->err.u.as_not_defined.name = xjr_nullptr;
                vm->err.u.as_not_defined.len = 0;
            }
            goto fail;
        }
        else
        {
            xjr_val_properties *props = XJR_VAL_PROPERTY_GET(vm->rts.rheap.mp, cur);
            xjr_val_property_type prop_type;
            xjr_val val;
            if (xjr_val_properties_get(vm->rts.rheap.mp, props, \
                        &val, &prop_type, \
                        key) == 0)
            {
                switch (prop_type)
                {
                    case XJR_VAL_PROPERTY_TYPE_NORMAL:
                        XJR_VM_SET_VREGV(vm, r_dst, val);
                        break;
                    case XJR_VAL_PROPERTY_TYPE_GETTER:
                        /* Invoke getter (in 'val') */
                        if (xjr_vm_op_call(vm, r_dst, val, obj, 0, xjr_nullptr) != 0)
                        { goto fail; }
                        break;
                    case XJR_VAL_PROPERTY_TYPE_SETTER:
                        XJR_VM_SET_VREGV(vm, r_dst, XJR_VAL_MAKE_UNDEFINED());
                        break;
                }
                /* Already got what we want */
                break;
            }
            /* Lookup by the path of prototype chain */
            cur = xjr_val_properties_get_bltin(props, XJR_VAL_PROPERTIES_BLTIN_PROTO);
        }
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int xjr_vm_objset(xjr_vm *vm, xjr_val r_dst, xjr_val v_obj, xjr_val v_key, xjr_val v_src)
{
    int ret = 0;
    xjr_mp_t *mp = vm->rts.rheap.mp;

    if (XJR_VAL_IS_ARRAY(v_obj))
    {
        if (XJR_VAL_IS_INTEGER(v_key))
        {
            int key_idx = XJR_VAL_AS_INTEGER_UNTAG(v_key);
            xjr_val v_dst = xjr_val_as_array_set_by_idx(mp, v_obj, key_idx, v_src);
            XJR_VM_SET_VREGV(vm, r_dst, v_dst);
            return 0;
        }
    }

    if (XJR_VAL_HAS_PROPERTIES(v_obj))
    {
        xjr_val_properties *props = XJR_VAL_PROPERTY_GET(vm->rts.rheap.mp, v_obj);
        if (xjr_val_properties_set(vm->rts.rheap.mp, props, \
                    XJR_VAL_PROPERTY_TYPE_NORMAL, v_key, v_src) != 0)
        {
            XJR_ERR_UPDATE(&vm->err, XJR_ERR_NOT_DEFINED);
            vm->err.u.as_not_defined.name = xjr_nullptr;
            vm->err.u.as_not_defined.len = 0;
            goto fail;
        }
        XJR_VM_SET_VREGV(vm, r_dst, v_obj);
    }
    else
    {
        XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_TYPE);
        goto fail;
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int xjr_vm_op_arrpush(xjr_vm *vm, xjr_val v_arr, xjr_val v_elem)
{
    xjr_urid_t u_elem = xjr_val_make_array_item(vm->rts.rheap.mp, v_elem);
    if (u_elem == XJR_URID_INVALID)
    { XJR_ERR_UPDATE(&vm->err, XJR_ERR_OOM); return -1; }
    xjr_val_as_array_push(vm->rts.rheap.mp, v_arr, v_elem);
    return 0;
}

/* Start executing */
int xjr_vm_step(xjr_vm *vm, int *to_halt)
{
    int ret = 0;
    xjr_u8 opcode;

    /* Fetch instruction */
    vm->err.rts.pc = (int)vm->rts.pc;
    FETCH_OPCODE(&opcode);
    vm->err.rts.opcode = (int)opcode;

    switch (opcode)
    {
        case XJR_BC_OP_NOP:
            break;

        case XJR_BC_OP_ALLOCREG:
            {
                xjr_u8 nreg;
                FETCH_U8(&nreg);
                xjr_vm_op_allocreg(vm, nreg);
            }
            break;

        case XJR_BC_OP_ARG:
            {
                xjr_u8 idx; xjr_u32 offset;
                xjr_val v_arg;
                FETCH_U32(&offset);
                FETCH_U8(&idx);
                v_arg = XJR_VM_GET_ARGV(vm, idx);
                if (xjr_vm_op_declvar(vm, offset) != 0) { goto fail; }
                if (xjr_vm_op_store(vm, v_arg, offset) != 0) { goto fail; }
            }
            break;

        case XJR_BC_OP_HALT:
            *to_halt = 1;
            break;

        case XJR_BC_OP_DEBUG:
            *to_halt = 1;
            break;

        case XJR_BC_OP_INSPECT:
            {
                xjr_reg r;
                xjr_val v;
                FETCH_U8(&r);
                v = XJR_VM_GET_VREGV(vm, r);
                xjr_vm_op_inspect(vm, v);
            }
            break;

        case XJR_BC_OP_DYNLIB:
            {
                xjr_u8 r_exports, r_module;
                xjr_val v_exports, v_module;
                FETCH_U8(&r_exports);
                FETCH_U8(&r_module);
                v_exports = XJR_VM_GET_VREGV(vm, r_exports);
                v_module = XJR_VM_GET_VREGV(vm, r_module);
                xjr_vm_op_dynlib(vm, v_exports, v_module);
            }
            break;

        case XJR_BC_OP_BR:
            {
                xjr_val offset;
                FETCH_U32(&offset);
                vm->rts.pc = offset;
            }
            break;

        case XJR_BC_OP_BRC:
            {
                xjr_u8 r;
                xjr_val v_cond;
                xjr_val v_offset;
                FETCH_U8(&r);
                v_cond = XJR_VM_GET_VREGV(vm, r);
                FETCH_U32(&v_offset);
                if (xjr_vm_cond_eq2_as_true(vm, v_cond) == xjr_true)
                { vm->rts.pc = v_offset; }
            }
            break;

        case XJR_BC_OP_MERGE:
            {
                xjr_u8 r_dst, r_test, r_csq, r_alt;
                xjr_val v_dst, v_test;
                FETCH_U8(&r_dst);
                FETCH_U8(&r_test);
                FETCH_U8(&r_csq);
                FETCH_U8(&r_alt);
                v_test = XJR_VM_GET_VREGV(vm, r_test);
                if (xjr_vm_cond_eq2_as_true(vm, v_test) == xjr_true)
                { v_dst = XJR_VM_GET_VREGV(vm, r_csq); }
                else
                { v_dst = XJR_VM_GET_VREGV(vm, r_alt); }
                XJR_VM_SET_VREGV(vm, r_dst, v_dst);
            }
            break;

        case XJR_BC_OP_CALL:
        case XJR_BC_OP_CALLT:
            {
                xjr_u8 r_dst, r_callee, r_argc, r_argv[XJR_BC_ARGC_MAX];
                xjr_val v_callee, v_this;
                xjr_u8 i;
                FETCH_U8(&r_dst);
                FETCH_U8(&r_callee);
                if (opcode == XJR_BC_OP_CALL)
                {
                    v_this = XJR_VM_GET_THIS(vm);
                }
                else if (opcode == XJR_BC_OP_CALLT)
                {
                    xjr_u8 r_this;
                    FETCH_U8(&r_this);
                    v_this = XJR_VM_GET_VREGV(vm, r_this);
                }
                else
                { XJR_ERR_UPDATE(&vm->err, XJR_ERR_INTERNAL); goto fail; }
                FETCH_U8(&r_argc);
                v_callee = XJR_VM_GET_VREGV(vm, r_callee);
                if (r_argc > XJR_BC_ARGC_MAX)
                {
                    XJR_ERR_UPDATE(&vm->err, XJR_ERR_ARGC_EXCEED);
                    goto fail;
                }
                for (i = 0; i != r_argc; i++)
                {
                    FETCH_U8(&r_argv[i]);
                }
                if (xjr_vm_op_call(vm, r_dst, v_callee, v_this, r_argc, r_argv) != 0)
                { goto fail; }
            }
            break;

        case XJR_BC_OP_NEW:
            {
                xjr_u8 dst, callee, argc, argv[XJR_BC_ARGC_MAX];
                xjr_val callee_data, _this;
                xjr_u8 i;
                FETCH_U8(&dst);
                FETCH_U8(&callee);
                FETCH_U8(&argc);
                callee_data = XJR_VM_GET_VREGV(vm, callee);
                if (argc > XJR_BC_ARGC_MAX)
                {
                    XJR_ERR_UPDATE(&vm->err, XJR_ERR_ARGC_EXCEED);
                    goto fail;
                }
                for (i = 0; i != argc; i++)
                {
                    FETCH_U8(&argv[i]);
                }
                /* 'this' been pointed to the new should be the new created function */
                /* _this */

                /* Only function and object could be used as a constructor */
                {

                    if (!(XJR_VAL_IS_OBJECT(callee_data) || XJR_VAL_IS_FUNCTION(callee_data)))
                    {
                        XJR_ERR_UPDATE(&vm->err, XJR_ERR_NOT_CONSTRUCTOR);
                        goto fail;
                    }
                    _this = xjr_val_make_object(vm->rts.rheap.mp);
                    {
                        xjr_val_properties *props_this, *props_callee;
                        xjr_val callee_prototype;

                        props_callee = XJR_VAL_PROPERTY_GET(vm->rts.rheap.mp, callee_data);
                        props_this = xjr_val_as_object_property_get(vm->rts.rheap.mp, _this);
                        /* this.__proto__ = callee.prototype */
                        callee_prototype = xjr_val_properties_get_bltin(props_callee, XJR_VAL_PROPERTIES_BLTIN_PROTOTYPE);
                        xjr_val_properties_set_bltin(props_this, XJR_VAL_PROPERTIES_BLTIN_PROTO, callee_prototype);
                    }
                    if (xjr_vm_op_new(vm, dst, callee_data, _this, argc, argv) != 0)
                    { goto fail; }
                }
            }
            break;

        case XJR_BC_OP_THIS:
            {
                xjr_u8 r_dst;
                FETCH_U8(&r_dst);
                XJR_VM_SET_VREGV(vm, r_dst, XJR_VM_GET_THIS(vm));
            }
            break;

        case XJR_BC_OP_RET:
            {
                xjr_u8 r_dst;
                FETCH_U8(&r_dst);
                if (xjr_vm_op_ret(vm, r_dst, to_halt) != 0)
                { goto fail; }
            }
            break;

        case XJR_BC_OP_DECLVAR:
            {
                xjr_u32 offset;
                FETCH_U32(&offset);
                if (xjr_vm_op_declvar(vm, offset) != 0)
                { goto fail; }
            }
            break;

        case XJR_BC_OP_LOAD:
            {
                xjr_u8 r; xjr_u32 offset;
                xjr_val v;
                FETCH_U8(&r);
                FETCH_U32(&offset);
                if (xjr_vm_op_load(vm, &v, offset) != 0) { goto fail; }
                XJR_VM_SET_VREGV(vm, r, v);
            }
            break;

        case XJR_BC_OP_STORE:
            {
                xjr_u8 r; xjr_u32 offset;
                xjr_val v;
                FETCH_U8(&r);
                v= XJR_VM_GET_VREGV(vm, r);
                FETCH_U32(&offset);
                if (xjr_vm_op_store(vm, v, offset) != 0) { goto fail; }
            }
            break;

        case XJR_BC_OP_LOAD_FALSE:
            {
                xjr_reg r;
                FETCH_REG(&r);
                XJR_VM_SET_VREGV(vm, r, XJR_VAL_MAKE_BOOLEAN_FALSE());
            }
            break;

        case XJR_BC_OP_LOAD_TRUE:
            {
                xjr_reg r;
                FETCH_REG(&r);
                XJR_VM_SET_VREGV(vm, r, XJR_VAL_MAKE_BOOLEAN_TRUE());
            }
            break;

        case XJR_BC_OP_LOAD_UNDEFINED:
            {
                xjr_reg r;
                FETCH_REG(&r);
                XJR_VM_SET_VREGV(vm, r, XJR_VAL_MAKE_UNDEFINED());
            }
            break;

        case XJR_BC_OP_LOAD_NULL:
            {
                xjr_reg r;
                FETCH_REG(&r);
                XJR_VM_SET_VREGV(vm, r, XJR_VAL_MAKE_NULL());
            }
            break;

        case XJR_BC_OP_LOAD_NUMBER_S8:
            {
                xjr_reg r; xjr_s8 v;
                FETCH_REG(&r); FETCH_S8(&v);
                XJR_VM_SET_VREGV(vm, r, XJR_VAL_MAKE_INTEGER((int)v));
            }
            break;

        case XJR_BC_OP_LOAD_NUMBER_U8:
            {
                xjr_reg r; xjr_u8 v; int vi;
                FETCH_REG(&r); FETCH_U8(&v);
                vi = (int)v;
                XJR_VM_SET_VREGV(vm, r, XJR_VAL_MAKE_INTEGER(vi));
            }
            break;

        case XJR_BC_OP_LOAD_NUMBER_S16:
            {
                xjr_reg r; xjr_s16 v;
                FETCH_REG(&r); FETCH_S16(&v);
                XJR_VM_SET_VREGV(vm, r, XJR_VAL_MAKE_INTEGER(v));
            }
            break;

        case XJR_BC_OP_LOAD_NUMBER_U16:
            {
                xjr_reg r; xjr_u16 v; int vi;
                FETCH_REG(&r); FETCH_U16(&v);
                vi = (int)v;
                XJR_VM_SET_VREGV(vm, r, XJR_VAL_MAKE_INTEGER(vi));
            }
            break;

        case XJR_BC_OP_LOAD_NUMBER_S32:
            {
                xjr_reg r; xjr_s32 v;
                FETCH_REG(&r); FETCH_S32(&v);
                XJR_VM_SET_VREGV(vm, r, XJR_VAL_MAKE_INTEGER(v));
            }
            break;

        case XJR_BC_OP_LOAD_NUMBER_U32:
            {
                xjr_reg r; xjr_u32 v; int vi;
                FETCH_REG(&r); FETCH_U32(&v);
                vi = (int)v;
                XJR_VM_SET_VREGV(vm, r, XJR_VAL_MAKE_INTEGER(vi));
            }
            break;

        case XJR_BC_OP_LOAD_NUMBER_F64:
            {
                xjr_reg r; xjr_f64 v;
                FETCH_REG(&r); FETCH_F64(&v);
                XJR_VM_SET_VREGV(vm, r, xjr_val_make_f64(vm->rts.rheap.mp, v));
            }
            break;

        case XJR_BC_OP_LOAD_STRING:
            {
                xjr_val v; xjr_reg r; xjr_u32 offset;
                FETCH_REG(&r); FETCH_U32(&offset);
                {
                    v = xjr_val_make_string_from_datasec( \
                            &vm->xf, vm->rts.rheap.mp, offset);
                    if (XJR_VAL_IS_UNDEFINED(v))
                    { XJR_ERR_UPDATE(&vm->err, XJR_ERR_INTERNAL); goto fail; }
                    xjr_val_properties_install(vm, v);
                }
                XJR_VM_SET_VREGV(vm, r, v);
            }
            break;

        case XJR_BC_OP_LOAD_STRING_EMPTY:
            return -1;

        case XJR_BC_OP_LOAD_FUNCTION:
            {
                xjr_urid_t env_parent = XJR_VM_GET_ENV(vm);
                xjr_u8 r; xjr_u32 offset;
                xjr_val v;
                FETCH_U8(&r);
                FETCH_U32(&offset);
                {
                    if ((v = xjr_val_make_function(vm->rts.rheap.mp, env_parent, offset)) == XJR_URID_INVALID)
                    { goto fail; }
                    /* Function is a little special that it could not just install the '__proto__'
                     * xjr_val_properties_install(vm, v);
                     */
                    {
                        xjr_val_properties *props = xjr_val_as_function_property_get(vm->rts.rheap.mp, v);
                        xjr_val_properties_set_bltin(props, XJR_VAL_PROPERTIES_BLTIN_PROTO, vm->fundamental.global_function_prototype);
                        xjr_val_properties_set_bltin(props, XJR_VAL_PROPERTIES_BLTIN_PROTOTYPE, xjr_val_make_object(vm->rts.rheap.mp));
                    }
                }
                XJR_VM_SET_VREGV(vm, r, v);
            }
            break;

        case XJR_BC_OP_LOAD_OBJECT:
            {
                xjr_u8 r;
                xjr_val v;
                FETCH_U8(&r);
                {
                    /* Primitive make object */
                    if ((v = xjr_val_make_object(vm->rts.rheap.mp)) == XJR_URID_INVALID)
                    { goto fail; }
                    xjr_val_properties_install(vm, v);
                }
                XJR_VM_SET_VREGV(vm, r, v);
            }
            break;

        case XJR_BC_OP_LOAD_ARRAY:
            {
                xjr_u8 r;
                xjr_val v;
                FETCH_U8(&r);
                {
                    v = xjr_val_make_array(vm->rts.rheap.mp);
                    if (v == XJR_URID_INVALID) { goto fail; }
                    xjr_val_properties_install(vm, v);
                }
                XJR_VM_SET_VREGV(vm, r, v);
            }
            break;

        case XJR_BC_OP_BINADD:
        case XJR_BC_OP_BINSUB:
        case XJR_BC_OP_BINMUL:
        case XJR_BC_OP_BINDIV:
        case XJR_BC_OP_BINMOD:
        case XJR_BC_OP_BINE2:
        case XJR_BC_OP_BINNE2:
        case XJR_BC_OP_BINE3:
        case XJR_BC_OP_BINNE3:
        case XJR_BC_OP_BINL:
        case XJR_BC_OP_BINLE:
        case XJR_BC_OP_BING:
        case XJR_BC_OP_BINGE:
        case XJR_BC_OP_BINAND:
        case XJR_BC_OP_BINOR:
            {
                xjr_reg r_dst, r_lhs, r_rhs;
                xjr_val v_lhs, v_rhs, v_dst;
                FETCH_REG(&r_dst);
                FETCH_REG(&r_lhs);
                FETCH_REG(&r_rhs);
                v_lhs = XJR_VM_GET_VREGV(vm, r_lhs);
                v_rhs = XJR_VM_GET_VREGV(vm, r_rhs);
                if (opcode == XJR_BC_OP_BINADD)
                {
                    if (xjr_vm_op_binary_add(vm, &v_dst, v_lhs, v_rhs) != 0) { goto fail; }
                }
                else if (opcode == XJR_BC_OP_BINSUB)
                {
                    if (xjr_vm_op_binary_sub(vm, &v_dst, v_lhs, v_rhs) != 0) { goto fail; }
                }
                else if (opcode == XJR_BC_OP_BINMUL)
                {
                    if (xjr_vm_op_binary_mul(vm, &v_dst, v_lhs, v_rhs) != 0) { goto fail; }
                }
                else if (opcode == XJR_BC_OP_BINDIV)
                {
                    if (xjr_vm_op_binary_div(vm, &v_dst, v_lhs, v_rhs) != 0) { goto fail; }
                }
                else if (opcode == XJR_BC_OP_BINMOD)
                {
                    if (xjr_vm_op_binary_mod(vm, &v_dst, v_lhs, v_rhs) != 0) { goto fail; }
                }
                else if (opcode == XJR_BC_OP_BINE2)
                {
                    if (xjr_vm_op_binary_e2(vm, &v_dst, v_lhs, v_rhs) != 0) { goto fail; }
                }
                else if (opcode == XJR_BC_OP_BINNE2)
                {
                    if (xjr_vm_op_binary_ne2(vm, &v_dst, v_lhs, v_rhs) != 0) { goto fail; }
                }
                else if (opcode == XJR_BC_OP_BINE3)
                {
                    if (xjr_vm_op_binary_e3(vm, &v_dst, v_lhs, v_rhs) != 0) { goto fail; }
                }
                else if (opcode == XJR_BC_OP_BINNE3)
                {
                    if (xjr_vm_op_binary_ne3(vm, &v_dst, v_lhs, v_rhs) != 0) { goto fail; }
                }
                else if (opcode == XJR_BC_OP_BINL)
                {
                    if (xjr_vm_op_binary_l(vm, &v_dst, v_lhs, v_rhs) != 0) { goto fail; }
                }
                else if (opcode == XJR_BC_OP_BINLE)
                {
                    if (xjr_vm_op_binary_le(vm, &v_dst, v_lhs, v_rhs) != 0) { goto fail; }
                }
                else if (opcode == XJR_BC_OP_BING)
                {
                    if (xjr_vm_op_binary_g(vm, &v_dst, v_lhs, v_rhs) != 0) { goto fail; }
                }
                else if (opcode == XJR_BC_OP_BINGE)
                {
                    if (xjr_vm_op_binary_ge(vm, &v_dst, v_lhs, v_rhs) != 0) { goto fail; }
                }
                else if (opcode == XJR_BC_OP_BINAND)
                {
                    if (xjr_vm_op_binary_and(vm, &v_dst, v_lhs, v_rhs) != 0) { goto fail; }
                }
                else /* if (opcode == XJR_BC_OP_BINOR) */
                {
                    if (xjr_vm_op_binary_or(vm, &v_dst, v_lhs, v_rhs) != 0) { goto fail; }
                }

                XJR_VM_SET_VREGV(vm, r_dst, v_dst);
            }
            break;

        case XJR_BC_OP_UNADD:
        case XJR_BC_OP_UNSUB:
        case XJR_BC_OP_UNNOT:
        case XJR_BC_OP_UNBNOT:
            {
                xjr_reg r_dst, r_src;
                xjr_val v_src, dst;
                FETCH_REG(&r_dst);
                FETCH_REG(&r_src);
                v_src = XJR_VM_GET_VREGV(vm, r_src);
                if (opcode == XJR_BC_OP_UNADD)
                { if (xjr_vm_op_unary_add(vm, &dst, v_src) != 0) { goto fail; } }
                else if (opcode == XJR_BC_OP_UNSUB)
                { if (xjr_vm_op_unary_sub(vm, &dst, v_src) != 0) { goto fail; } }
                else if (opcode == XJR_BC_OP_UNNOT)
                { if (xjr_vm_op_unary_not(vm, &dst, v_src) != 0) { goto fail; } }
                else /* if (opcode == XJR_BC_OP_UNBNOT) */
                { if (xjr_vm_op_unary_bnot(vm, &dst, v_src) != 0) { goto fail; } }
                XJR_VM_SET_VREGV(vm, r_dst, dst);
            }
            break;

        case XJR_BC_OP_OBJGET:
            {
                xjr_reg r_dst, r_obj, r_member;
                xjr_val v_obj, v_member;
                FETCH_REG(&r_dst);
                FETCH_REG(&r_obj);
                FETCH_REG(&r_member);
                v_obj = XJR_VM_GET_VREGV(vm, r_obj);
                v_member = XJR_VM_GET_VREGV(vm, r_member);
                if (xjr_vm_objget(vm, r_dst, v_obj, v_member) != 0)
                { goto fail; }
            }
            break;

        case XJR_BC_OP_OBJSET:
            {
                xjr_reg r_dst, r_obj, r_member, r_src;
                xjr_val v_obj, v_member, v_src;
                FETCH_REG(&r_dst);
                FETCH_REG(&r_obj);
                FETCH_REG(&r_member);
                FETCH_REG(&r_src);
                v_obj = XJR_VM_GET_VREGV(vm, r_obj);
                v_member = XJR_VM_GET_VREGV(vm, r_member);
                v_src = XJR_VM_GET_VREGV(vm, r_src);
                if (xjr_vm_objset(vm, r_dst, v_obj, v_member, v_src) != 0) { goto fail; }
            }
            break;

        case XJR_BC_OP_ARRPUSH:
            {
                xjr_reg r_arr, r_elem;
                xjr_val v_arr, v_elem;
                FETCH_REG(&r_arr);
                FETCH_REG(&r_elem);
                v_arr = XJR_VM_GET_VREGV(vm, r_arr);
                v_elem = XJR_VM_GET_VREGV(vm, r_elem);
                if (xjr_vm_op_arrpush(vm, v_arr, v_elem) != 0) { goto fail; }
            }
            break;

        default:
            XJR_ERR_UPDATE(&vm->err, XJR_ERR_INVALID_OP);
            goto fail;
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

/* Register external objects */
int xjr_vm_register_external_val(xjr_vm *vm, xjr_val v)
{
    {
        xjr_size_t i;
        for (i = 0; i != vm->misc.external_vals.size; i++)
        {
            if (vm->misc.external_vals.body[i] == v) return 0;
        }
    }

    if (vm->misc.external_vals.size == XJR_VM_EXTERNAL_VAL_CAPACITY) return -1;

    vm->misc.external_vals.body[vm->misc.external_vals.size] = v;
    vm->misc.external_vals.size++;

    return 0;
}

/* Unregister external objects */
int xjr_vm_unregister_external_val(xjr_vm *vm, xjr_val v)
{
    xjr_size_t i;

    for (i = 0; i != vm->misc.external_vals.size; i++)
    {
        if (vm->misc.external_vals.body[i] == v)
        {
            while (i < vm->misc.external_vals.size - 1)
            {
                vm->misc.external_vals.body[i] = vm->misc.external_vals.body[i + 1];
                i++;
            }
            vm->misc.external_vals.size--;
            break;
        }
    }

    return -1;
}

