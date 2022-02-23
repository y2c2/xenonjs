/* Test Framework
 * Copyright(c) 2017 y2c2 */

#ifndef TESTFW_H
#define TESTFW_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>
#include <string.h>

    struct cunittest
    {
        unsigned int passed;
        unsigned int total;
    };

    int cunittest_printf(const char* format, ...);
    void cunittest_result(struct cunittest* cu_p);

/* Global variable hold */
#define CUNITTEST_HOLD() static struct cunittest cu;

#define CUNITTEST_DECLARE(cu) struct cunittest* _this_cu = cu;

/* Initialize Unit Test Framework */
#define CUNITTEST_INIT()                                                       \
    do                                                                         \
    {                                                                          \
        _this_cu->passed = 0;                                                  \
        _this_cu->total = 0;                                                   \
    } while (0)

#define CUNITTEST_INIT_WITH_TITLE(title)                                       \
    do                                                                         \
    {                                                                          \
        _this_cu->passed = 0;                                                  \
        _this_cu->total = 0;                                                   \
        cunittest_printf("Test: %s\n", title);                                 \
    } while (0)

#define CUNITTEST_ASSERT(expr)                                                 \
    do                                                                         \
    {                                                                          \
        _this_cu->total++;                                                     \
        if (expr)                                                              \
        {                                                                      \
            _this_cu->passed++;                                                \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            cunittest_printf("Assertion Failed at %s:%u\n", __FILE__,          \
                             __LINE__);                                        \
        }                                                                      \
    } while (0)

/* Reach */
#define CUNITTEST_ASSERT_REACH() CUNITTEST_ASSERT(1)

/* Not reach */
#define CUNITTEST_ASSERT_NOT_REACH_SET(_name) int _name = 0;

#define CUNITTEST_ASSERT_NOT_REACH_HIT(_name)                                  \
    do                                                                         \
    {                                                                          \
        _name = 1;                                                             \
    } while (0)

#define CUNITTEST_ASSERT_NOT_REACH_TEST(_name) CUNITTEST_ASSERT(_name == 0)

#define CUNITTEST_ASSERT_EQ(expr1, expr2)                                      \
    do                                                                         \
    {                                                                          \
        _this_cu->total++;                                                     \
        if ((expr1) == (expr2))                                                \
        {                                                                      \
            _this_cu->passed++;                                                \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            cunittest_printf(                                                  \
                "Assertion Failed at %s:%u, expected %d, actual %d\n",         \
                __FILE__, __LINE__, expr2, expr1);                             \
        }                                                                      \
    } while (0)

#define CUNITTEST_ASSERT_STREQ_RAW(expr1, expr2, _file, _line)                 \
    do                                                                         \
    {                                                                          \
        _this_cu->total++;                                                     \
        if ((strlen(expr1) == strlen(expr2)) &&                                \
            (strncmp(expr1, expr2, strlen(expr2)) == 0))                       \
        {                                                                      \
            _this_cu->passed++;                                                \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            cunittest_printf(                                                  \
                "Assertion Failed at %s:%u, expected %s, actual %s\n", _file,  \
                _line, expr2, expr1);                                          \
        }                                                                      \
    } while (0)

#define CUNITTEST_ASSERT_STREQ(expr1, expr2)                                   \
    CUNITTEST_ASSERT_STREQ_RAW(expr1, expr2, __FILE__, __LINE__)

#define CUNITTEST_ASSERT_NE(expr1, expr2)                                      \
    do                                                                         \
    {                                                                          \
        _this_cu->total++;                                                     \
        if ((expr1) != (expr2))                                                \
        {                                                                      \
            _this_cu->passed++;                                                \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            cunittest_printf(                                                  \
                "Assertion Failed at %s:%u, expected %d, actual %d\n",         \
                __FILE__, __LINE__, expr2, expr1);                             \
        }                                                                      \
    } while (0)

#define CUNITTEST_RESULT()                                                     \
    do                                                                         \
    {                                                                          \
        cunittest_result(&cu);                                                 \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif
