#include "test_maybe.h"
#include "testfw.h"
#include "type_maybe.h"
#include <stdio.h>

static int test_maybe_match(maybe_int* v)
{
    int ret;
    ect_maybe_match_begin(maybe_int, v) ect_maybe_match_just(maybe_int, 2)
    {
        ret = 1;
    }
    ect_maybe_match_just(maybe_int, 3) { ret = 2; }
    ect_maybe_match_nothing(maybe_int) { ret = 3; }
    ect_maybe_match_otherwise(maybe_int) { ret = 4; }
    ect_maybe_match_end();
    return ret;
}

void test_maybe(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("Container : maybe");
    {
        /* new & delete */
        maybe_int* m = ect_maybe_new_just(maybe_int, 3);
        CUNITTEST_ASSERT_NE(m, NULL);
        ec_delete(m);
    }
    {
        /* new & delete */
        maybe_int* m = ect_maybe_new_nothing(maybe_int);
        CUNITTEST_ASSERT_NE(m, NULL);
        ec_delete(m);
    }
    {
        /* if */
        maybe_int* m_just = ect_maybe_new_just(maybe_int, 3);
        maybe_int* m_nothing = ect_maybe_new_nothing(maybe_int);
        CUNITTEST_ASSERT_EQ(ect_maybe_is_nothing(maybe_int, m_just), ec_false);
        CUNITTEST_ASSERT_EQ(ect_maybe_is_nothing(maybe_int, m_nothing),
                            ec_true);
        CUNITTEST_ASSERT_EQ(ect_maybe_is_just(maybe_int, m_just, 2), ec_false);
        CUNITTEST_ASSERT_EQ(ect_maybe_is_just(maybe_int, m_just, 3), ec_true);
        CUNITTEST_ASSERT_EQ(ect_maybe_is_just(maybe_int, m_just, 4), ec_false);
        CUNITTEST_ASSERT_EQ(ect_maybe_is_just(maybe_int, m_nothing, 3),
                            ec_false);
        ec_delete(m_just);
        ec_delete(m_nothing);
    }
    {
        /* match */
        maybe_int* m_just_2 = ect_maybe_new_just(maybe_int, 2);
        maybe_int* m_just_3 = ect_maybe_new_just(maybe_int, 3);
        maybe_int* m_just_4 = ect_maybe_new_just(maybe_int, 4);
        maybe_int* m_nothing = ect_maybe_new_nothing(maybe_int);

        CUNITTEST_ASSERT_EQ(test_maybe_match(m_just_2), 1);
        CUNITTEST_ASSERT_EQ(test_maybe_match(m_just_3), 2);
        CUNITTEST_ASSERT_EQ(test_maybe_match(m_nothing), 3);
        CUNITTEST_ASSERT_EQ(test_maybe_match(m_just_4), 4);

        ec_delete(m_just_2);
        ec_delete(m_just_3);
        ec_delete(m_just_4);
        ec_delete(m_nothing);
    }
    {
        /* unwrap */
        maybe_int* m_just_2 = ect_maybe_new_just(maybe_int, 2);
        maybe_int* m_just_3 = ect_maybe_new_just(maybe_int, 3);
        CUNITTEST_ASSERT_EQ(ect_maybe_unwrap(maybe_int, m_just_2), 2);
        CUNITTEST_ASSERT_NE(ect_maybe_unwrap(maybe_int, m_just_2), 3);
        CUNITTEST_ASSERT_EQ(ect_maybe_unwrap(maybe_int, m_just_3), 3);
        CUNITTEST_ASSERT_NE(ect_maybe_unwrap(maybe_int, m_just_3), 2);
        ec_delete(m_just_2);
        ec_delete(m_just_3);
    }
    CUNITTEST_RESULT();
}
