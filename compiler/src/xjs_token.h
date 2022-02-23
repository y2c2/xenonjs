/* XenonJS : Token
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_TOKEN_H
#define XJS_TOKEN_H

#include <ec_list.h>
#include "xjs_types.h"

/* Token */
xjs_token_ref xjs_token_new(void);

/* Type */
xjs_token_type xjs_token_type_get(xjs_token_ref token);
void xjs_token_type_set(xjs_token_ref token, xjs_token_type type);

/* Value */
ec_string *xjs_token_value_get(xjs_token_ref token);
void xjs_token_value_set(xjs_token_ref token, ec_string *value);

/* Loc */
void xjs_token_loc_start_set(xjs_token_ref token, \
        const xjs_size_t start_ln, const xjs_size_t start_col);

void xjs_token_loc_end_set(xjs_token_ref token, \
        const xjs_size_t end_ln, const xjs_size_t end_col);

xjs_size_t xjs_token_loc_start_ln_get(xjs_token_ref token);
xjs_size_t xjs_token_loc_start_col_get(xjs_token_ref token);
xjs_size_t xjs_token_loc_end_ln_get(xjs_token_ref token);
xjs_size_t xjs_token_loc_end_col_get(xjs_token_ref token);

void xjs_token_range_start_set(xjs_token_ref token, \
        const xjs_size_t start);
void xjs_token_range_end_set(xjs_token_ref token, \
        const xjs_size_t end);
xjs_size_t xjs_token_range_start_get(xjs_token_ref token);
xjs_size_t xjs_token_range_end_get(xjs_token_ref token);

#endif

