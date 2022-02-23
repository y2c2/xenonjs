/* XenonJS : L1
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_L1_H
#define XJS_L1_H

#include "xjs_types.h"

/* L1 ([IR] -> IR) */
int xjs_l1_start( \
        xjs_error_ref err, \
        xjs_ir_ref *ir_out, \
        xjs_ir_ref *ir_in, xjs_size_t ir_in_count, \
        const char *entry, \
        const xjs_bool no_stdlib, \
        const char *stdlib_source_data, \
        const xjs_size_t stdlib_source_len, \
        xjs_load_sys_lib_cb_t load_sys_lib_cb, \
        int generate_debug_info);

#endif

