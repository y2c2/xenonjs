/* XenonJS : Runtime Time System : Native Function
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_NATIVEFN_H
#define XJR_NATIVEFN_H

#include "xjr_dt.h"
#include "xjr_extn.h"
#include "xjr_mp.h"
#include "xjr_vm_dt.h"

/* Native Function */

struct xjr_opaque_native_fn_args
{
    xjr_val callee;
    xjr_size_t argc;
    xjr_val *argv;
    xjr_val ret;

    /* External things */
    xjr_extn *extn;
    xjr_mp_t *mp;
    xjr_urid_t env;
    xjr_val _this;
    xjr_vm *vm;
};
typedef struct xjr_opaque_native_fn_args xjr_native_fn_args;

typedef void (*xjr_native_fn_cb_t)(xjr_native_fn_args *args);



#endif

