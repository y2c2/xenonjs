/* XenonJS : Runtime Time System : Builtin Functions
 * Copyright(c) 2017 y2c2 */

#include "xjr_dt.h"
#include "xjr_val.h"
#include "xjr_extn.h"
#include "xjr_err.h"
#include "xjr_mbuf.h"
#include "xjr_libc.h"
#include "xjr_nativefn.h"
#include "xjr_env.h"
#include "xjr_bltin_helper.h"
#include "xjr_bltin_infra.h"
#include "xjr_bltin_math.h"
#include "xjr_bltin_json.h"
#include "xjr_bltin_typedarray.h"
#include "xjr_bltin.h"

int xjr_bltin_init(xjr_vm *vm, xjr_urid_t env)
{
    if ((vm->opts & XJR_VM_OPTIONS_NOSTDLIB) == 0)
    {
        /* Infra */
        xjr_bltin_infra_init(vm, env);
        /* Math */
        xjr_bltin_math_init(vm, env);
        /* JSON */
        xjr_bltin_json_init(vm, env);
        /* TypedArray */
        xjr_bltin_typedarray_init(vm, env);
    }

    return 0;
}

