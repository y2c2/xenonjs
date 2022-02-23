/* Lexer Tests
 * Copyright(c) 2017 y2c2 */

#include <ec_encoding.h>
#include <ec_string.h>
#include <ec_algorithm.h>
#include "print_error.h"
#include "xjs.h"
#include "testfw.h"
#include "test_parser.h"

static int _test_parser_item_ok( \
        struct cunittest *this_cu, \
        const char *expected_result, const char *encoded_s, \
        xjs_bool is_module)
{
    CUNITTEST_DECLARE(this_cu);
    ec_encoding_t enc;
    xjs_error err;
    ec_string *s = NULL;
    xjs_token_list_ref tokens = NULL;
    xjs_ast_program_ref program = NULL;
    char *result = NULL;

    CUNITTEST_ASSERT_NOT_REACH_SET(nr0);

    ec_encoding_utf8_init(&enc);
    xjs_error_init(&err);

    if (ec_encoding_decode(&enc, &s, (const ec_byte_t *)encoded_s, strlen(encoded_s)) != 0)
    {
        fprintf(stderr, "testcase: %s\nerror: decode failed\n", encoded_s);
        CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
        goto fail;
    }

    tokens = xjs_lexer_start(&err, s);
    if (tokens == NULL)
    {
        fprintf(stderr, "testcase: %s\nerror: tokenize failed\n", encoded_s);
        if (err.error_no != 0) { print_error(&err); }
        CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
        goto fail;
    }

    program = xjs_parser_start(&err, tokens, is_module);
    if (program == NULL)
    {
        fprintf(stderr, "testcase: %s\nerror: parse failed\n", encoded_s);
        if (err.error_no != 0) { print_error(&err); }
        CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
        goto fail;
    }

    {
        ec_string *readable_ir_source = NULL;

        if (xjs_astprinter_start(&err, &readable_ir_source, program) != 0)
        {
            fprintf(stderr, "testcase: %s\nerror: serialize failed\n", encoded_s);
            CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
            goto fail;
        }

        {
            ec_size_t encoded_result_len;

            ec_encoding_t enc;
            ec_encoding_utf8_init(&enc);
            if (ec_encoding_encode(&enc, (ec_byte_t **)&result, &encoded_result_len, readable_ir_source) != 0)
            {
                fprintf(stderr, "testcase: %s\nerror: serialize failed\n", encoded_s);
                CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
                ec_delete(readable_ir_source);
                goto fail;
            }
            ec_delete(readable_ir_source);
        }
    }

    {
        int result_len = (int)strlen(result);
        int expected_result_len = (int)strlen(expected_result);
        if ((result_len != expected_result_len) || \
                (strncmp(result, expected_result, expected_result_len) != 0))
        {
            fprintf(stderr, "testcase: %s\nerror: serialized result does not match\n"
                    "Expected (%d bytes):\n  %s\n"
                    "Actual (%d bytes):\n  %s\n"
                    "", \
                    encoded_s, \
                    expected_result_len, \
                    expected_result, \
                    result_len, \
                    result);
            CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
        }
    }

    CUNITTEST_ASSERT_NOT_REACH_TEST(nr0);

fail:
    if (tokens != NULL) ec_delete(tokens);
    if (program != NULL) ec_delete(program);
    ec_delete(s);
    ec_free(result);
    return 0;
}


static void test_parser_simple(struct cunittest *this_cu, \
        const char *test_file, \
        const xjs_bool is_module)
{
    CUNITTEST_DECLARE(this_cu);

#define test_parser_item_ok(_this_cu, _expected_result, _source, _is_module) \
    _test_parser_item_ok(_this_cu, _expected_result, _source, _is_module)

#define TESTCASE_LINE_MAX 4096
#define EXPECTED_LINE_MAX 8192
    {
        FILE *fp = NULL;
        char testcase[TESTCASE_LINE_MAX + 1];
        char expected[EXPECTED_LINE_MAX + 1];
        int lineno = 1;

        if ((fp = fopen(test_file, "rb")) == NULL)
        {
            fprintf(stderr, "error: failed to open %s\n", test_file);
            fflush(stderr);
            return;
        }
        for (;;)
        {
            /* Read input */
            if (fgets(testcase, TESTCASE_LINE_MAX, fp) == NULL)
            { break; }
            /* Trim */
            if ((strlen(testcase) > 0) && \
                    testcase[strlen(testcase) - 1] == '\n')
            { testcase[strlen(testcase) - 1] = '\0'; }
            lineno++;

            /* Read expected result */
            if (fgets(expected, EXPECTED_LINE_MAX, fp) == NULL)
            {
                fprintf(stderr, "%d: error: testset error, expected result missing\n", \
                        lineno);
                fflush(stderr);
                fclose(fp);
                return;
            }
            /* Trim */
            if ((strlen(expected) > 0) && \
                    expected[strlen(expected) - 1] == '\n')
            { expected[strlen(expected) - 1] = '\0'; }
            lineno++;

            /* Perform testing */
            test_parser_item_ok(CUNITTEST_THIS, expected, testcase, is_module);
        }

        fclose(fp);
    }
}

void test_parse(const char *test_file)
{
    CUNITTEST_HOLD_AND_DECLARE();
    CUNITTEST_INIT_WITH_TITLE("Parse (Script)");

    if (test_file == NULL)
    {
        fprintf(stderr, "error: parse testcase missing\n");
        fflush(stderr);
        return;
    }
    test_parser_simple(CUNITTEST_THIS, test_file, xjs_false);

    CUNITTEST_RESULT();
}

void test_parsem(const char *test_file)
{
    CUNITTEST_HOLD_AND_DECLARE();
    CUNITTEST_INIT_WITH_TITLE("Parse (Module)");

    if (test_file == NULL)
    {
        fprintf(stderr, "error: parse testcase missing\n");
        fflush(stderr);
        return;
    }
    test_parser_simple(CUNITTEST_THIS, test_file, xjs_true);

    CUNITTEST_RESULT();
}

