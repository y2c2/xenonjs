/* XenonJS : IR Printer
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_IRPRINTER_H
#define XJS_IRPRINTER_H

#include <ec_string.h>
#include "xjs.h"

/* IR Printer */
int xjs_irprinter_start_ex( \
        xjs_error_ref err, \
        ec_string **result_out, \
        xjs_ir_ref ir, \
        const xjs_bool generate_debug_info);

int xjs_irprinter_start( \
        xjs_error_ref err, \
        ec_string **result_out, \
        xjs_ir_ref ir);

#endif

