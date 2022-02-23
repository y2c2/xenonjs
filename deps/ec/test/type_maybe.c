#include "ec_maybe.h"

static int int_cmp(int* a, int* b)
{
    if (*a < *b)
        return -1;
    else if (*a > *b)
        return 1;
    else
        return 0;
}

ect_maybe_define(maybe_int, int, NULL, int_cmp);
