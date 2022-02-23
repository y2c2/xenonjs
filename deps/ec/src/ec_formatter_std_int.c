/* Enhanced C : Formatter : Standard : int
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_formatter_std_int.h"
#include "ec_algorithm.h"
#include "ec_dt.h"
#include "ec_formatter.h"
#include "ec_string.h"

#define make_signed_integer_cb_apply_anchor(_anchor_name, _arg_type)           \
    static int ec_formatter_plugin_cb_apply_anchor_##_anchor_name(             \
        ec_formatter_plugin_args_apply_anchor_t* args)                         \
    {                                                                          \
        _arg_type x =                                                          \
            ec_formatter_plugin_args_apply_anchor_va_arg(args, _arg_type);     \
        if (x == 0)                                                            \
        {                                                                      \
            ec_formatter_plugin_args_apply_anchor_append_c_str(args, "0");     \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            if (x < 0)                                                         \
            {                                                                  \
                ec_formatter_plugin_args_apply_anchor_append_c_str(args, "-"); \
                x = -x;                                                        \
            }                                                                  \
            {                                                                  \
                ec_string* digits;                                             \
                ect_reverse_iterator(ec_string) it_d;                          \
                digits = ec_string_new();                                      \
                while (x != 0)                                                 \
                {                                                              \
                    ec_string_push_back(digits,                                \
                                        (ec_char_t)("0123456789"[x % 10]));    \
                    x /= 10;                                                   \
                }                                                              \
                ect_for_reverse(ec_string, digits, it_d)                       \
                {                                                              \
                    ec_formatter_plugin_args_apply_anchor_append_char(         \
                        args, ect_deref(ec_char_t, it_d));                     \
                }                                                              \
                ec_delete(digits);                                             \
            }                                                                  \
        }                                                                      \
        return 0;                                                              \
    }

#define make_unsigned_integer_cb_apply_anchor(_anchor_name, _arg_type)         \
    static int ec_formatter_plugin_cb_apply_anchor_##_anchor_name(             \
        ec_formatter_plugin_args_apply_anchor_t* args)                         \
    {                                                                          \
        _arg_type x =                                                          \
            ec_formatter_plugin_args_apply_anchor_va_arg(args, _arg_type);     \
        if (x == 0)                                                            \
        {                                                                      \
            ec_formatter_plugin_args_apply_anchor_append_c_str(args, "0");     \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            {                                                                  \
                ec_string* digits;                                             \
                ect_reverse_iterator(ec_string) it_d;                          \
                digits = ec_string_new();                                      \
                while (x != 0)                                                 \
                {                                                              \
                    ec_string_push_back(digits,                                \
                                        (ec_char_t)("0123456789"[x % 10]));    \
                    x /= 10;                                                   \
                }                                                              \
                ect_for_reverse(ec_string, digits, it_d)                       \
                {                                                              \
                    ec_formatter_plugin_args_apply_anchor_append_char(         \
                        args, ect_deref(ec_char_t, it_d));                     \
                }                                                              \
                ec_delete(digits);                                             \
            }                                                                  \
        }                                                                      \
        return 0;                                                              \
    }

make_signed_integer_cb_apply_anchor(int, int)
    make_unsigned_integer_cb_apply_anchor(unsigned_int, unsigned int)
        make_signed_integer_cb_apply_anchor(s8, int)
            make_unsigned_integer_cb_apply_anchor(u8, int)
                make_signed_integer_cb_apply_anchor(s16, int)
                    make_unsigned_integer_cb_apply_anchor(u16, int)
                        make_signed_integer_cb_apply_anchor(s32, ec_s32)
                            make_unsigned_integer_cb_apply_anchor(u32, ec_u32)
                                make_unsigned_integer_cb_apply_anchor(size_t,
                                                                      ec_size_t)

                                    int ec_formatter_install_plugin_int(
                                        ec_formatter_t* formatter)
{
    ec_formatter_install_plugin(formatter, "int", NULL, NULL, NULL,
                                ec_formatter_plugin_cb_apply_anchor_int);
    return 0;
}

int ec_formatter_install_plugin_unsigned_int(ec_formatter_t* formatter)
{
    ec_formatter_install_plugin(
        formatter, "unsigned_int", NULL, NULL, NULL,
        ec_formatter_plugin_cb_apply_anchor_unsigned_int);
    return 0;
}

int ec_formatter_install_plugin_s8(ec_formatter_t* formatter)
{
    ec_formatter_install_plugin(formatter, "s8", NULL, NULL, NULL,
                                ec_formatter_plugin_cb_apply_anchor_s8);
    return 0;
}

int ec_formatter_install_plugin_u8(ec_formatter_t* formatter)
{
    ec_formatter_install_plugin(formatter, "u8", NULL, NULL, NULL,
                                ec_formatter_plugin_cb_apply_anchor_u8);
    return 0;
}

int ec_formatter_install_plugin_s16(ec_formatter_t* formatter)
{
    ec_formatter_install_plugin(formatter, "s16", NULL, NULL, NULL,
                                ec_formatter_plugin_cb_apply_anchor_s16);
    return 0;
}

int ec_formatter_install_plugin_u16(ec_formatter_t* formatter)
{
    ec_formatter_install_plugin(formatter, "u16", NULL, NULL, NULL,
                                ec_formatter_plugin_cb_apply_anchor_u16);
    return 0;
}

int ec_formatter_install_plugin_s32(ec_formatter_t* formatter)
{
    ec_formatter_install_plugin(formatter, "s32", NULL, NULL, NULL,
                                ec_formatter_plugin_cb_apply_anchor_s32);
    return 0;
}

int ec_formatter_install_plugin_u32(ec_formatter_t* formatter)
{
    ec_formatter_install_plugin(formatter, "u32", NULL, NULL, NULL,
                                ec_formatter_plugin_cb_apply_anchor_u32);
    return 0;
}

int ec_formatter_install_plugin_size_t(ec_formatter_t* formatter)
{
    ec_formatter_install_plugin(formatter, "size_t", NULL, NULL, NULL,
                                ec_formatter_plugin_cb_apply_anchor_size_t);
    return 0;
}
