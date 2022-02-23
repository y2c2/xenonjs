/* XenonJS : C2
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_C2_H
#define XJS_C2_H

#include "xjs_types.h"

/* C2 (CFG -> IR) */
xjs_ir_ref xjs_c2_start_ex( \
        xjs_error_ref err, \
        xjs_cfg_ref cfg, \
        const char *filename);
xjs_ir_ref xjs_c2_start( \
        xjs_error_ref err, \
        xjs_cfg_ref cfg);

#endif
