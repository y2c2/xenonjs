/* Enhanced C : Encoding : Data Types
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_ENCODING_DT_H
#define EC_ENCODING_DT_H

#include "ec_dt.h"
#include "ec_string.h"

/* Convert from Unicode to bytes */
typedef int (*ec_encoding_encode_cb_t)(ec_byte_t** s, ec_size_t* len,
                                       ec_string* unicode_string);

/* Convert from bytes to Unicode */
typedef int (*ec_encoding_decode_cb_t)(ec_string** unicode_string,
                                       const ec_byte_t* s, ec_size_t len);

struct ec_encoding
{
    ec_encoding_encode_cb_t encode;
    ec_encoding_decode_cb_t decode;
};
typedef struct ec_encoding ec_encoding_t;

#endif
