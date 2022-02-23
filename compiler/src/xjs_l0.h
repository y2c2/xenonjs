/* XenonJS : L0
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_L0_H
#define XJS_L0_H

#include "xjs_types.h"

/* L0 (bytecode -> IR) */
int xjs_l0_start( \
        xjs_error_ref err, \
        xjs_ir_ref *ir_out, \
        char *bytecode, xjs_size_t bytecode_len);

#endif

