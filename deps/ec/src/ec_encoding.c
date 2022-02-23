/* Enhanced C : Encoding
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_encoding.h"
#include "ec_dt.h"
#include "ec_string.h"

/* Encode */
int ec_encoding_encode(ec_encoding_t* encoding, ec_byte_t** s, ec_size_t* len,
                       ec_string* unicode_string)
{
    return encoding->encode(s, len, unicode_string);
}

/* Decode */
int ec_encoding_decode(ec_encoding_t* encoding, ec_string** unicode_string,
                       const ec_byte_t* s, const ec_size_t len)
{
    return encoding->decode(unicode_string, s, len);
}
