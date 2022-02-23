#include "test_bitset.h"
#include "ec_algorithm.h"
#include "ec_bitset.h"
#include "testfw.h"
#include <stdio.h>

void test_bitset(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("Container : bitset");

    {
        /* new & delete */
        ec_delete(ec_bitset_new(0));
    }
    {
        /* new & delete */
        ec_bitset* bs;

        bs = ec_bitset_new(1);
        CUNITTEST_ASSERT_EQ((int)ec_bitset_size(bs), 1);
        ec_delete(bs);

        bs = ec_bitset_new(2);
        CUNITTEST_ASSERT_EQ((int)ec_bitset_size(bs), 2);
        ec_delete(bs);

        bs = ec_bitset_new(32);
        CUNITTEST_ASSERT_EQ((int)ec_bitset_size(bs), 32);
        ec_delete(bs);
    }
    {
        /* resetall, setall */
        ec_bitset* bs;

        bs = ec_bitset_new(32);
        ec_bitset_setall(bs);
        CUNITTEST_ASSERT_EQ((int)ec_bitset_count(bs), 32);
        ec_bitset_resetall(bs);
        CUNITTEST_ASSERT_EQ((int)ec_bitset_count(bs), 0);
        ec_bitset_setall(bs);
        CUNITTEST_ASSERT_EQ((int)ec_bitset_count(bs), 32);
        ec_delete(bs);
    }

    CUNITTEST_RESULT();
}
