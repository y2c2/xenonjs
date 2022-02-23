#include "test_map.h"
#include "ec_algorithm.h"
#include "testfw.h"
#include "type_intmap.h"
#include "type_stringmap.h"
#include <stdio.h>

void test_map(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("Container : map");
    {
        /* new & delete (int) */
        ec_delete(ect_map_new(map_int_t));
    }
    {
        /* new & delete (string) */
        ec_delete(ect_map_new(map_string_string_t));
    }
    {
        /* size (int) */
        map_int_t* mp = ect_map_new(map_int_t);
        CUNITTEST_ASSERT_EQ((int)ect_map_size(map_int_t, mp), 0);
        ec_delete(mp);
    }
    {
        /* size (string) */
        map_string_string_t* mp = ect_map_new(map_string_string_t);
        CUNITTEST_ASSERT_EQ((int)ect_map_size(map_string_string_t, mp), 0);
        ec_delete(mp);
    }
    {
        /* insert (int) */
        map_int_t* mp = ect_map_new(map_int_t);
        CUNITTEST_ASSERT_EQ((int)ect_map_size(map_int_t, mp), 0);
        ect_map_insert(map_int_t, mp, 1, -1);
        CUNITTEST_ASSERT_EQ((int)ect_map_size(map_int_t, mp), 1);
        ect_map_insert(map_int_t, mp, 2, -2);
        CUNITTEST_ASSERT_EQ((int)ect_map_size(map_int_t, mp), 2);
        ect_map_insert(map_int_t, mp, 3, -3);
        CUNITTEST_ASSERT_EQ((int)ect_map_size(map_int_t, mp), 3);
        ec_delete(mp);
    }
    {
        /* insert (string) */
        map_string_string_t* mp = ect_map_new(map_string_string_t);
        CUNITTEST_ASSERT_EQ((int)ect_map_size(map_string_string_t, mp), 0);
        ect_map_insert(map_string_string_t, mp, ec_string_new_assign_c_str("1"),
                       ec_string_new());
        CUNITTEST_ASSERT_EQ((int)ect_map_size(map_string_string_t, mp), 1);
        ect_map_insert(map_string_string_t, mp, ec_string_new_assign_c_str("2"),
                       ec_string_new());
        CUNITTEST_ASSERT_EQ((int)ect_map_size(map_string_string_t, mp), 2);
        ect_map_insert(map_string_string_t, mp, ec_string_new_assign_c_str("3"),
                       ec_string_new());
        CUNITTEST_ASSERT_EQ((int)ect_map_size(map_string_string_t, mp), 3);
        ec_delete(mp);
    }
    {
        /* insert, erase, iterator & reverse_iterator */
        int expected_array_bound[] = {0, 1, 2, 3, 0}, *p;
        int* expected_array = expected_array_bound + 1;
        map_int_t* mp = ect_map_new(map_int_t);
        ect_iterator(map_int_t) item1;

        /* 0 element */
        CUNITTEST_ASSERT_NOT_REACH_SET(nr0);
        ect_for(map_int_t, mp, item1) { CUNITTEST_ASSERT_NOT_REACH_HIT(nr0); }
        CUNITTEST_ASSERT_NOT_REACH_TEST(nr0);

        /* 1 element */
        ect_map_insert(map_int_t, mp, 1, -1);
        p = expected_array;
        ect_for(map_int_t, mp, item1)
        {
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref_key(item1)), *p);
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref_value(item1)), -(*p));
            p++;
        }

        /* 2 elements */
        ect_map_insert(map_int_t, mp, 2, -2);
        p = expected_array;
        ect_for(map_int_t, mp, item1)
        {
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref_key(item1)), *p);
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref_value(item1)), -(*p));
            p++;
        }

        /* 3 elements */
        ect_map_insert(map_int_t, mp, 3, -3);
        p = expected_array;
        ect_for(map_int_t, mp, item1)
        {
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref_key(item1)), *p);
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref_value(item1)), -(*p));
            p++;
        }

        ec_delete(mp);
    }
    {
        /* erase */
        int expected_array_bound[] = {0, 1, 2, 3, 0}, *p;
        int* expected_array = expected_array_bound + 1;
        map_int_t* mp = ect_map_new(map_int_t);
        ect_iterator(map_int_t) item1;

        ect_map_insert(map_int_t, mp, 1, -1);
        ect_map_insert(map_int_t, mp, 2, -2);
        ect_map_insert(map_int_t, mp, 3, -3);

        p = expected_array;
        ect_for(map_int_t, mp, item1)
        {
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref_key(item1)), *p);
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref_value(item1)), -(*p));
            p++;
        }

        ect_map_erase(map_int_t, mp, 3);
        p = expected_array;
        ect_for(map_int_t, mp, item1)
        {
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref_key(item1)), *p);
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref_value(item1)), -(*p));
            p++;
        }

        ect_map_erase(map_int_t, mp, 2);
        p = expected_array;
        ect_for(map_int_t, mp, item1)
        {
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref_key(item1)), *p);
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref_value(item1)), -(*p));
            p++;
        }

        ect_map_erase(map_int_t, mp, 1);
        CUNITTEST_ASSERT_NOT_REACH_SET(nr1);
        ect_for(map_int_t, mp, item1) { CUNITTEST_ASSERT_NOT_REACH_HIT(nr1); }
        CUNITTEST_ASSERT_NOT_REACH_TEST(nr1);

        ec_delete(mp);
    }
    {
        /* clear */
        int expected_array_bound[] = {0, 1, 2, 3, 0}, *p;
        int* expected_array = expected_array_bound + 1;
        map_int_t* mp = ect_map_new(map_int_t);
        ect_iterator(map_int_t) item1;

        ect_map_insert(map_int_t, mp, 1, -1);
        ect_map_insert(map_int_t, mp, 2, -2);
        ect_map_insert(map_int_t, mp, 3, -3);

        p = expected_array;
        ect_for(map_int_t, mp, item1)
        {
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref_key(item1)), *p);
            CUNITTEST_ASSERT_EQ(*((int*)ec_deref_value(item1)), -(*p));
            p++;
        }

        ect_map_clear(map_int_t, mp);

        CUNITTEST_ASSERT_NOT_REACH_SET(nr0);
        ect_for(map_int_t, mp, item1) { CUNITTEST_ASSERT_NOT_REACH_HIT(nr0); }
        CUNITTEST_ASSERT_NOT_REACH_TEST(nr0);

        ec_delete(mp);
    }
    {
        /* get */
        map_int_t* mp = ect_map_new(map_int_t);

        ect_map_insert(map_int_t, mp, 1, -1);
        ect_map_insert(map_int_t, mp, 2, -2);
        ect_map_insert(map_int_t, mp, 3, -3);
        CUNITTEST_ASSERT_EQ(ect_map_get(map_int_t, mp, 1), -1);

        ec_delete(mp);
    }

    CUNITTEST_RESULT();
}
