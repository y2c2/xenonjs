/* Enhanced C : String
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_basic_string.h"
#include "ec_libc.h"

define_ec_basic_string(ec_string, ec_char_t);

ec_string* ec_string_append_c_str(ec_string* p, char* s);

ec_string* ec_string_append_c_str(ec_string* p, char* s)
{
    ec_basic_string_size_type len_lhs = p->size;
    ec_basic_string_size_type len_rhs = (ec_basic_string_size_type)ec_strlen(s);
    ec_basic_string_size_type cnt;
    ec_char_t* dst_p;
    char* src_p;
    ec_string__extend(p, len_lhs + len_rhs);
    cnt = len_rhs;
    dst_p = p->_body + p->size;
    src_p = s;
    while (cnt-- != 0)
    {
        *dst_p++ = (ec_char_t)*src_p++;
    }
    *dst_p++ = '\0';
    p->size = len_lhs + len_rhs;
    return p;
}

ec_bool ec_string_match_c_str(ec_string* p, char* s);

ec_bool ec_string_match_c_str(ec_string* p, char* s)
{
    ec_basic_string_size_type len_rhs = (ec_basic_string_size_type)ec_strlen(s);
    ec_basic_string_size_type cnt;
    if (len_rhs != p->size)
        return ec_false;
    for (cnt = 0; cnt != len_rhs; cnt++)
    {
        if (p->_body[cnt] != (ec_char_t)s[cnt])
        {
            return ec_false;
        }
    }
    return ec_true;
}
