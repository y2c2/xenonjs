/* XenonJS : L1 : Standard Library
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_L1_STDLIB_H
#define XJS_L1_STDLIB_H

#include "xjs_private.h"
#include "xjs_l1_stdlib.h"

#define XJS_STDLIB_MODULE_NAME "stdlib"
#define XJS_STDLIB_MODULE_NAME_LEN 6

#define XJS_STDLIB_FILE_NAME "/stdlib.js"
#define XJS_STDLIB_FILE_NAME_LEN 10

int xjs_l1_ir_generate( \
        xjs_error_ref err, \
        xjs_ir_ref *ir_out, \
        const char *source, const xjs_size_t source_len, \
        char *module_name, xjs_size_t module_name_len, \
        char *file_name, xjs_size_t file_name_len, \
        int generate_debug_info);

int xjs_l1_stdlib_generate( \
        xjs_error_ref err, \
        xjs_ir_ref *ir_out, \
        const char *stdlib_js, const xjs_size_t stdlib_js_len, \
        int generate_debug_info);


#endif

