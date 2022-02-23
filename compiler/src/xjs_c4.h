/* XenonJS : C4
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_C4_C
#define XJS_C4_C

#include "xjs_types.h"

/* C4 (IR -> bytecode) */

int xjs_c4_start_ex( \
        xjs_error_ref err, \
        char **bytecode_out, xjs_size_t *bytecode_len_out, \
        xjs_ir_ref ir, \
        const xjs_bool generate_debug_info);

int xjs_c4_start( \
        xjs_error_ref err, \
        char **bytecode_out, xjs_size_t *bytecode_len_out, \
        xjs_ir_ref ir);

#endif


