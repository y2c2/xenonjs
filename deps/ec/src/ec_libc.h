/* Enhanced C : LibC
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_LIBC_H
#define EC_LIBC_H

#include "ec_dt.h"

ec_size_t ec_strlen(const char* s);

char* ec_strdup(const char* s);

int ec_strcmp(const char* s1, const char* s2);
int ec_strncmp(const char* s1, const char* s2, ec_size_t n);

#endif
