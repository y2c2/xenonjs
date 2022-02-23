/* XenonJS : C4 : Register Allocation
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_C4_REGALLOC_H
#define XJS_C4_REGALLOC_H

#include "xjs_types.h"
#include "xjs_c4_helper.h"

/* Register Scheduler */

typedef struct xjs_opaque_regsched xjs_regsched;
typedef struct xjs_opaque_regsched *xjs_regsched_ref;

/* Create a new register scheduler (function scope) */
xjs_regsched_ref xjs_regsched_new(xjs_error_ref err);

int xjs_regsched_set_simple(xjs_regsched_ref sched, \
        ec_size_t regs_count);

int xjs_regsched_set_r1( \
        xjs_error_ref err, \
        xjs_regsched_ref sched, \
        xjs_ir_function_ref func);

int xjs_regsched_alloc(xjs_regsched_ref sched, \
        xjs_size_t lp_cur, \
        xjs_ir_var var);

/* Get the number requirement of registers */
ec_size_t xjs_regsched_requirement(xjs_regsched_ref sched);


#endif

