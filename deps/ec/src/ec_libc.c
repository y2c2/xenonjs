/* Enhanced C : LibC
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_libc.h"
#include "ec_alloc.h"
#include "ec_dt.h"

ec_size_t ec_strlen(const char* s)
{
    ec_size_t len = 0;
    const char* p = s;
    while (*p++ != '\0')
        len++;
    return len;
}

char* ec_strdup(const char* s)
{
    ec_size_t len = ec_strlen(s);
    char* p;
    if ((p = ec_malloc(sizeof(char) * (len + 1))) == NULL)
    {
        return NULL;
    }
    ec_memcpy(p, s, len + 1);
    return p;
}

int ec_strcmp(const char* s1, const char* s2)
{
    const char *p1 = s1, *p2 = s2;
    if ((p1 == NULL) || (p2 == NULL))
        return -1;
    for (;;)
    {
        if ((*p1 != *p2) || (*p1 == '\0'))
            break;
        p1++;
        p2++;
    }
    return *p1 - *p2;
}

int ec_strncmp(const char* s1, const char* s2, ec_size_t n)
{
    const char *p1 = s1, *p2 = s2;
    if (n != 0)
    {
        do
        {
            if (*p1 != *p2++)
                break;
            if (*p1++ == '\0')
                return 0;
        } while (--n != 0);
        if (n > 0)
        {
            if (*p1 == '\0')
                return -1;
            if (*--p2 == '\0')
                return 1;
            return (unsigned char)*p1 - (unsigned char)*p2;
        }
    }
    return 0;
}
