/* Enhanced C : Byte String
 * Copyright(c) 2017-2020 y2c2 */

/* Sequences of 'char'-type of chars
 *
 *
 * Prototype:
 * ----------------------
 * ec_bytestring
 * ----------------------
 *
 * Example:
 * ----------------------
 * ec_bytestring* s = ec_bytestring_new();
 * ec_delete(s);
 * ----------------------
 */

#ifndef EC_BYTESTRING_H
#define EC_BYTESTRING_H

#include "ec_basic_string.h"

typedef ec_basic_string_size_type ec_bytestring_size_type;
#define ec_bytestring_npos ((ec_bytestring_size_type)(-1))

declare_ec_basic_string(ec_bytestring, char);

#ifndef ECT_BYTESTRING_GENERIC_INTERFACE
#define ECT_BYTESTRING_GENERIC_INTERFACE
#define ec_bytestring_c_str(p) (ec_bytestring__body_get(p))
ec_bytestring* ec_bytestring_append_c_str(ec_bytestring* p, char* s);
ec_bool ec_bytestring_match_c_str(ec_bytestring* p, char* s);
#endif

typedef ec_basic_string_size_type ec_bytestring_size_type;
#define ec_bytestring_npos ((ec_bytestring_size_type)(-1))

#endif
