#include <ec_encoding.h>
#include <ec_string.h>
#include <ec_algorithm.h>
#include "xjs.h"
#include "test_lexer_serialize.h"

static int xjs_token_list_serialize_append_unescaped_json_string( \
        ec_string *result, ec_string *value_string)
{
    ect_iterator(ec_string) it;
    ec_char_t ch;

    ect_for(ec_string, value_string, it)
    {
        ch = ect_deref(ec_char_t , it);
        if (ch == '"')
        {
            ec_string_push_back(result, '\\');
            ec_string_push_back(result, '\"');
        }
        else if (ch == '\\')
        {
            ec_string_push_back(result, '\\');
            ec_string_push_back(result, '\\');
        }
        else
        {
            ec_string_push_back(result, ch);
        }
    }

    return 0;
}

char *xjs_token_list_serialize(xjs_token_list_ref tokens)
{
    char *encoded_result = NULL;
    ec_size_t encoded_result_len;
    ec_string *result = NULL;
    xjs_token_ref token_cur;
    ec_bool first = ec_true;
    ect_iterator(xjs_token_list) it_token;

    if ((result = ec_string_new()) == NULL) { return NULL; }
    ec_string_push_back(result, '[');

    ect_for(xjs_token_list, tokens, it_token)
    {
        token_cur = ect_deref(xjs_token_ref, it_token);

        /* Skip EOF */
        if (xjs_token_type_get(token_cur) == XJS_TOKEN_TYPE_EOF)
        { break; }

        if (first == ec_true) first = ec_false; else ec_string_append_c_str(result, ",");

        ec_string_push_back(result, '{');
        switch (xjs_token_type_get(token_cur))
        {
#define _s_append(_x) ec_string_append_c_str(result, "\"type\":\"" _x "\"");
            case XJS_TOKEN_TYPE_COMMENT: _s_append("Comment") break;
            case XJS_TOKEN_TYPE_UNKNOWN: _s_append("Unknown") break;
            case XJS_TOKEN_TYPE_EOF: _s_append("EOF") break;
            case XJS_TOKEN_TYPE_IDENTIFIER: _s_append("Identifier") break;
            case XJS_TOKEN_TYPE_KEYWORD: _s_append("Keyword") break;
            case XJS_TOKEN_TYPE_NULL_LITERAL: _s_append("Null") break;
            case XJS_TOKEN_TYPE_BOOLEAN_LITERAL: _s_append("Boolean") break;
            case XJS_TOKEN_TYPE_NUMERIC_LITERAL: _s_append("Numeric") break;
            case XJS_TOKEN_TYPE_STRING_LITERAL: _s_append("String") break;
            case XJS_TOKEN_TYPE_PUNCTUATOR: _s_append("Punctuator") break;
#undef _s_append
        }
        ec_string_append_c_str(result, ",\"value\":\"");
        if (xjs_token_type_get(token_cur) == XJS_TOKEN_TYPE_STRING_LITERAL)
        {
            xjs_token_list_serialize_append_unescaped_json_string( \
                    result, xjs_token_value_get(token_cur));
        }
        else
        {
            ec_string_append(result, xjs_token_value_get(token_cur));
        }
        ec_string_push_back(result, '\"');
        ec_string_push_back(result, '}');
    }

    ec_string_push_back(result, ']');

    /* Encode as UTF-8 */
    {
        ec_encoding_t enc;
        ec_encoding_utf8_init(&enc);
        ec_encoding_encode(&enc, (ec_byte_t **)&encoded_result, &encoded_result_len, result);
    }

    ec_delete(result);
    return encoded_result;
}

