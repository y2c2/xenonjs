/* Lexer Tests
 * Copyright(c) 2017 y2c2 */

#include <ec_encoding.h>
#include <ec_string.h>
#include <ec_algorithm.h>
#include "xjs.h"
#include "test_lexer_serialize.h"
#include "testfw.h"
#include "test_lexer.h"

static int _test_lexer_item_ok( \
        struct cunittest *this_cu, \
        const char *filename, const xjs_size_t line, \
        const char *expected_result, const char *encoded_s)
{
    ec_encoding_t enc;
    xjs_error err;
    ec_string *s = NULL;
    xjs_token_list_ref tokens = NULL;
    char *result = NULL;

    ec_encoding_utf8_init(&enc);
    xjs_error_init(&err);

    CUNITTEST_DECLARE(this_cu);

    CUNITTEST_ASSERT_EQ(ec_encoding_decode(&enc, &s, (const ec_byte_t *)encoded_s, strlen(encoded_s)), 0);

    CUNITTEST_ASSERT_NE_RAW(tokens = xjs_lexer_start(&err, s), NULL, filename, line);
    if (tokens == NULL) { goto fail; }

    CUNITTEST_ASSERT_NE_RAW(result = xjs_token_list_serialize(tokens), NULL, filename, line);
    if (result == NULL) { goto fail; }
    CUNITTEST_ASSERT_STREQ_RAW(result, expected_result, filename, line);

fail:
    if (tokens != NULL) ec_delete(tokens);
    ec_delete(s);
    ec_free(result);
    return 0;
}

static void test_lexer_simple(struct cunittest *this_cu, const char *test_file)
{
    CUNITTEST_DECLARE(this_cu);

#define test_lexer_item_ok(_this_cu, _expected_result, _source) \
    _test_lexer_item_ok(_this_cu, __FILE__, __LINE__, _expected_result, _source)

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
            test_lexer_item_ok(CUNITTEST_THIS, expected, testcase);
        }

        fclose(fp);
    }
}

void test_lexer(const char *test_file)
{
    CUNITTEST_HOLD_AND_DECLARE();
    CUNITTEST_INIT_WITH_TITLE("Lexer");

    if (test_file == NULL)
    {
        fprintf(stderr, "error: lexer testcase missing\n");
        fflush(stderr);
        return;
    }
    test_lexer_simple(CUNITTEST_THIS, test_file);

    CUNITTEST_RESULT();
}

