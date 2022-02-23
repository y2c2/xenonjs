#include "test_matrix.h"
#include "ec_algorithm.h"
#include "testfw.h"
#include "type_intmatrix.h"
#include <stdio.h>

void test_matrix(void)
{
    CUNITTEST_HOLD();
    CUNITTEST_DECLARE(&cu);
    CUNITTEST_INIT_WITH_TITLE("Container : matrix");
    {
        /* new & delete */
        ec_delete(ect_matrix_new(matrix_int_t, 3, 2));
    }
    {
        /* width & height */
        matrix_int_t* m = ect_matrix_new(matrix_int_t, 3, 2);
        CUNITTEST_ASSERT_EQ((int)ect_matrix_width(matrix_int_t, m), 3);
        CUNITTEST_ASSERT_EQ((int)ect_matrix_height(matrix_int_t, m), 2);
        ec_delete(m);
    }
    {
        ec_size_t x, y;

        /* fill */
        matrix_int_t* m = ect_matrix_new(matrix_int_t, 3, 2);
        ect_matrix_fill(matrix_int_t, m, 0);
        for (x = 0; x != ect_matrix_width(matrix_int_t, m); x++)
        {
            for (y = 0; y != ect_matrix_height(matrix_int_t, m); y++)
            {
                CUNITTEST_ASSERT_EQ((int)ect_matrix_get(matrix_int_t, m, x, y),
                                    0);
            }
        }
        ect_matrix_fill(matrix_int_t, m, 1);
        for (x = 0; x != ect_matrix_width(matrix_int_t, m); x++)
        {
            for (y = 0; y != ect_matrix_height(matrix_int_t, m); y++)
            {
                CUNITTEST_ASSERT_EQ((int)ect_matrix_get(matrix_int_t, m, x, y),
                                    1);
            }
        }
        ec_delete(m);
    }
    {
        ec_size_t x, y;

        /* set, get */
        matrix_int_t* m = ect_matrix_new(matrix_int_t, 3, 2);
        for (x = 0; x != ect_matrix_width(matrix_int_t, m); x++)
        {
            for (y = 0; y != ect_matrix_height(matrix_int_t, m); y++)
            {
                ect_matrix_set(matrix_int_t, m, x, y, x * y);
            }
        }
        for (x = 0; x != ect_matrix_width(matrix_int_t, m); x++)
        {
            for (y = 0; y != ect_matrix_height(matrix_int_t, m); y++)
            {
                CUNITTEST_ASSERT_EQ((int)ect_matrix_get(matrix_int_t, m, x, y),
                                    (int)(x * y));
            }
        }
        ec_delete(m);
    }

    CUNITTEST_RESULT();
}
