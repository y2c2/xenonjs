#include "test_formatter.h"
#include "ec_bytestring.h"
#include "ec_encoding.h"
#include "ec_formatter.h"
#include "ec_formatter_template.h"
#include "ec_string.h"
#include "testfw.h"
#include <stdarg.h>
#include <stdio.h>

#define test_formatter_template_item_ok(_s, _expected_items_count)             \
    do                                                                         \
    {                                                                          \
        ec_formatter_template_t* tplt = NULL;                                  \
        CUNITTEST_ASSERT_EQ(                                                   \
            ec_formatter_template_build_from_fmt_cstr(&tplt, formatter, _s),   \
            0);                                                                \
        CUNITTEST_ASSERT_NE(tplt, NULL);                                       \
        CUNITTEST_ASSERT_EQ(ec_formatter_template_items_count(tplt),           \
                            _expected_items_count);                            \
        ec_delete(tplt);                                                       \
    } while (0)

#define test_formatter_template_item_error(_s)                                 \
    do                                                                         \
    {                                                                          \
        ec_formatter_template_t* tplt = NULL;                                  \
        CUNITTEST_ASSERT_NE(                                                   \
            ec_formatter_template_build_from_fmt_cstr(&tplt, formatter, _s),   \
            0);                                                                \
        ec_delete(tplt);                                                       \
    } while (0)

static void test_formatter_template(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("formatter : template");
    {
        /* template */
        ec_formatter_t* formatter = ec_formatter_new();

        /* Install plugin */
        ec_formatter_install_plugin(formatter, "b", NULL, NULL, NULL, NULL);
        ec_formatter_install_plugin(formatter, "bc", NULL, NULL, NULL, NULL);

        test_formatter_template_item_ok("", 0);
        test_formatter_template_item_ok("a", 1);
        test_formatter_template_item_error("}"); /* '}' must be escaped */
        test_formatter_template_item_ok("{{", 1);
        test_formatter_template_item_ok("}}", 1);
        test_formatter_template_item_ok("ab", 1);
        test_formatter_template_item_ok("abc", 1);
        test_formatter_template_item_error("a{");
        test_formatter_template_item_error("a}");
        test_formatter_template_item_ok("a{{", 2);
        test_formatter_template_item_error("a{}");
        test_formatter_template_item_error("a{(");
        test_formatter_template_item_error("a{)");
        test_formatter_template_item_error("a{a");
        test_formatter_template_item_error("a{a{");
        test_formatter_template_item_error("a{a}"); /* plugin 'a' not exists */
        test_formatter_template_item_ok("a{b}", 2); /* plugin 'b' exists */
        test_formatter_template_item_error(
            "a{ba}"); /* plugin 'ba' not exists */
        test_formatter_template_item_ok("a{bc}", 2); /* plugin 'bc' exists */
        test_formatter_template_item_ok("a{b}c", 3);
        test_formatter_template_item_error("a{b(");
        test_formatter_template_item_error("a{b({");
        test_formatter_template_item_error("a{b(}");
        test_formatter_template_item_error("a{b((");
        test_formatter_template_item_error("a{b()");
        test_formatter_template_item_error("a{b(,");
        test_formatter_template_item_error("a{b(:");
        test_formatter_template_item_error("a{b(a");
        test_formatter_template_item_error("a{b(a:");
        test_formatter_template_item_error("a{b(a:1");
        test_formatter_template_item_error("a{b(a:{");
        test_formatter_template_item_error("a{b(a:}");
        test_formatter_template_item_error("a{b(a:(");
        test_formatter_template_item_error("a{b(a:)");
        test_formatter_template_item_error("a{b(a::");
        test_formatter_template_item_error("a{b(a:,");
        test_formatter_template_item_error("a{b(a:1");
        test_formatter_template_item_error("a{b(a:1)");
        test_formatter_template_item_ok("a{b(a:1)}", 2);
        test_formatter_template_item_error("a{b(a:1,)}");
        test_formatter_template_item_error("{");
        test_formatter_template_item_error("{a");
        test_formatter_template_item_error("{a}"); /* plugin 'a' not exists */
        test_formatter_template_item_ok("{b}", 1); /* plugin 'bc' exists */
        test_formatter_template_item_ok("{b}{b}", 2); /* plugin 'bc' exists */
        test_formatter_template_item_ok("{b}{b}{b}",
                                        3); /* plugin 'bc' exists */

        ec_delete(formatter);
    }
    CUNITTEST_RESULT();
}

static int ec_formatter_plugin_cb_apply_anchor_int(
    ec_formatter_plugin_args_apply_anchor_t* args)
{
    int x = ec_formatter_plugin_args_apply_anchor_va_arg(args, int);
    char buf[20 + 1];
    snprintf(buf, 20, "%d", x);

    ec_formatter_plugin_args_apply_anchor_append_c_str(args, buf);

    return 0;
}

#define test_formatter_plugin_item(_expected_result, ...)                      \
    do                                                                         \
    {                                                                          \
        ec_encoding_t enc;                                                     \
        ec_string *result = NULL, *decoded_expected_result = NULL;             \
        ec_encoding_utf8_init(&enc);                                           \
        ec_encoding_decode(&enc, &decoded_expected_result,                     \
                           (const ec_byte_t*)_expected_result,                 \
                           strlen(_expected_result));                          \
        CUNITTEST_ASSERT_EQ(                                                   \
            ec_formatter_format(formatter, &result, __VA_ARGS__), 0);          \
        CUNITTEST_ASSERT_NE(result, NULL);                                     \
        CUNITTEST_ASSERT_EQ(ec_string_eq(result, decoded_expected_result),     \
                            ec_true);                                          \
        ec_delete(result);                                                     \
        ec_delete(decoded_expected_result);                                    \
    } while (0)

