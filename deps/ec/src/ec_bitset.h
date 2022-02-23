/* Enhanced C : Bit Set
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_BITSET_H
#define EC_BITSET_H

#include "ec_alloc.h"
#include "ec_dt.h"

struct ec_opaque_bitset;
typedef struct ec_opaque_bitset ec_bitset;

ec_bitset* ec_bitset_new(ec_size_t size);

ec_size_t ec_bitset_size(ec_bitset* bs);

void ec_bitset_resetall(ec_bitset* bs);
void ec_bitset_setall(ec_bitset* bs);

int ec_bitset_reset(ec_bitset* bs, ec_size_t pos);
int ec_bitset_set(ec_bitset* bs, ec_size_t pos);
ec_bool ec_bitset_test(ec_bitset* bs, ec_size_t pos);
int ec_bitset_flip(ec_bitset* bs, ec_size_t pos);

ec_bool ec_bitset_all(ec_bitset* bs);
ec_bool ec_bitset_any(ec_bitset* bs);
ec_bool ec_bitset_none(ec_bitset* bs);

ec_size_t ec_bitset_count(ec_bitset* bs);

#endif
