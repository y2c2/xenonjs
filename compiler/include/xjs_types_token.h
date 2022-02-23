/* XenonJS : Types : Token
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_TYPES_TOKEN_H
#define XJS_TYPES_TOKEN_H

#include <ec_list.h>

/* Token */

enum xjs_opaque_token_type
{
    XJS_TOKEN_TYPE_UNKNOWN = 0,
    XJS_TOKEN_TYPE_EOF,
    XJS_TOKEN_TYPE_IDENTIFIER,
    XJS_TOKEN_TYPE_KEYWORD,
    XJS_TOKEN_TYPE_NULL_LITERAL,
    XJS_TOKEN_TYPE_BOOLEAN_LITERAL,
    XJS_TOKEN_TYPE_NUMERIC_LITERAL,
    XJS_TOKEN_TYPE_STRING_LITERAL,
    XJS_TOKEN_TYPE_PUNCTUATOR,
    XJS_TOKEN_TYPE_COMMENT,
};
typedef enum xjs_opaque_token_type xjs_token_type;

struct xjs_opaque_token;
typedef struct xjs_opaque_token xjs_token;
typedef struct xjs_opaque_token *xjs_token_ref;

/* Token List */
ect_list_declare(xjs_token_list, xjs_token_ref);

#endif

