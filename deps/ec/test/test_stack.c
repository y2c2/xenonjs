#include "test_stack.h"
#include "ec_algorithm.h"
#include "testfw.h"
#include "type_intstack.h"
#include <stdio.h>

void test_stack(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("Container : stack");
    {
        /* new & delete */
        ec_delete(ect_stack_new(stack_int_t));
    }
    {
        /* size */
        stack_int_t* stk = ect_stack_new(stack_int_t);
        CUNITTEST_ASSERT_EQ((int)ect_stack_size(stack_int_t, stk), 0);
        ec_delete(stk);
    }
    {
        /* push */
        stack_int_t* stk = ect_stack_new(stack_int_t);

        ect_stack_push(stack_int_t, stk, 1);
        CUNITTEST_ASSERT_EQ((int)ect_stack_size(stack_int_t, stk), 1);
        ect_stack_push(stack_int_t, stk, 2);
        CUNITTEST_ASSERT_EQ((int)ect_stack_size(stack_int_t, stk), 2);
        ect_stack_push(stack_int_t, stk, 3);
        CUNITTEST_ASSERT_EQ((int)ect_stack_size(stack_int_t, stk), 3);

        ect_stack_clear(stack_int_t, stk);
        CUNITTEST_ASSERT_EQ((int)ect_stack_size(stack_int_t, stk), 0);

        ec_delete(stk);
    }
    {
        /* top, pop */
        stack_int_t* stk = ect_stack_new(stack_int_t);
        ect_stack_push(stack_int_t, stk, 1);
        CUNITTEST_ASSERT_EQ(ect_stack_top(stack_int_t, stk), 1);
        ect_stack_push(stack_int_t, stk, 2);
        CUNITTEST_ASSERT_EQ(ect_stack_top(stack_int_t, stk), 2);
        ect_stack_push(stack_int_t, stk, 3);
        CUNITTEST_ASSERT_EQ(ect_stack_top(stack_int_t, stk), 3);
        ect_stack_pop(stack_int_t, stk);
        CUNITTEST_ASSERT_EQ(ect_stack_top(stack_int_t, stk), 2);
        ect_stack_pop(stack_int_t, stk);
        CUNITTEST_ASSERT_EQ(ect_stack_top(stack_int_t, stk), 1);

        ec_delete(stk);
    }
    CUNITTEST_RESULT();
}
