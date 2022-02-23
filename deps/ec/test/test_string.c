#include "test_string.h"
#include "ec_algorithm.h"
#include "ec_bytestring.h"
#include "testfw.h"
#include <stdio.h>

void test_string(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("Container : bytestring");

    {
        /* new & delete */
        ec_delete(ec_bytestring_new());
    }
    {
        /* size & length */
        ec_bytestring* s = ec_bytestring_new();
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_size(s), 0);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_length(s), 0);
        ec_delete(s);
    }
    {
        /* push_back & clear & empty */
        ec_bytestring* s = ec_bytestring_new();

        CUNITTEST_ASSERT_EQ((int)ec_bytestring_size(s), 0);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_length(s), 0);
        CUNITTEST_ASSERT_EQ((ec_bool)ec_bytestring_empty(s), ec_true);

        ec_bytestring_push_back(s, 'a');
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_size(s), 1);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_length(s), 1);
        CUNITTEST_ASSERT_EQ((ec_bool)ec_bytestring_empty(s), ec_false);
        CUNITTEST_ASSERT_EQ(ec_bytestring_match_c_str(s, "a"), ec_true);
        ec_bytestring_push_back(s, 'b');
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_size(s), 2);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_length(s), 2);
        CUNITTEST_ASSERT_EQ((ec_bool)ec_bytestring_empty(s), ec_false);
        CUNITTEST_ASSERT_EQ(ec_bytestring_match_c_str(s, "ab"), ec_true);
        ec_bytestring_push_back(s, 'c');
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_size(s), 3);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_length(s), 3);
        CUNITTEST_ASSERT_EQ((ec_bool)ec_bytestring_empty(s), ec_false);
        CUNITTEST_ASSERT_EQ(ec_bytestring_match_c_str(s, "abc"), ec_true);

        ec_bytestring_clear(s);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_size(s), 0);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_length(s), 0);
        CUNITTEST_ASSERT_EQ((ec_bool)ec_bytestring_empty(s), ec_true);

        ec_delete(s);
    }
    {
        /* push_front */
        ec_bytestring* s = ec_bytestring_new();

        CUNITTEST_ASSERT_EQ((int)ec_bytestring_size(s), 0);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_length(s), 0);
        CUNITTEST_ASSERT_EQ((ec_bool)ec_bytestring_empty(s), ec_true);

        ec_bytestring_push_front(s, 'a');
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_size(s), 1);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_length(s), 1);
        CUNITTEST_ASSERT_EQ((ec_bool)ec_bytestring_empty(s), ec_false);
        CUNITTEST_ASSERT_EQ(ec_bytestring_match_c_str(s, "a"), ec_true);
        ec_bytestring_push_front(s, 'b');
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_size(s), 2);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_length(s), 2);
        CUNITTEST_ASSERT_EQ((ec_bool)ec_bytestring_empty(s), ec_false);
        CUNITTEST_ASSERT_EQ(ec_bytestring_match_c_str(s, "ba"), ec_true);
        ec_bytestring_push_front(s, 'c');
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_size(s), 3);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_length(s), 3);
        CUNITTEST_ASSERT_EQ((ec_bool)ec_bytestring_empty(s), ec_false);
        CUNITTEST_ASSERT_EQ(ec_bytestring_match_c_str(s, "cba"), ec_true);

        ec_bytestring_clear(s);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_size(s), 0);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_length(s), 0);
        CUNITTEST_ASSERT_EQ((ec_bool)ec_bytestring_empty(s), ec_true);

        ec_delete(s);
    }
    {
        /* at */
        ec_bytestring* s = ec_bytestring_new();

        ec_bytestring_push_back(s, 'a');
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_at(s, 0), 'a');
        ec_bytestring_push_back(s, 'b');
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_at(s, 0), 'a');
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_at(s, 1), 'b');
        ec_bytestring_push_back(s, 'c');
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_at(s, 0), 'a');
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_at(s, 1), 'b');
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_at(s, 2), 'c');

        ec_delete(s);
    }
    {
        /* iterator & reverse_iterator */
        char expected_array[] = "abc", *p;
        ec_bytestring* s = ec_bytestring_new();
        ect_iterator(ec_bytestring) item1;
        ect_reverse_iterator(ec_bytestring) item2;

        /* 0 element */
        {
            CUNITTEST_ASSERT_NOT_REACH_SET(nr0);
            ect_for(ec_bytestring, s, item1)
            {
                CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
            }
            ect_for_reverse(ec_bytestring, s, item2)
            {
                CUNITTEST_ASSERT_NOT_REACH_HIT(nr0);
            }
            CUNITTEST_ASSERT_NOT_REACH_TEST(nr0);
        }

        /* 1 element */
        {
            ec_bytestring_push_back(s, 'a');

            p = expected_array;
            ect_for(ec_bytestring, s, item1)
            {
                CUNITTEST_ASSERT_EQ(*((char*)ec_deref(item1)), *p);
                p++;
            }
            p = expected_array;
            ect_for_reverse(ec_bytestring, s, item2)
            {
                CUNITTEST_ASSERT_EQ(*((char*)ec_deref(item2)), *p);
                p--;
            }
            ec_bytestring_clear(s);
        }

        /* 2 elements */
        {
            ec_bytestring_push_back(s, 'a');
            ec_bytestring_push_back(s, 'b');

            p = expected_array;
            ect_for(ec_bytestring, s, item1)
            {
                CUNITTEST_ASSERT_EQ(*((char*)ec_deref(item1)), *p);
                p++;
            }
            p = expected_array + 1;
            ect_for_reverse(ec_bytestring, s, item2)
            {
                CUNITTEST_ASSERT_EQ(*((char*)ec_deref(item2)), *p);
                p--;
            }
            ec_bytestring_clear(s);
        }

        /* 3 elements */
        {
            ec_bytestring_push_back(s, 'a');
            ec_bytestring_push_back(s, 'b');
            ec_bytestring_push_back(s, 'c');

            p = expected_array;
            ect_for(ec_bytestring, s, item1)
            {
                CUNITTEST_ASSERT_EQ(*((char*)ec_deref(item1)), *p);
                p++;
            }
            p = expected_array + 2;
            ect_for_reverse(ec_bytestring, s, item2)
            {
                CUNITTEST_ASSERT_EQ(*((char*)ec_deref(item2)), *p);
                p--;
            }
            ec_bytestring_clear(s);
        }

        ec_delete(s);
    }
    {
        /* c_str */
        ec_bytestring* s = ec_bytestring_new();

        ec_bytestring_c_str(s);
        CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s), "");
        ec_bytestring_push_back(s, 'a');
        CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s), "a");
        ec_bytestring_push_back(s, 'b');
        CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s), "ab");
        ec_bytestring_push_back(s, 'c');
        CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s), "abc");

        ec_delete(s);
    }
    {
        /* assign */
        ec_bytestring* s = ec_bytestring_new();

        ec_delete(s);
    }
    {
        /* append */
        ec_bytestring* s = ec_bytestring_new();
        ec_bytestring* s2 = ec_bytestring_new();

        ec_bytestring_assign_c_str(s, "ab");
        CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s), "ab");
        ec_bytestring_append(s, s2);
        CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s), "ab");

        ec_bytestring_assign_c_str(s2, "cd");
        CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s2), "cd");
        ec_bytestring_append(s, s2);
        CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s), "abcd");

        ec_delete(s);
        ec_delete(s2);
    }
    {
        /* append_c_str */
        ec_bytestring* s = ec_bytestring_new();

        CUNITTEST_ASSERT_EQ((int)ec_bytestring_size(s), 0);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_length(s), 0);
        CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s), "");

        ec_bytestring_append_c_str(s, "a");

        CUNITTEST_ASSERT_EQ((int)ec_bytestring_size(s), 1);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_length(s), 1);
        CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s), "a");

        ec_bytestring_append_c_str(s, "bc");

        CUNITTEST_ASSERT_EQ((int)ec_bytestring_size(s), 3);
        CUNITTEST_ASSERT_EQ((int)ec_bytestring_length(s), 3);
        CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s), "abc");

        ec_delete(s);
    }
    {
        /* find */
        ec_bytestring* s = ec_bytestring_new();

        ec_bytestring_assign_c_str(s, "abc");
        CUNITTEST_ASSERT_EQ(ec_bytestring_find(s, 'a', 0), 0);
        CUNITTEST_ASSERT_EQ(ec_bytestring_find(s, 'a', 1), ec_bytestring_npos);
        CUNITTEST_ASSERT_EQ(ec_bytestring_find(s, 'a', 2), ec_bytestring_npos);
        CUNITTEST_ASSERT_EQ(ec_bytestring_find(s, 'b', 0), 1);
        CUNITTEST_ASSERT_EQ(ec_bytestring_find(s, 'b', 1), 1);
        CUNITTEST_ASSERT_EQ(ec_bytestring_find(s, 'b', 2), ec_bytestring_npos);
        CUNITTEST_ASSERT_EQ(ec_bytestring_find(s, 'c', 0), 2);
        CUNITTEST_ASSERT_EQ(ec_bytestring_find(s, 'c', 1), 2);
        CUNITTEST_ASSERT_EQ(ec_bytestring_find(s, 'c', 2), 2);

        ec_delete(s);
    }
    {
        /* rfind */
        ec_bytestring* s = ec_bytestring_new();

        ec_bytestring_assign_c_str(s, "abc");
        CUNITTEST_ASSERT_EQ(ec_bytestring_rfind(s, 'c', 2), 2);
        CUNITTEST_ASSERT_EQ(ec_bytestring_rfind(s, 'c', 1), ec_bytestring_npos);
        CUNITTEST_ASSERT_EQ(ec_bytestring_rfind(s, 'c', 0), ec_bytestring_npos);
        CUNITTEST_ASSERT_EQ(ec_bytestring_rfind(s, 'b', 2), 1);
        CUNITTEST_ASSERT_EQ(ec_bytestring_rfind(s, 'b', 1), 1);
        CUNITTEST_ASSERT_EQ(ec_bytestring_rfind(s, 'b', 0), ec_bytestring_npos);
        CUNITTEST_ASSERT_EQ(ec_bytestring_rfind(s, 'a', 2), 0);
        CUNITTEST_ASSERT_EQ(ec_bytestring_rfind(s, 'a', 1), 0);
        CUNITTEST_ASSERT_EQ(ec_bytestring_rfind(s, 'a', 0), 0);

        ec_delete(s);
    }
    {
        /* substr */
        ec_bytestring* s = ec_bytestring_new();

        ec_bytestring_assign_c_str(s, "abc");
        {
            ec_bytestring* s1;
            s1 = ec_bytestring_substr(s, 0, 0);
            CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s1), "");
            ec_delete(s1);
            s1 = ec_bytestring_substr(s, 0, 1);
            CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s1), "a");
            ec_delete(s1);
            s1 = ec_bytestring_substr(s, 0, 2);
            CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s1), "ab");
            ec_delete(s1);
            s1 = ec_bytestring_substr(s, 0, 3);
            CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s1), "abc");
            ec_delete(s1);
            s1 = ec_bytestring_substr(s, 1, 0);
            CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s1), "");
            ec_delete(s1);
            s1 = ec_bytestring_substr(s, 1, 1);
            CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s1), "b");
            ec_delete(s1);
            s1 = ec_bytestring_substr(s, 1, 2);
            CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s1), "bc");
            ec_delete(s1);
            s1 = ec_bytestring_substr(s, 1, 3);
            CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s1), "bc");
            ec_delete(s1);
            s1 = ec_bytestring_substr(s, 2, 0);
            CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s1), "");
            ec_delete(s1);
            s1 = ec_bytestring_substr(s, 2, 1);
            CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s1), "c");
            ec_delete(s1);
            s1 = ec_bytestring_substr(s, 2, 2);
            CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s1), "c");
            ec_delete(s1);
            s1 = ec_bytestring_substr(s, 2, 3);
            CUNITTEST_ASSERT_STREQ(ec_bytestring_c_str(s1), "c");
            ec_delete(s1);
        }

        ec_delete(s);
    }

    CUNITTEST_RESULT();
}
