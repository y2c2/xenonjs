#include "test_exception.h"
#include "ec_exception.h"
#include "testfw.h"
#include <stdio.h>
#include <stdlib.h>

enum
{
    Exception,
    BooleanException,
    TrueException,
    FalseException,
    ExceptionCount,
};

static void test_exception_try_catch(struct cunittest* cu, ec_exception_t* expt)
{
    CUNITTEST_DECLARE(cu);

    /* Empty */
    {
        ec_try { CUNITTEST_ASSERT_REACH(); }
        CUNITTEST_ASSERT_REACH();
    }

    /* Exception raised */
    {
        CUNITTEST_ASSERT_NOT_REACH_SET(nr0);
        ec_try
        {
            CUNITTEST_ASSERT_REACH();
            ec_throw(expt, Exception);
            CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
        }
        ec_catch(expt, Exception) { CUNITTEST_ASSERT_REACH(); }
        ec_catch(expt, Exception) { CUNITTEST_ASSERT_NOT_REACH_HIT(nr0); }
        CUNITTEST_ASSERT_NOT_REACH_TEST(nr0);
    }

    /* Sub exception catching 1 */
    {
        CUNITTEST_ASSERT_NOT_REACH_SET(nr0);
        ec_try { ec_throw(expt, BooleanException); }
        ec_catch(expt, TrueException) { CUNITTEST_ASSERT_NOT_REACH_HIT(nr0); }
        ec_catch(expt, FalseException) { CUNITTEST_ASSERT_NOT_REACH_HIT(nr0); }
        ec_catch(expt, BooleanException) { CUNITTEST_ASSERT_REACH(); }
        CUNITTEST_ASSERT_NOT_REACH_TEST(nr0);
    }

    /* Sub exception catching 2 */
    {ec_try{ec_throw(expt, BooleanException);
}
ec_catch(expt, Exception) { CUNITTEST_ASSERT_REACH(); }
}

/* Sub exception catching 3 */
{
    CUNITTEST_ASSERT_NOT_REACH_SET(nr0);
    ec_try { ec_throw(expt, FalseException); }
    ec_catch(expt, TrueException) { CUNITTEST_ASSERT_NOT_REACH_HIT(nr0); }
    ec_catch(expt, FalseException) { CUNITTEST_ASSERT_REACH(); }
    CUNITTEST_ASSERT_NOT_REACH_TEST(nr0);

    ec_try { ec_throw(expt, FalseException); }
    ec_catch(expt, BooleanException) { CUNITTEST_ASSERT_REACH(); }

    ec_try { ec_throw(expt, FalseException); }
    ec_catch(expt, Exception) { CUNITTEST_ASSERT_REACH(); }
}
}

static void test_exception1(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("exception");

    ec_exception_t expt;

    ec_exception_init(&expt, ExceptionCount);
    ec_exception_define_root_item(&expt, "Exception", Exception);
    ec_exception_define_item(&expt, "BooleanException", BooleanException,
                             Exception);
    ec_exception_define_item(&expt, "TrueException", TrueException,
                             BooleanException);
    ec_exception_define_item(&expt, "FalseException", FalseException,
                             BooleanException);
    {
        test_exception_try_catch(&cu, &expt);
    }
    ec_exception_uninit(&expt);

    CUNITTEST_RESULT();
}

void test_exception(void)
{
    ec_exception_allocator_set_malloc(malloc);
    ec_exception_allocator_set_free(free);

    test_exception1();
}
