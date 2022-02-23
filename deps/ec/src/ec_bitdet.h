/* Enhanced C : Bit Determinant
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_BITDET_H
#define EC_BITDET_H

#include "ec_alloc.h"
#include "ec_dt.h"

struct ec_opaque_bitdet;
typedef struct ec_opaque_bitdet ec_bitdet;

ec_bitdet* ec_bitdet_new(ec_size_t w, ec_size_t h);

ec_size_t ec_bitdet_width(ec_bitdet* bd);
ec_size_t ec_bitdet_height(ec_bitdet* bd);

void ec_bitdet_resetall(ec_bitdet* bd);
void ec_bitdet_setall(ec_bitdet* bd);

int ec_bitdet_reset(ec_bitdet* bd, ec_size_t x, ec_size_t y);
int ec_bitdet_set(ec_bitdet* bd, ec_size_t x, ec_size_t y);
ec_bool ec_bitdet_test(ec_bitdet* bd, ec_size_t x, ec_size_t y);
int ec_bitdet_flip(ec_bitdet* bd, ec_size_t x, ec_size_t y);

ec_bool ec_bitdet_all(ec_bitdet* bd);
ec_bool ec_bitdet_any(ec_bitdet* bd);
ec_bool ec_bitdet_none(ec_bitdet* bd);

ec_size_t ec_bitdet_count(ec_bitdet* bd);

#endif