static void test_formatter_plugin(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("formatter : plugin");
    {
        /* template */
        ec_formatter_t* formatter = ec_formatter_new();

        /* Install plugin */
        ec_formatter_install_plugin(formatter, "int", NULL, NULL, NULL,
                                    ec_formatter_plugin_cb_apply_anchor_int);

        test_formatter_plugin_item("", "");
        test_formatter_plugin_item("a", "a");
        test_formatter_plugin_item("ab", "ab");
        test_formatter_plugin_item("{", "{{");
        test_formatter_plugin_item("}", "}}");
        test_formatter_plugin_item("123", "{int}", 123);
        test_formatter_plugin_item("123456", "{int}{int}", 123, 456);
        test_formatter_plugin_item("a123", "a{int}", 123);
        test_formatter_plugin_item("123b", "{int}b", 123);
        test_formatter_plugin_item("a123b", "a{int}b", 123);
        test_formatter_plugin_item("-123", "{int}", -123);

        ec_delete(formatter);
    }
    CUNITTEST_RESULT();
}

static void test_formatter_std(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("formatter : std");
    {
        /* template */
        ec_formatter_t* formatter = ec_formatter_std_new();
        CUNITTEST_ASSERT_NE(formatter, NULL);

        test_formatter_plugin_item("", "");

        /* int */
        {
            test_formatter_plugin_item("1", "{int}", 1);
            test_formatter_plugin_item("12", "{int}", 12);
            test_formatter_plugin_item("123", "{int}", 123);
            test_formatter_plugin_item("-123", "{int}", -123);
            test_formatter_plugin_item("0", "{int}", 0);
            test_formatter_plugin_item("0", "{int}", -0);
        }

        /* unsigned int */
        {
            test_formatter_plugin_item("1", "{unsigned_int}", 1);
            test_formatter_plugin_item("12", "{unsigned_int}", 12);
            test_formatter_plugin_item("123", "{unsigned_int}", 123);
            test_formatter_plugin_item("0", "{unsigned_int}", 0);
        }

        /* size_t */
        {
            test_formatter_plugin_item("1", "{size_t}", 1);
            test_formatter_plugin_item("12", "{size_t}", 12);
            test_formatter_plugin_item("123", "{size_t}", 123);
            test_formatter_plugin_item("0", "{size_t}", 0);
        }

        /* s8, s16, s32 */
        {
            test_formatter_plugin_item("0", "{s8}", 0);
            test_formatter_plugin_item("123", "{s8}", 123);
            test_formatter_plugin_item("-123", "{s8}", -123);
            test_formatter_plugin_item("0", "{s16}", 0);
            test_formatter_plugin_item("123", "{s16}", 123);
            test_formatter_plugin_item("-123", "{s16}", -123);
            test_formatter_plugin_item("0", "{s32}", 0);
            test_formatter_plugin_item("123", "{s32}", 123);
            test_formatter_plugin_item("-123", "{s32}", -123);
        }

        /* u8, u16, u32 */
        {
            test_formatter_plugin_item("0", "{s8}", 0);
            test_formatter_plugin_item("123", "{s8}", 123);
            test_formatter_plugin_item("0", "{s16}", 0);
            test_formatter_plugin_item("123", "{s16}", 123);
            test_formatter_plugin_item("0", "{s32}", 0);
            test_formatter_plugin_item("123", "{s32}", 123);
        }

        /* c_str */
        {
            test_formatter_plugin_item("", "{c_str}", "");
            test_formatter_plugin_item("a", "{c_str}", "a");
            test_formatter_plugin_item("ab", "{c_str}", "ab");
        }

        /* char_t */
        {
            test_formatter_plugin_item("a", "{char_t}", (ec_char_t)'a');
            test_formatter_plugin_item("ab", "{char_t}{char_t}", (ec_char_t)'a',
                                       (ec_char_t)'b');
        }

        /* string */
        {
            ec_string *s, *s_a, *s_ab;
            s = ec_string_new();
            s_a = ec_string_new();
            s_ab = ec_string_new();

            ec_string_push_back(s_a, 'a');
            ec_string_push_back(s_ab, 'a');
            ec_string_push_back(s_ab, 'b');
            test_formatter_plugin_item("", "{string}", s);
            test_formatter_plugin_item("a", "{string}", s_a);
            test_formatter_plugin_item("ab", "{string}", s_ab);

            ec_delete(s);
            ec_delete(s_a);
            ec_delete(s_ab);
        }

        /* bytestring */
        {
            ec_bytestring *s, *s_a, *s_ab;
            s = ec_bytestring_new();
            s_a = ec_bytestring_new();
            s_ab = ec_bytestring_new();

            ec_bytestring_push_back(s_a, 'a');
            ec_bytestring_push_back(s_ab, 'a');
            ec_bytestring_push_back(s_ab, 'b');
            test_formatter_plugin_item("", "{bytestring}", s);
            test_formatter_plugin_item("a", "{bytestring}", s_a);
            test_formatter_plugin_item("ab", "{bytestring}", s_ab);

            ec_delete(s);
            ec_delete(s_a);
            ec_delete(s_ab);
        }

        ec_delete(formatter);
    }
    CUNITTEST_RESULT();
}

static void test_formatter_1(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("formatter");
    {
        /* new & delete */
        ec_formatter_t* formatter = ec_formatter_new();
        CUNITTEST_ASSERT_NE(formatter, NULL);

        ec_delete(formatter);
    }
    CUNITTEST_RESULT();
}

void test_formatter(void)
{
    test_formatter_template();
    test_formatter_plugin();
    test_formatter_std();
    test_formatter_1();
}
