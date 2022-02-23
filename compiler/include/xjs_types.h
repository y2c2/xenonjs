/* XenonJS : Types
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_TYPES_H
#define XJS_TYPES_H

#include "xjs_types_error.h"
#include "xjs_types_token.h"
#include "xjs_types_ast.h"
#include "xjs_types_cfg.h"
#include "xjs_types_c1.h"
#include "xjs_types_ir.h"
#include "xjs_types_val.h"
#include "xjs_types_vm.h"

struct xjs_opaque_token;
typedef struct xjs_opaque_token xjs_token;
typedef struct xjs_opaque_token *xjs_token_ref;

struct xjs_token_list_container;
typedef struct xjs_token_list_container xjs_token_list;
typedef struct xjs_token_list_container *xjs_token_list_ref;

struct xjs_opaque_lexer;
typedef struct xjs_opaque_lexer xjs_lexer;
typedef struct xjs_opaque_lexer *xjs_lexer_ref;

typedef int (xjs_load_sys_lib_cb_t)( \
        char **data_out, xjs_size_t *size_out, \
        char **lib_fullpath_out, xjs_size_t *lib_fullpath_len_out, \
        const char *name, const xjs_size_t name_len, \
        xjs_u32 opts);

#endif

