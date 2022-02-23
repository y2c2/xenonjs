/* XenonJS : VM
 * Copyright(c) 2017 y2c2 */

#include "xjs_types.h"
#include "xjs_vm.h"

void xjs_vm_options_init(xjs_vm_options_ref opts)
{
    (void)opts;
}

static void xjs_vm_ctor(void *data)
{
    xjs_vm_ref r = data;
    (void)r;
}

static void xjs_vm_dtor(void *data)
{
    xjs_vm_ref r = data;
    (void)r;
}

xjs_vm_ref xjs_vm_new(xjs_vm_options_ref opts)
{
    xjs_vm_ref vm = ec_newcd(xjs_vm, \
            xjs_vm_ctor, xjs_vm_dtor);

    (void)opts;

    /* Registers */
    {
        xjs_size_t i;
        for(i = 0; i != XJS_VM_REG_COUNT; i++)
        { vm->regs[i].u.as_u32 = 0; }
    }

    return vm;
}

void xjs_vm_delete(xjs_vm_ref vm)
{
    ec_delete(vm);
}

xjs_val_ref xjs_vm_eval( \
        xjs_vm_ref vm, \
        char *bytecode, xjs_size_t bytecode_len)
{
    (void)vm;
    (void)bytecode;
    (void)bytecode_len;
    return NULL;
}

void xjs_vm_println( \
        xjs_val_ref val)
{
    (void)val;
}

