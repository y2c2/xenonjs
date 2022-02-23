#include "test_set.h"
#include "ec_algorithm.h"
#include "testfw.h"
#include "type_intset.h"
#include "type_stringset.h"
#include <stdio.h>

void test_set(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("Container : set");
    {
        /* new & delete (int) */
        ec_delete(ect_set_new(set_int_t));
    }
    {
        /* new & delete (string) */
        ec_delete(ect_set_new(set_string_t));
    }
    {
        /* size (int) */
        set_int_t* st = ect_set_new(set_int_t);
        CUNITTEST_ASSERT_EQ((int)ect_set_size(set_int_t, st), 0);
        ec_delete(st);
    }
    {
        /* size (string) */
        set_string_t* st = ect_set_new(set_string_t);
        CUNITTEST_ASSERT_EQ((int)ect_set_size(set_string_t, st), 0);
        ec_delete(st);
    }
    {
        /* insert (int) */
        set_int_t* st = ect_set_new(set_int_t);
        CUNITTEST_ASSERT_EQ((int)ect_set_size(set_int_t, st), 0);
        ect_set_insert(set_int_t, st, 1);
        CUNITTEST_ASSERT_EQ((int)ect_set_size(set_int_t, st), 1);
        ect_set_insert(set_int_t, st, 2);
        CUNITTEST_ASSERT_EQ((int)ect_set_size(set_int_t, st), 2);
        ect_set_insert(set_int_t, st, 3);
        CUNITTEST_ASSERT_EQ((int)ect_set_size(set_int_t, st), 3);
        ec_delete(st);
    }
    {
        /* insert (string) */
        set_string_t* st = ect_set_new(set_string_t);
        CUNITTEST_ASSERT_EQ((int)ect_set_size(set_string_t, st), 0);
        ect_set_insert(set_string_t, st, ec_string_new_assign_c_str("1"));
        CUNITTEST_ASSERT_EQ((int)ect_set_size(set_string_t, st), 1);
        ect_set_insert(set_string_t, st, ec_string_new_assign_c_str("2"));
        CUNITTEST_ASSERT_EQ((int)ect_set_size(set_string_t, st), 2);
        ect_set_insert(set_string_t, st, ec_string_new_assign_c_str("3"));
        CUNITTEST_ASSERT_EQ((int)ect_set_size(set_string_t, st), 3);
        ec_delete(st);
    }
    {
        /* insert, erase, iterator & reverse_iterator */
        int expected_array_bound[] = {0, 1, 2, 3, 0}, *p;
        int* expected_array = expected_array_bound + 1;
        set_int_t* st = ect_set_new(set_int_t);
        ect_iterator(set_int_t) item1;

        /* 0 element */
        CUNITTEST_ASSERT_NOT_REACH_SET(nr0);
        ect_for(set_int_t, st, item1) { CUNITTEST_ASSERT_NOT_REACH_HIT(nr0); }
        CUNITTEST_ASSERT_NOT_REACH_TEST(nr0);

        /* 1 element */
        ect_set_insert(set_int_t, st, 1);
        p = expected_array;
        ect_for(set_int_t, st, item1)
        {
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref(item1)), *p);
            p++;
        }

        /* 2 elements */
        ect_set_insert(set_int_t, st, 2);
        p = expected_array;
        ect_for(set_int_t, st, item1)
        {
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref(item1)), *p);
            p++;
        }

        /* 3 elements */
        ect_set_insert(set_int_t, st, 3);
        p = expected_array;
        ect_for(set_int_t, st, item1)
        {
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref(item1)), *p);
            p++;
        }

        /* 2 elements */
        ect_set_erase(set_int_t, st, 3);
        CUNITTEST_ASSERT_EQ((int)ect_set_size(set_int_t, st), 2);
        p = expected_array;
        ect_for(set_int_t, st, item1)
        {
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref(item1)), *p);
            p++;
        }

        /* 1 element */
        ect_set_erase(set_int_t, st, 2);
        CUNITTEST_ASSERT_EQ((int)ect_set_size(set_int_t, st), 1);
        p = expected_array;
        ect_for(set_int_t, st, item1)
        {
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref(item1)), *p);
            p++;
        }

        /* 0 element */
        ect_set_erase(set_int_t, st, 1);
        CUNITTEST_ASSERT_EQ((int)ect_set_size(set_int_t, st), 0);
        CUNITTEST_ASSERT_NOT_REACH_SET(nr1);
        ect_for(set_int_t, st, item1) { CUNITTEST_ASSERT_NOT_REACH_HIT(nr1); }
        CUNITTEST_ASSERT_NOT_REACH_TEST(nr1);

        ec_delete(st);
    }

    goto skip;
    {
        /* clear */
        int expected_array_bound[] = {0, 1, 2, 3, 0}, *p;
        int* expected_array = expected_array_bound + 1;
        set_int_t* st = ect_set_new(set_int_t);
        ect_iterator(set_int_t) item1;

        ect_set_insert(set_int_t, st, 1);
        ect_set_insert(set_int_t, st, 2);
        ect_set_insert(set_int_t, st, 3);
        p = expected_array;
        ect_for(set_int_t, st, item1)
        {
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref(item1)), *p);
            p++;
        }

        ect_set_clear(set_int_t, st);

        CUNITTEST_ASSERT_NOT_REACH_SET(nr1);
        ect_for(set_int_t, st, item1) { CUNITTEST_ASSERT_NOT_REACH_HIT(nr1); }
        CUNITTEST_ASSERT_NOT_REACH_TEST(nr1);

        ec_delete(st);
    }
    {
        /* count */
        set_int_t* st = ect_set_new(set_int_t);

        ect_set_insert(set_int_t, st, 1);
        ect_set_insert(set_int_t, st, 2);
        ect_set_insert(set_int_t, st, 3);

        CUNITTEST_ASSERT_EQ(ect_set_count(set_int_t, st, 0), 0);
        CUNITTEST_ASSERT_EQ(ect_set_count(set_int_t, st, 1), 1);
        CUNITTEST_ASSERT_EQ(ect_set_count(set_int_t, st, 2), 1);
        CUNITTEST_ASSERT_EQ(ect_set_count(set_int_t, st, 3), 1);
        CUNITTEST_ASSERT_EQ(ect_set_count(set_int_t, st, 4), 0);

        ec_delete(st);
    }

skip:
    CUNITTEST_RESULT();
}
