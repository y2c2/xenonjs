/* Enhanced C : Encoding : UTF-8
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_encoding_utf8.h"
#include "ec_algorithm.h"
#include "ec_alloc.h"
#include "ec_dt.h"
#include "ec_string.h"

/* #define EC_IS_UTF8_LEADER(ch) (((ch)&128)!=0?1:0) */

/* The bytes number from the leader char */
static ec_size_t ec_utf8_char_length(ec_byte_t ch)
{
    ec_size_t bytes_number;
    ec_u8 uch = (ec_u8)ch;
    /* 0xxxxxxx */
    if ((uch & 0x80) == 0)
        bytes_number = 1;
    /* 110xxxxx, 10xxxxxx */
    else if ((uch & 0xe0) == 0xc0)
        bytes_number = 2;
    /* 1110xxxx, 10xxxxxx, 10xxxxxx */
    else if ((uch & 0xf0) == 0xe0)
        bytes_number = 3;
    /* 11110xxx, 10xxxxxx, 10xxxxxx, 10xxxxxx */
    else if ((uch & 0xf8) == 0xf0)
        bytes_number = 4;
    /* 111110xx, 10xxxxxx, 10xxxxxx, 10xxxxxx, 10xxxxxx */
    else if ((uch & 0xfc) == 0xf8)
        bytes_number = 5;
    /* 1111110x, 10xxxxxx, 10xxxxxx, 10xxxxxx, 10xxxxxx, 10xxxxxx */
    else if ((uch & 0xfe) == 0xfc)
        bytes_number = 6;
    else
        bytes_number = 0;
    return bytes_number;
}

/* The number of unicode chars of a given utf-8 byte stream */
/*
static ec_size_t ec_utf8_string_length(const char *s, ec_size_t len)
{
    const char *p = s;
    ec_size_t utf8_char_len;
    ec_size_t result = 0;

    while (len-- != 0)
    {
        utf8_char_len = 1;
        if (EC_IS_UTF8_LEADER(*p))
        { utf8_char_len = ec_utf8_char_length(*s); }

        s += len;
        result += utf8_char_len;
    }

    return result;
}
*/

#define EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, str_p)                     \
    do                                                                         \
    {                                                                          \
        unicode_char = (unicode_char << 6) | ((*str_p) & 0x3f);                \
        str_p++;                                                               \
    } while (0)

/* The bytes number encoded in utf-8 of a unicode char */
static ec_size_t ec_utf8_char_length_of_unicode_char(ec_char_t uch)
{
    if (uch <= 0x7f)
    {
        return 1;
    }
    else if (uch <= 0x7ff)
    {
        return 2;
    }
    else if (uch <= 0xffff)
    {
        return 3;
    }
    else if (uch <= 0x1fffff)
    {
        return 4;
    }
    else if (uch <= 0x3ffffff)
    {
        return 5;
    }
    else if (uch <= 0x7fffffff)
    {
        return 6;
    }
    else
        return 0;
}

/* The bytes number encoded in utf-8 of a unicode string */
static ec_size_t
ec_utf8_char_length_of_unicode_string(ec_string* unicode_string)
{
    ec_size_t result = 0;
    ect_iterator(ec_string) item1;
    ec_char_t uc;
    ect_for(ec_string, unicode_string, item1)
    {
        uc = (*((ec_char_t*)ec_deref(item1)));
        if (uc == 0)
        {
            return 0;
        }
        else
        {
            result += ec_utf8_char_length_of_unicode_char(uc);
        }
    }
    return result;
}

