#include "test_list.h"
#include "ec_algorithm.h"
#include "testfw.h"
#include "type_intlist.h"
#include <stdio.h>

void test_list(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("Container : list");
    {
        /* new & delete */
        ec_delete(ect_list_new(list_int_t));
    }
    {
        /* size */
        list_int_t* lst = ect_list_new(list_int_t);
        CUNITTEST_ASSERT_EQ((int)ect_list_size(list_int_t, lst), 0);
        ec_delete(lst);
    }
    {
        /* push_back */
        list_int_t* lst = ect_list_new(list_int_t);

        ect_list_push_back(list_int_t, lst, 1);
        CUNITTEST_ASSERT_EQ((int)ect_list_size(list_int_t, lst), 1);
        ect_list_push_back(list_int_t, lst, 2);
        CUNITTEST_ASSERT_EQ((int)ect_list_size(list_int_t, lst), 2);
        ect_list_push_back(list_int_t, lst, 3);
        CUNITTEST_ASSERT_EQ((int)ect_list_size(list_int_t, lst), 3);

        ect_list_clear(list_int_t, lst);
        CUNITTEST_ASSERT_EQ((int)ect_list_size(list_int_t, lst), 0);

        ec_delete(lst);
    }
    {
        /* push_front & pop_front */
        list_int_t* lst = ect_list_new(list_int_t);

        ect_list_push_front(list_int_t, lst, 1);
        CUNITTEST_ASSERT_EQ((int)ect_list_size(list_int_t, lst), 1);
        ect_list_push_front(list_int_t, lst, 2);
        CUNITTEST_ASSERT_EQ((int)ect_list_size(list_int_t, lst), 2);
        ect_list_push_front(list_int_t, lst, 3);
        CUNITTEST_ASSERT_EQ((int)ect_list_size(list_int_t, lst), 3);
        ect_list_pop_front(list_int_t, lst);
        CUNITTEST_ASSERT_EQ((int)ect_list_size(list_int_t, lst), 2);
        ect_list_pop_front(list_int_t, lst);
        CUNITTEST_ASSERT_EQ((int)ect_list_size(list_int_t, lst), 1);
        ect_list_pop_front(list_int_t, lst);
        CUNITTEST_ASSERT_EQ((int)ect_list_size(list_int_t, lst), 0);

        ect_list_clear(list_int_t, lst);
        CUNITTEST_ASSERT_EQ((int)ect_list_size(list_int_t, lst), 0);

        ec_delete(lst);
    }
    {
        /* iterator & reverse_iterator */
        int expected_array_bound[] = {0, 1, 2, 3, 0}, *p;
        int* expected_array = expected_array_bound + 1;
        list_int_t* lst = ect_list_new(list_int_t);
        ect_iterator(list_int_t) item1;
        ect_reverse_iterator(list_int_t) item2;

        /* 0 element */
        {
            CUNITTEST_ASSERT_NOT_REACH_SET(nr0);
            ect_for(list_int_t, lst, item1)
            {
                CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
            }
            ect_for_reverse(list_int_t, lst, item2)
            {
                CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
            }
            CUNITTEST_ASSERT_NOT_REACH_TEST(nr0);
        }

        /* 1 element */
        {
            ect_list_push_back(list_int_t, lst, 1);

            p = expected_array;
            ect_for(list_int_t, lst, item1)
            {
                CUNITTEST_ASSERT_EQ(ect_deref(int, item1), *p);
                p++;
            }
            p = expected_array;
            ect_for_reverse(list_int_t, lst, item2)
            {
                CUNITTEST_ASSERT_EQ(ect_deref(int, item2), *p);
                p--;
            }
            ect_list_clear(list_int_t, lst);
        }

        /* 2 elements */
        {
            ect_list_push_back(list_int_t, lst, 1);
            ect_list_push_back(list_int_t, lst, 2);

            p = expected_array;
            ect_for(list_int_t, lst, item1)
            {
                CUNITTEST_ASSERT_EQ(ect_deref(int, item1), *p);
                p++;
            }
            p = expected_array + 1;
            ect_for_reverse(list_int_t, lst, item2)
            {
                CUNITTEST_ASSERT_EQ(ect_deref(int, item2), *p);
                p--;
            }
            ect_list_clear(list_int_t, lst);
        }

        /* 3 elements */
        {
            ect_list_push_back(list_int_t, lst, 1);
            ect_list_push_back(list_int_t, lst, 2);
            ect_list_push_back(list_int_t, lst, 3);

            p = expected_array;
            ect_for(list_int_t, lst, item1)
            {
                CUNITTEST_ASSERT_EQ(ect_deref(int, item1), *p);
                p++;
            }
            p = expected_array + 2;
            ect_for_reverse(list_int_t, lst, item2)
            {
                CUNITTEST_ASSERT_EQ(ect_deref(int, item2), *p);
                p--;
            }
            ect_list_clear(list_int_t, lst);
        }

        ec_delete(lst);
    }

    CUNITTEST_RESULT();
}
