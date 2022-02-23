/* XenonJS : Lexer
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_LEXER_H
#define XJS_LEXER_H

#include <ec_string.h>
#include "xjs_types.h"
#include "xjs_error.h"

xjs_token_list_ref xjs_lexer_start_ex( \
        xjs_error_ref err, \
        ec_string *src_code, \
        const char *filename);

xjs_token_list_ref xjs_lexer_start( \
        xjs_error_ref err, \
        ec_string *src_code);

#endif

