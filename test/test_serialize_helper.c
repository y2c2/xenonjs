#include <ec_string.h>
#include "test_serialize_helper.h"

int xjs_serialize_append_number( \
        ec_string *result, double value)
{
    int ret = 0;
    ec_string *result_integer_part = NULL;
    int value_integer = (int)value;
    ec_bool negative = ec_false;

    if (value_integer < 0)
    {
        negative = ec_true;
        value_integer = -value_integer;
    }

    value -= (double)value_integer;

    /* Integer part */
    if ((result_integer_part = ec_string_new()) == NULL)
    { return -1; }
    if (value_integer == 0)
    {
        ec_string_push_back(result_integer_part, '0');
    }
    else
    {
        while (value_integer != 0)
        {
            ec_string_push_front(result_integer_part, '0' + (value_integer % 10));
            value_integer /= 10;
        }
        if (negative == ec_true) { ec_string_push_front(result_integer_part, '-'); }
    }
    ec_string_append(result, result_integer_part);

    /* Fractal part */
    if (value != 0.0)
    {
        /* const double double_epsilon = 2.22045e-016; */
        /* const double double_epsilon = 0.000000000000001; /1* Magic? *1/ */
        const double double_epsilon = 0.0000000001; /* Magic? */
        int limit = 16; /* Magic? */
        ec_string_append_c_str(result, ".");
        while ((value >= double_epsilon) && (limit-- >= 0))
        {
            int d;
            value *= 10;
            d = ((int)value % 10);
            ec_string_push_back(result, (ec_char_t)('0' + d));
            value = value - (double)d;
        }
    }

    ec_delete(result_integer_part);
    return ret;
}

