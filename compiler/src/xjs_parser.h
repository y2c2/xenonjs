/* XenonJS : Parser
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_PARSER_H
#define XJS_PARSER_H

#include <ec_string.h>
#include "xjs_types.h"
#include "xjs_error.h"

xjs_ast_program_ref xjs_parser_start_ex( \
        xjs_error_ref err, \
        const xjs_token_list_ref tokens, \
        const xjs_bool module_mode, \
        const char *filename, \
        const xjs_bool loc);

xjs_ast_program_ref xjs_parser_start( \
        xjs_error_ref err, \
        const xjs_token_list_ref tokens, \
        const xjs_bool module_mode);

#endif

