#include "ec_alloc.h"
#include "test_bitset.h"
#include "test_encoding.h"
#include "test_enum.h"
#include "test_exception.h"
#include "test_formatter.h"
#include "test_list.h"
#include "test_map.h"
#include "test_matrix.h"
#include "test_maybe.h"
#include "test_set.h"
#include "test_stack.h"
#include "test_string.h"
#include "test_vector.h"
#include <stdlib.h>
#include <string.h>

int main(void)
{
    ec_allocator_set_malloc(malloc);
    ec_allocator_set_calloc(calloc);
    ec_allocator_set_free(free);
    ec_allocator_set_memset(memset);
    ec_allocator_set_memcpy(memcpy);

    test_list();
    test_vector();
    test_matrix();
    test_stack();
    test_set();
    test_map();
    test_string();
    test_bitset();
    test_enum();
    test_maybe();
    test_encoding();
    test_formatter();
    test_exception();

    return 0;
}
