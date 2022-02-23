#include "ec_set.h"

static int int_ctor(int* detta_key, int* key)
{
    *detta_key = *key;
    return 0;
}

static void int_dtor(int* detta_key) { (void)detta_key; }

static int int_cmp(int* a, int* b)
{
    if (*a > *b)
        return 1;
    else if (*a < *b)
        return -1;
    return 0;
}

ect_set_define_undeclared(set_int_t, int, int_ctor, int_dtor, int_cmp);
