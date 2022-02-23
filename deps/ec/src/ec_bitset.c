/* Enhanced C : Bit Set
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_bitset.h"

struct ec_opaque_bitset
{
    ec_u32* units;
    ec_size_t unit_count;
    ec_size_t bit_count;
};

static void ec_bitset_ctor(void* data)
{
    ec_bitset* r = data;
    r->units = NULL;
    r->unit_count = 0;
    r->bit_count = 0;
}

static void ec_bitset_dtor(void* data)
{
    ec_bitset* r = data;
    if (r->units != NULL)
        ec_free(r->units);
}

ec_bitset* ec_bitset_new(ec_size_t bit_count)
{
    ec_size_t unit_count;
    ec_u32* units;

    if (bit_count == 0)
        return NULL;

    unit_count = (bit_count >> 5) + ((bit_count & 31) != 0 ? 1 : 0);
    units = ec_malloc(sizeof(ec_u32) * unit_count);
    if (units != NULL)
    {
        ec_size_t i;
        ec_bitset* r = ec_newcd(ec_bitset, ec_bitset_ctor, ec_bitset_dtor);
        r->bit_count = bit_count;
        r->unit_count = unit_count;
        r->units = units;
        for (i = 0; i != unit_count; i++)
        {
            r->units[i] = 0;
        }
        return r;
    }
    return NULL;
}

ec_size_t ec_bitset_size(ec_bitset* bs) { return bs->bit_count; }

void ec_bitset_resetall(ec_bitset* bs)
{
    ec_size_t i;
    for (i = 0; i != bs->unit_count; i++)
    {
        bs->units[i] = 0;
    }
}

void ec_bitset_setall(ec_bitset* bs)
{
    ec_size_t i;
    for (i = 0; i != bs->unit_count; i++)
    {
        bs->units[i] = 0xFFFFFFFF;
    }
}

int ec_bitset_reset(ec_bitset* bs, ec_size_t pos)
{
    if (pos > bs->bit_count)
        return -1;
    bs->units[pos >> 5] &= ~(1 << (pos & 31));
    return 0;
}

int ec_bitset_set(ec_bitset* bs, ec_size_t pos)
{
    if (pos > bs->bit_count)
        return -1;
    bs->units[pos >> 5] |= 1 << (pos & 31);
    return 0;
}

ec_bool ec_bitset_test(ec_bitset* bs, ec_size_t pos)
{
    if (pos > bs->bit_count)
        return ec_false;

    return ((bs->units[pos >> 5] & (1 << (pos & 31))) != 0) ? ec_true
                                                            : ec_false;
}

int ec_bitset_flip(ec_bitset* bs, ec_size_t pos)
{
    if (pos > bs->bit_count)
        return -1;

    if (ec_bitset_test(bs, pos) == ec_true)
    {
        ec_bitset_reset(bs, pos);
    }
    else
    {
        ec_bitset_set(bs, pos);
    }

    return 0;
}

ec_bool ec_bitset_all(ec_bitset* bs)
{
    ec_size_t pos;

    /* TODO: Optimize (check unit by unit) */
    for (pos = 0; pos != bs->bit_count; pos++)
    {
        if (ec_bitset_test(bs, pos) == ec_false)
            return ec_false;
    }

    return ec_true;
}

ec_bool ec_bitset_any(ec_bitset* bs)
{
    ec_size_t pos;

    /* TODO: Optimize (check unit by unit) */
    for (pos = 0; pos != bs->bit_count; pos++)
    {
        if (ec_bitset_test(bs, pos) == ec_true)
            return ec_true;
    }

    return ec_false;
}

ec_bool ec_bitset_none(ec_bitset* bs)
{
    ec_size_t pos;

    /* TODO: Optimize (check unit by unit) */
    for (pos = 0; pos != bs->bit_count; pos++)
    {
        if (ec_bitset_test(bs, pos) == ec_true)
            return ec_false;
    }

    return ec_true;
}

ec_size_t ec_bitset_count(ec_bitset* bs)
{
    ec_size_t pos;
    ec_size_t count = 0;

    for (pos = 0; pos != bs->bit_count; pos++)
    {
        if (ec_bitset_test(bs, pos) == ec_true)
            count++;
    }

    return count;
}
