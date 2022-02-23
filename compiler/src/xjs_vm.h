/* XenonJS : VM
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_VM_H
#define XJS_VM_H

#include "xjs_types.h"

void xjs_vm_options_init(xjs_vm_options_ref opts);

xjs_vm_ref xjs_vm_new(xjs_vm_options_ref opts);

void xjs_vm_delete(xjs_vm_ref vm);

xjs_val_ref xjs_vm_eval( \
        xjs_vm_ref vm, \
        char *bytecode, xjs_size_t bytecode_len);

void xjs_vm_println( \
        xjs_val_ref val);

#endif

