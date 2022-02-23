/* Enhanced C : String
 * Copyright(c) 2017-2020 y2c2 */

/* Sequences of 'ec_char_t'-type of chars
 *
 *
 * Prototype:
 * ----------------------
 * ec_string
 * ----------------------
 *
 * Sample:
 * ----------------------
 * #include <ec_string.h>
 * ec_string* s = ec_string_new();
 * ec_delete(s);
 * ----------------------
 */

#ifndef EC_STRING_H
#define EC_STRING_H

#include "ec_basic_string.h"

typedef ec_basic_string_size_type ec_string_size_type;
#define ec_string_npos ((ec_string_size_type)(-1))

declare_ec_basic_string(ec_string, ec_char_t);

#ifndef ECT_STRING_GENERIC_INTERFACE
#define ECT_STRING_GENERIC_INTERFACE
ec_string* ec_string_append_c_str(ec_string* p, char* s);
ec_bool ec_string_match_c_str(ec_string* p, char* s);
#endif

#endif
