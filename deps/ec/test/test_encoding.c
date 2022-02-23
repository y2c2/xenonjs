#include "test_encoding.h"
#include "ec_encoding.h"
#include "ec_encoding_utf8.h"
#include "ec_string.h"
#include "testfw.h"
#include <stdio.h>

void test_encoding(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("encoding");
    {
        ec_encoding_t enc;

        ec_encoding_utf8_init(&enc);
    }
    {
        /* encoding */
        ec_encoding_t enc;
        ec_string* s = NULL;
        ec_byte_t* encoded_bytes = NULL;
        ec_size_t encoded_bytes_len;

        ec_encoding_utf8_init(&enc);
        s = ec_string_new();
        ec_string_push_back(s, 'a');
        ec_string_push_back(s, 'b');
        ec_string_push_back(s, 'c');

        CUNITTEST_ASSERT_EQ(
            ec_encoding_encode(&enc, &encoded_bytes, &encoded_bytes_len, s), 0);
        CUNITTEST_ASSERT_EQ(encoded_bytes_len, 3);
        CUNITTEST_ASSERT_EQ(encoded_bytes[0], 'a');
        CUNITTEST_ASSERT_EQ(encoded_bytes[1], 'b');
        CUNITTEST_ASSERT_EQ(encoded_bytes[2], 'c');
        ec_free(encoded_bytes);

        ec_delete(s);
    }
    {
        /* decoding */
        ec_encoding_t enc;
        ec_string* decoded_s = NULL;
        ec_byte_t* encoded_bytes = (ec_byte_t*)"abc";
        ec_size_t encoded_bytes_len = 3;

        ec_encoding_utf8_init(&enc);

        CUNITTEST_ASSERT_EQ(ec_encoding_decode(&enc, &decoded_s, encoded_bytes,
                                               encoded_bytes_len),
                            0);
        CUNITTEST_ASSERT_EQ(ec_string_length(decoded_s), 3);
        CUNITTEST_ASSERT_EQ(ec_string_at(decoded_s, 0), 'a');
        CUNITTEST_ASSERT_EQ(ec_string_at(decoded_s, 1), 'b');
        CUNITTEST_ASSERT_EQ(ec_string_at(decoded_s, 2), 'c');

        ec_delete(decoded_s);
    }
    CUNITTEST_RESULT();
}