static int ec_encoding_utf8_encode(ec_byte_t** s, ec_size_t* len,
                                   ec_string* unicode_string)
{
    int ret = 0;
    ec_size_t utf8_string_len;
    ect_iterator(ec_string) item1;
    ec_char_t uc;
    ec_byte_t* new_utf8_s = NULL;
    ec_byte_t* dst_p;
    utf8_string_len = ec_utf8_char_length_of_unicode_string(unicode_string);
    if ((ec_string_length(unicode_string) != 0) && (utf8_string_len == 0))
        return -1;
    if ((new_utf8_s = ec_malloc(sizeof(ec_byte_t) * (utf8_string_len + 1))) ==
        NULL)
    {
        return -1;
    }
    dst_p = new_utf8_s;
    ect_for(ec_string, unicode_string, item1)
    {
        uc = (*((ec_char_t*)ec_deref(item1)));
        switch (ec_utf8_char_length_of_unicode_char(uc))
        {
        case 1:
            *dst_p++ = (ec_byte_t)(uc & 0x7f);
            break;
        case 2:
            *dst_p++ = (ec_byte_t)((0xc0) | ((uc >> 6) & 0x1f));
            *dst_p++ = (ec_byte_t)((0x80) | (uc & 0x3f));
            break;
        case 3:
            *dst_p++ = (ec_byte_t)((0xe0) | ((uc >> 12) & 0xf));
            *dst_p++ = (ec_byte_t)((0x80) | ((uc >> 6) & 0x3f));
            *dst_p++ = (ec_byte_t)((0x80) | (uc & 0x3f));
            break;
        case 4:
            *dst_p++ = (ec_byte_t)((0xf0) | ((uc >> 18) & 0x7));
            *dst_p++ = (ec_byte_t)((0x80) | ((uc >> 12) & 0x3f));
            *dst_p++ = (ec_byte_t)((0x80) | ((uc >> 6) & 0x3f));
            *dst_p++ = (ec_byte_t)((0x80) | (uc & 0x3f));
            break;
        case 5:
            *dst_p++ = (ec_byte_t)((0xf8) | ((uc >> 24) & 0x3));
            *dst_p++ = (ec_byte_t)((0x80) | ((uc >> 18) & 0x3f));
            *dst_p++ = (ec_byte_t)((0x80) | ((uc >> 12) & 0x3f));
            *dst_p++ = (ec_byte_t)((0x80) | ((uc >> 6) & 0x3f));
            *dst_p++ = (ec_byte_t)((0x80) | (uc & 0x3f));
            break;
        case 6:
            *dst_p++ = (ec_byte_t)((0xfc) | ((uc >> 30) & 0x1));
            *dst_p++ = (ec_byte_t)((0x80) | ((uc >> 24) & 0x3f));
            *dst_p++ = (ec_byte_t)((0x80) | ((uc >> 18) & 0x3f));
            *dst_p++ = (ec_byte_t)((0x80) | ((uc >> 12) & 0x3f));
            *dst_p++ = (ec_byte_t)((0x80) | ((uc >> 6) & 0x3f));
            *dst_p++ = (ec_byte_t)((0x80) | (uc & 0x3f));
            break;
        default:
            goto fail;
        }
    }
    *dst_p = '\0';
    *s = new_utf8_s;
    *len = utf8_string_len;
    goto done;
fail:
    if (new_utf8_s != NULL)
        ec_delete(new_utf8_s);
done:
    return ret;
}

static int ec_encoding_utf8_decode(ec_string** unicode_string,
                                   const ec_byte_t* s, ec_size_t len)
{
    int ret = 0;
    ec_string* new_string = NULL;
    ec_byte_t ch_first;
    const ec_byte_t* p = s;
    ec_size_t unicode_char_bytes;
    ec_char_t unicode_char;
    if ((new_string = ec_string_new()) == NULL)
    {
        ret = -1;
        goto fail;
    }
    while (len != 0)
    {
        ch_first = *p++;
        unicode_char_bytes = ec_utf8_char_length(ch_first);
        if ((unicode_char_bytes == 0) || (len < unicode_char_bytes))
        {
            ret = -1;
            goto fail;
        }
        switch (unicode_char_bytes)
        {
        case 1:
            unicode_char = (ec_char_t)ch_first & 0x7f;
            break;
        case 2:
            unicode_char = (ec_char_t)ch_first & 0x1f;
            EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, p);
            break;
        case 3:
            unicode_char = (ec_char_t)ch_first & 0x1f;
            EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, p);
            EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, p);
            break;
        case 4:
            unicode_char = (ec_char_t)ch_first & 0x1f;
            EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, p);
            EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, p);
            EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, p);
            break;
        case 5:
            unicode_char = (ec_char_t)ch_first & 0x1f;
            EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, p);
            EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, p);
            EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, p);
            EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, p);
            break;
        case 6:
            unicode_char = (ec_char_t)ch_first & 0x1f;
            EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, p);
            EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, p);
            EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, p);
            EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, p);
            EC_EAT_FROM_UTF_8_READ_FOLLOW(unicode_char, p);
            break;
        default:
            goto fail;
        }
        len -= unicode_char_bytes;
        ec_string_push_back(new_string, unicode_char);
    }
    *unicode_string = new_string;
    goto done;
fail:
    if (new_string != NULL)
        ec_delete(new_string);
done:
    return ret;
}

int ec_encoding_utf8_init(ec_encoding_t* encoding)
{
    encoding->encode = ec_encoding_utf8_encode;
    encoding->decode = ec_encoding_utf8_decode;
    return 0;
}
