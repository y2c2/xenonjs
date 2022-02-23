/* XenonJS : L0
 * Copyright(c) 2017 y2c2 */

#include "xjs_helper.h"
#include "xjs_l0.h"

/* L0 (bytecode -> IR) */
int xjs_l0_start( \
        xjs_error_ref err, \
        xjs_ir_ref *ir_out, \
        char *bytecode, xjs_size_t bytecode_len)
{
    int ret = -1;

    XJS_ERROR(err, XJS_ERRNO_NOTIMP);
    XJS_ERROR_PUTS(err, "error: 'l0' not implemented");

    (void)err;
    (void)ir_out;
    (void)bytecode;
    (void)bytecode_len;

    return ret;
}

