/* Enhanced C : Byte String
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_basic_string.h"
#include "ec_libc.h"

typedef ec_basic_string_size_type ec_bytestring_size_type;

define_ec_basic_string(ec_bytestring, char);

ec_bytestring* ec_bytestring_append_c_str(ec_bytestring* p, char* s);

ec_bytestring* ec_bytestring_append_c_str(ec_bytestring* p, char* s)
{
    ec_bytestring_size_type len_lhs = p->size;
    ec_bytestring_size_type len_rhs = (ec_bytestring_size_type)ec_strlen(s);
    ec_bytestring_size_type cnt;
    char *dst_p, *src_p;
    ec_bytestring__extend(p, len_lhs + len_rhs);
    cnt = len_rhs;
    dst_p = p->_body + p->size;
    src_p = s;
    while (cnt-- != 0)
    {
        *dst_p++ = *src_p++;
    }
    *dst_p++ = '\0';
    p->size = len_lhs + len_rhs;
    return p;
}

ec_bool ec_bytestring_match_c_str(ec_bytestring* p, char* s);

ec_bool ec_bytestring_match_c_str(ec_bytestring* p, char* s)
{
    ec_bytestring_size_type len_rhs = (ec_bytestring_size_type)ec_strlen(s);
    ec_bytestring_size_type cnt;
    if (len_rhs != p->size)
        return ec_false;
    for (cnt = 0; cnt != len_rhs; cnt++)
    {
        if (p->_body[cnt] != (char)s[cnt])
        {
            return ec_false;
        }
    }
    return ec_true;
}
