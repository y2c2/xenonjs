/* XenonJS : Runtime Time System : LibC
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_LIBC_H
#define XJR_LIBC_H

#include "xjr_dt.h"

void *xjr_memset(void *s, int c, xjr_size_t n);
void *xjr_memcpy(void *dest, const void *src, xjr_size_t n);
int xjr_strncmp(const char *s1, const char *s2, xjr_size_t n);
xjr_size_t xjr_strlen(const char *s);

#endif

