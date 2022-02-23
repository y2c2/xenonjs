/* XenonJS : C1
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_C1_H
#define XJS_C1_H

#include "xjs_types.h"

/* C1 (CFG Oriented Optimization) */
xjs_cfg_ref xjs_c1_start( \
        xjs_error_ref err, \
        xjs_cfg_ref cfg, \
        xjs_c1_options_ref opts);

#endif

