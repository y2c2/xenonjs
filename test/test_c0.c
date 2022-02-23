/* C0 Tests (Script)
 * Copyright(c) 2017 y2c2 */

#include <ec_encoding.h>
#include <ec_string.h>
#include <ec_algorithm.h>
#include "print_error.h"
#include "xjs.h"
#include "testfw.h"
#include "test_c0_serialize.h"
#include "test_c0.h"

static int _test_c0_item_ok( \
        struct cunittest *this_cu, \
        const char *expected_result, const char *encoded_s, \
        const char *filename, \
        int lineno)
{
    CUNITTEST_DECLARE(this_cu);
    ec_encoding_t enc;
    xjs_error err;
    ec_string *s = NULL;
    xjs_token_list_ref tokens = NULL;
    xjs_ast_program_ref program = NULL;
    xjs_cfg_ref cfg = NULL;
    char *result = NULL;

    CUNITTEST_ASSERT_NOT_REACH_SET(nr0);

    ec_encoding_utf8_init(&enc);
    xjs_error_init(&err);

    if (ec_encoding_decode(&enc, &s, (const ec_byte_t *)encoded_s, strlen(encoded_s)) != 0)
    {
        fprintf(stderr, "%s:%d: error: decode failed\n"
                "testcase: %s\n", filename, lineno, encoded_s);
        CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
        goto fail;
    }

    tokens = xjs_lexer_start(&err, s);
    if (tokens == NULL)
    {
        fprintf(stderr, "%s:%d: error: tokenize failed\n"
                "testcase: %s\n", \
                filename, lineno, \
                encoded_s);
        if (err.error_no != 0) { print_error(&err); }
        CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
        goto fail;
    }

    program = xjs_parser_start(&err, tokens, xjs_false);
    if (program == NULL)
    {
        fprintf(stderr, "%s:%d: error: parse failed\n"
                "testcase: %s\n", \
                filename, lineno, \
                encoded_s);
        if (err.error_no != 0) { print_error(&err); }
        CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
        goto fail;
    }

    cfg = xjs_c0_start(&err, program);
    if (cfg == NULL)
    {
        fprintf(stderr, "%s:%d: error: c0 failed\n"
                "testcase: %s\n", \
                filename, lineno, \
                encoded_s);
        if (err.error_no != 0) { print_error(&err); }
        CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
        goto fail;
    }

    result = xjs_cfg_serialize(cfg);
    if (result == NULL)
    {
        fprintf(stderr, "%s:%d: error: serialize failed\n"
                "testcase: %s\n", \
                filename, lineno, \
                encoded_s);
        CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
        goto fail;
    }
    if ((strlen(result) != strlen(expected_result)) || \
            (strncmp(result, expected_result, strlen(expected_result)) != 0))
    {
        fprintf(stderr, "%s:%d: error: serialized result does not match\n"
                "testcase: %s\n"
                "Expected:\n  %s\n"
                "Actual:\n  %s\n"
                "", \
                filename, lineno, 
                encoded_s, \
                expected_result, \
                result);
        CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
    }

    CUNITTEST_ASSERT_NOT_REACH_TEST(nr0);

fail:
    if (tokens != NULL) ec_delete(tokens);
    if (program != NULL) ec_delete(program);
    if (cfg != NULL) ec_delete(cfg);
    ec_delete(s);
    ec_free(result);
    return 0;
}

static void test_c0_simple(struct cunittest *this_cu, \
        const char *test_file)
{
    CUNITTEST_DECLARE(this_cu);

#define test_c0_item_ok(_this_cu, _expected_result, _source, _filename, _lineno) \
    _test_c0_item_ok(_this_cu, _expected_result, _source, _filename, _lineno)

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
            int cur_lineno = lineno;
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
                fprintf(stderr, "%s:%d: error: testset error, expected result missing\n", \
                        test_file, \
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
            test_c0_item_ok(CUNITTEST_THIS, expected, testcase, test_file, cur_lineno);
        }

        fclose(fp);
    }
}

void test_c0(const char *test_file)
{
    CUNITTEST_HOLD_AND_DECLARE();
    CUNITTEST_INIT_WITH_TITLE("c0");

    if (test_file == NULL)
    {
        fprintf(stderr, "error: c0 testcase missing\n");
        fflush(stderr);
        return;
    }
    test_c0_simple(CUNITTEST_THIS, test_file);

    CUNITTEST_RESULT();
}

