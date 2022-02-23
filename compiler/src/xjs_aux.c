#include <ec_string.h>
#include <ec_algorithm.h>
#include <ec_libc.h>
#include "xjs_types.h"
#include "xjs_aux.h"

xjs_bool xjs_aux_string_match_with(ec_string *s, const char *c_str)
{
    xjs_size_t len = ec_strlen(c_str);
    ect_iterator(ec_string) it;
    const char *p = c_str;

    if (len != (xjs_size_t)ec_string_length(s)) return xjs_false;
    ect_for(ec_string, s, it)
    {
        if (ect_deref(ec_char_t, it) != (ec_char_t)(*p))
        { return xjs_false; }
        p++;
    }

    return xjs_true;
}

int xjs_aux_serialize_append_number( \
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

int xjs_aux_serialize_append_int(ec_string *result, int value)
{
    int ret = 0;
    ec_string *result_integer_part = NULL;
    ec_bool negative = ec_false;

    if (value < 0)
    {
        negative = ec_true;
        value = -value;
    }

    /* Integer part */
    if ((result_integer_part = ec_string_new()) == NULL)
    { return -1; }
    if (value == 0)
    {
        ec_string_push_back(result_integer_part, '0');
    }
    else
    {
        while (value != 0)
        {
            ec_string_push_front(result_integer_part, '0' + (value % 10));
            value /= 10;
        }
        if (negative == ec_true) { ec_string_push_front(result_integer_part, '-'); }
    }
    ec_string_append(result, result_integer_part);

    ec_delete(result_integer_part);

    return ret;
}

ec_string *xjs_aux_basename(ec_string *fullname)
{
    ec_string_size_type pos = ec_string_rfind(fullname, '/', ec_string_length(fullname));

    if (pos == ec_string_npos)
    { pos = ec_string_rfind(fullname, '\\', ec_string_length(fullname)); }

    if (pos == ec_string_npos) { return ec_string_clone(fullname); }

    return ec_string_substr(fullname, pos + 1, ec_string_length(fullname) - pos - 1);
}


ec_string *xjs_aux_extname(ec_string *fullname)
{
    ec_string_size_type pos_slash = ec_string_rfind(fullname, '/', ec_string_length(fullname));
    ec_string_size_type pos_dot = ec_string_rfind(fullname, '.', ec_string_length(fullname));

    if (pos_slash == ec_string_npos)
    { pos_slash = ec_string_rfind(fullname, '\\', ec_string_length(fullname)); }

    if (pos_dot == ec_string_npos) { return ec_string_new(); }

    if ((pos_slash != ec_string_npos) && (pos_dot < pos_slash)) { return ec_string_new(); }

    return ec_string_substr(fullname, pos_dot + 1, ec_string_length(fullname) - pos_dot - 1);
}

ec_string *xjs_aux_mainname(ec_string *fullname)
{
    ec_string *result;
    ec_string *basename = xjs_aux_basename(fullname);
    ec_string_size_type pos_dot = ec_string_rfind(basename, '.', ec_string_length(basename));
    if (pos_dot == ec_string_npos)
    {
        return basename;
    }

    result = ec_string_substr(basename, 0, pos_dot);
    ec_delete(basename);
    return result;
}

