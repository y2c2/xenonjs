/* Enhanced C : Bit Determinant
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_bitdet.h"
#include "ec_bitset.h"

struct ec_opaque_bitdet
{
    ec_bitset* bs;
    ec_size_t w, h;
};

static void ec_bitdet_ctor(void* data)
{
    ec_bitdet* r = data;
    r->bs = NULL;
    r->w = 0;
    r->h = 0;
}

static void ec_bitdet_dtor(void* data)
{
    ec_bitdet* r = data;
    ec_delete(r->bs);
}

ec_bitdet* ec_bitdet_new(ec_size_t w, ec_size_t h)
{
    ec_bitset* bs;
    ec_bitdet* bd;

    if (w == 0 || h == 0)
        return NULL;

    if ((bs = ec_bitset_new(w * h)) == NULL)
        return NULL;

    if ((bd = ec_newcd(ec_bitdet, ec_bitdet_ctor, ec_bitdet_dtor)) == NULL)
    {
        ec_delete(bs);
        return NULL;
    }
    bd->w = w;
    bd->h = h;
    bd->bs = bs;

    return bd;
}

ec_size_t ec_bitdet_width(ec_bitdet* bd) { return bd->w; }

ec_size_t ec_bitdet_height(ec_bitdet* bd) { return bd->h; }

void ec_bitdet_resetall(ec_bitdet* bd) { ec_bitset_resetall(bd->bs); }

void ec_bitdet_setall(ec_bitdet* bd) { ec_bitset_setall(bd->bs); }

int ec_bitdet_reset(ec_bitdet* bd, ec_size_t x, ec_size_t y)
{
    return ec_bitset_reset(bd->bs, x + y * bd->w);
}

int ec_bitdet_set(ec_bitdet* bd, ec_size_t x, ec_size_t y)
{
    return ec_bitset_set(bd->bs, x + y * bd->w);
}

ec_bool ec_bitdet_test(ec_bitdet* bd, ec_size_t x, ec_size_t y)
{
    return ec_bitset_test(bd->bs, x + y * bd->w);
}

int ec_bitdet_flip(ec_bitdet* bd, ec_size_t x, ec_size_t y)
{
    return ec_bitset_flip(bd->bs, x + y * bd->w);
}

ec_bool ec_bitdet_all(ec_bitdet* bd) { return ec_bitset_all(bd->bs); }

ec_bool ec_bitdet_any(ec_bitdet* bd) { return ec_bitset_any(bd->bs); }

ec_bool ec_bitdet_none(ec_bitdet* bd) { return ec_bitset_none(bd->bs); }

ec_size_t ec_bitdet_count(ec_bitdet* bd) { return ec_bitset_count(bd->bs); }
