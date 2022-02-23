/* XenonJS : C0
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_C0_H
#define XJS_C0_H

#include "xjs_types.h"

/* C0 */
xjs_cfg_ref xjs_c0_start_ex( \
        xjs_error_ref err, \
        xjs_ast_program_ref program, \
        const char *filename);

xjs_cfg_ref xjs_c0_start( \
        xjs_error_ref err, \
        xjs_ast_program_ref program);

#endif

