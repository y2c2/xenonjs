/* XenonJS : AST Printer
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_ASTPRINTER_H
#define XJS_ASTPRINTER_H

#include <ec_string.h>
#include "xjs.h"

/* AST Printer */
int xjs_astprinter_start( \
        xjs_error_ref err, \
        ec_string **result_out, \
        xjs_ast_program_ref ast);

#endif

