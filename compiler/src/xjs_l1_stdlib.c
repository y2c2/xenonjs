/* XenonJS : L1 : Standard Library
 * Copyright(c) 2017 y2c2 */

#include <ec_encoding.h>
#include <ec_encoding_utf8.h>
#include "xjs_error.h"
#include "xjs_lexer.h"
#include "xjs_parser.h"
#include "xjs_c0.h"
#include "xjs_ir.h"
#include "xjs_c2.h"
#include "xjs_l1_stdlib.h"

int xjs_l1_ir_generate( \
        xjs_error_ref err, \
        xjs_ir_ref *ir_out, \
        const char *source, const xjs_size_t source_len, \
        char *module_name, xjs_size_t module_name_len, \
        char *file_name, xjs_size_t file_name_len, \
        int generate_debug_info)
{
    int ret = 0;
    ec_string *uscript = NULL;
    xjs_token_list_ref tokens = NULL;
    xjs_ast_program_ref ast = NULL;
    xjs_cfg_ref cfg = NULL;
    xjs_ir_ref new_ir = NULL;

    /* Decoce */
    {
        int tmpret;
        ec_encoding_t enc;
        ec_encoding_utf8_init(&enc);
        tmpret = ec_encoding_decode(&enc, &uscript, (const ec_byte_t *)source, source_len);
        if ((tmpret != 0) || (uscript == NULL))
        {
            XJS_ERROR(err, XJS_ERRNO_LINK);
            XJS_ERROR_PRINTF(err, "error: compile standard library failed");
            goto fail;
        }
    }

    /* Tokenize */
    tokens = xjs_lexer_start_ex(err, uscript, file_name);
    ec_delete(uscript);
    if (tokens == NULL) { goto fail; }
    uscript = NULL;

    /* Parse */
    ast = xjs_parser_start_ex(err, tokens, xjs_true, \
            file_name, \
            generate_debug_info ? xjs_true : xjs_false);
    ec_delete(tokens);
    if (ast == NULL) { goto fail; }
    tokens = NULL;
    
    /* C0 */
    cfg = xjs_c0_start_ex(err, ast, file_name);
    ec_delete(ast);
    if (cfg == NULL) { goto fail; }
    ast = NULL;

    /* Generate IR */
    new_ir = xjs_c2_start_ex(err, cfg, file_name);
    ec_delete(cfg);
    if (new_ir == NULL) { goto fail; }
    cfg = NULL;

    xjs_ir_module_name_set(new_ir, module_name, module_name_len);
    xjs_ir_module_fullpath_set(new_ir, file_name, file_name_len);

    *ir_out = new_ir;

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

int xjs_l1_stdlib_generate( \
        xjs_error_ref err, \
        xjs_ir_ref *ir_out, \
        const char *stdlib_js, const xjs_size_t stdlib_js_len, \
        int generate_debug_info)
{
    return xjs_l1_ir_generate( \
            err, ir_out, \
            stdlib_js, stdlib_js_len, \
            XJS_STDLIB_MODULE_NAME, XJS_STDLIB_MODULE_NAME_LEN, \
            XJS_STDLIB_FILE_NAME, XJS_STDLIB_FILE_NAME_LEN, \
            generate_debug_info);
}

