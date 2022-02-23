#include "test_vector.h"
#include "ec_algorithm.h"
#include "testfw.h"
#include "type_intvector.h"
#include <stdio.h>

void test_vector(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("Container : vector");
    {
        /* new & delete */
        ec_delete(ect_vector_new(vector_int_t));
    }
    {
        /* size */
        vector_int_t* vec = ect_vector_new(vector_int_t);
        CUNITTEST_ASSERT_EQ((int)ect_vector_size(vector_int_t, vec), 0);
        ec_delete(vec);
    }
    {
        /* push_back */
        vector_int_t* vec = ect_vector_new(vector_int_t);

        CUNITTEST_ASSERT_EQ((ec_bool)ect_vector_empty(vector_int_t, vec),
                            ec_true);

        ect_vector_push_back(vector_int_t, vec, 1);
        CUNITTEST_ASSERT_EQ((int)ect_vector_size(vector_int_t, vec), 1);
        CUNITTEST_ASSERT_EQ((ec_bool)ect_vector_empty(vector_int_t, vec),
                            ec_false);
        ect_vector_push_back(vector_int_t, vec, 2);
        CUNITTEST_ASSERT_EQ((int)ect_vector_size(vector_int_t, vec), 2);
        CUNITTEST_ASSERT_EQ((ec_bool)ect_vector_empty(vector_int_t, vec),
                            ec_false);
        ect_vector_push_back(vector_int_t, vec, 3);
        CUNITTEST_ASSERT_EQ((int)ect_vector_size(vector_int_t, vec), 3);
        CUNITTEST_ASSERT_EQ((ec_bool)ect_vector_empty(vector_int_t, vec),
                            ec_false);

        ect_vector_clear(vector_int_t, vec);
        CUNITTEST_ASSERT_EQ((int)ect_vector_size(vector_int_t, vec), 0);
        CUNITTEST_ASSERT_EQ((ec_bool)ect_vector_empty(vector_int_t, vec),
                            ec_true);

        ec_delete(vec);
    }
    {
        /* iterator & reverse_iterator */
        int expected_array_bound[] = {0, 1, 2, 3, 4, 0}, *p;
        int* expected_array = expected_array_bound + 1;
        vector_int_t* vec = ect_vector_new(vector_int_t);
        ect_iterator(vector_int_t) item1;
        ect_reverse_iterator(vector_int_t) item2;

        /* 0 element */
        CUNITTEST_ASSERT_NOT_REACH_SET(nr0);
        ect_for(vector_int_t, vec, item1)
        {
            CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
        }
        ect_for_reverse(vector_int_t, vec, item2)
        {
            CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
        }
        CUNITTEST_ASSERT_NOT_REACH_TEST(nr0);

        /* 1 element */
        {
            ect_vector_push_back(vector_int_t, vec, 1);

            p = expected_array;
            ect_for(vector_int_t, vec, item1)
            {
                CUNITTEST_ASSERT_EQ(*((int*)ec_deref(item1)), *p);
                p++;
            }
            p = expected_array;
            ect_for_reverse(vector_int_t, vec, item2)
            {
                CUNITTEST_ASSERT_EQ(*((int*)ec_deref(item2)), *p);
                p--;
            }
            ect_vector_clear(vector_int_t, vec);
        }

        /* 2 elements */
        {
            ect_vector_push_back(vector_int_t, vec, 1);
            ect_vector_push_back(vector_int_t, vec, 2);

            p = expected_array;
            ect_for(vector_int_t, vec, item1)
            {
                CUNITTEST_ASSERT_EQ(*((int*)ec_deref(item1)), *p);
                p++;
            }
            p = expected_array + 1;
            ect_for_reverse(vector_int_t, vec, item2)
            {
                CUNITTEST_ASSERT_EQ(*((int*)ec_deref(item2)), *p);
                p--;
            }
            ect_vector_clear(vector_int_t, vec);
        }

        /* 3 elements */
        {
            ect_vector_push_back(vector_int_t, vec, 1);
            ect_vector_push_back(vector_int_t, vec, 2);
            ect_vector_push_back(vector_int_t, vec, 3);

            p = expected_array;
            ect_for(vector_int_t, vec, item1)
            {
                CUNITTEST_ASSERT_EQ(*((int*)ec_deref(item1)), *p);
                p++;
            }
            p = expected_array + 2;
            ect_for_reverse(vector_int_t, vec, item2)
            {
                CUNITTEST_ASSERT_EQ(*((int*)ec_deref(item2)), *p);
                p--;
            }
            ect_vector_clear(vector_int_t, vec);
        }

        ec_delete(vec);
    }

    CUNITTEST_RESULT();
}
