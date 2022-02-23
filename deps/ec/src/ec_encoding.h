/* Enhanced C : Encoding
 * Copyright(c) 2017-2020 y2c2 */

/* Encoding
 * ----------------------
 * #include <ec_encoding.h>
 * #include <ec_encoding_utf8.h>
 * ec_encoding_t enc;
 * ec_encoding_utf8_init(&enc);
 * ec_string* s = ec_string_new();
 * ec_byte_t* encoded_bytes = NULL;
 * ec_size_t encoded_bytes_len;
 * ec_encoding_encode(&enc, &encoded_bytes, &encoded_bytes_len, s);
 * ec_free(encoded_bytes);
 * ec_delete(s);
 * ----------------------
 */

#ifndef EC_ENCODING_H
#define EC_ENCODING_H

#include "ec_dt.h"
#include "ec_encoding_dt.h"
#include "ec_string.h"

/* Initialize UTF-8 Encoding */
int ec_encoding_utf8_init(ec_encoding_t* encoding);

/* Encode */
int ec_encoding_encode(ec_encoding_t* encoding, ec_byte_t** s, ec_size_t* len,
                       ec_string* unicode_string);

/* Decode */
int ec_encoding_decode(ec_encoding_t* encoding, ec_string** unicode_string,
                       const ec_byte_t* s, const ec_size_t len);

#endif
