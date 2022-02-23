/* XenonJS : Runtime Time System : LibC
 * Copyright(c) 2017 y2c2 */

#include <stdlib.h>
#include <string.h>
#include "xjr_libc.h"

void *xjr_memset(void *s, int c, xjr_size_t n)
{
    return memset(s, c, n);
}

void *xjr_memcpy(void *dest, const void *src, xjr_size_t n)
{
    char *dest_p = dest;
    const char *src_p = src;
    while (n-- != 0)
    {
        *dest_p++ = *src_p++;
    }
    return dest;
}

int xjr_strncmp(const char *s1, const char *s2, xjr_size_t n)
{
    const char *p1 = s1, *p2 = s2;
    
    if (n != 0)
    {
        do
        {
            if (*p1 != *p2++) break;
            if (*p1++ == '\0') return 0;
        }
        while (--n != 0);
        if (n > 0)
        {
            if (*p1 == '\0') return -1;
            if (*--p2 == '\0') return 1;
            return (unsigned char)*p1 - (unsigned char)*p2;
        }
    }
    return 0;
}

xjr_size_t xjr_strlen(const char *s)
{
    xjr_size_t len = 0;
    const char *p = s;
    while (*p++ != '\0') len++;
    return len;
}

