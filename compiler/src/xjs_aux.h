/* XenonJS : Lexer
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_AUX_H
#define XJS_AUX_H

#include <ec_string.h>
#include "xjs_types.h"

xjs_bool xjs_aux_string_match_with(ec_string *s, const char *c_str);

int xjs_aux_serialize_append_number(ec_string *result, double value);

int xjs_aux_serialize_append_int(ec_string *result, int value);

ec_string *xjs_aux_basename(ec_string *fullname);

ec_string *xjs_aux_extname(ec_string *fullname);

ec_string *xjs_aux_mainname(ec_string *fullname);

#endif

