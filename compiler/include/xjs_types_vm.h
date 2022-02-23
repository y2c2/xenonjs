/* XenonJS : Types : VM
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_TYPES_VM_H
#define XJS_TYPES_VM_H

#include "xjs_dt.h"

#define XJS_VM_REG_COUNT (16)
#define XJS_VM_REG_PC (15)

struct xjs_opaque_vm_options;
typedef struct xjs_opaque_vm_options xjs_vm_options ;
typedef struct xjs_opaque_vm_options *xjs_vm_options_ref;

struct xjs_opaque_vm_reg;
typedef struct xjs_opaque_vm_reg xjs_vm_reg;
typedef struct xjs_opaque_vm_reg *xjs_vm_reg_ref;

struct xjs_opaque_vm;
typedef struct xjs_opaque_vm xjs_vm;
typedef struct xjs_opaque_vm *xjs_vm_ref;

struct xjs_opaque_vm_options
{
    int dummy;
};

/* Register */
struct xjs_opaque_vm_reg
{
    union
    {
        double as_double;
        xjs_u32 as_u32;
        xjs_s32 as_s32;
        xjs_bool as_bool;
    } u;
};

/* Virtual Machine */
struct xjs_opaque_vm
{
    xjs_vm_reg regs[XJS_VM_REG_COUNT];
};

#endif

