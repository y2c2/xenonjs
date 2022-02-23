/* XenonJS : LibC
 * Copyright(c) 2017 y2c2 */

#include "xjs_libc.h"

/*
void *xjs_memcpy(void *dest, const void *src, xjs_size_t n)
{
    char *dest_p = dest;
    const char *src_p = src;
    while (n-- != 0)
    {
        *dest_p++ = *src_p++;
    }
    return dest;
}
*/

xjs_size_t xjs_strlen(const char *s)
{
    xjs_size_t len = 0;
    const char *p = s;
    while (*p++ != '\0') len++;
    return len;
}

