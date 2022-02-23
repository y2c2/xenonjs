#include "test_mixdt.h"
#include "ec_algorithm.h"
#include "mixdt.h"
#include <stdio.h>

void test_mixdt(void)
{
    vector_int_t2* vec = ect_vector_new(vector_int_t2);
    {
        ect_iterator(vector_int_t2) item1;
        ect_reverse_iterator(vector_int_t2) item2;

        ect_vector_push_back(vector_int_t2, vec, 1);
        ect_vector_push_back(vector_int_t2, vec, 2);
        ect_vector_push_back(vector_int_t2, vec, 3);

        printf("size=%d\n", (int)ect_vector_size(vector_int_t2, vec));

        ect_for(vector_int_t2, vec, item1)
        {
            printf("%d ", *((int*)ec_deref(item1)));
        }
        ect_for_reverse(vector_int_t2, vec, item2)
        {
            printf("%d ", *((int*)ec_deref(item2)));
        }
        printf("\n");
    }
    ect_vector_clear(vector_int_t2, vec);
    {
    }
    ec_delete(vec);
}
