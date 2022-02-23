/* Test Framework
 * Copyright(c) 2017 y2c2 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "testfw.h"


int cunittest_printf(const char *format, ...)
{
    va_list args;
    int ret;
 
    va_start(args,format);
    ret = vprintf(format,args);
    va_end(args);

    return ret;
}

void cunittest_result(struct cunittest *cu_p)
{
    if (cu_p->total == 0)
    {
        printf("No testcase has been added\n");
    }
    else
    {
        if (cu_p->passed == cu_p->total)
        {
            printf("All %u testcase(s) passed\n", \
                    cu_p->total);
        }
        else
        {
            printf("%u of %u testcase(s) passed\n", \
                    cu_p->passed, cu_p->total);
        }
    }
    printf("\n");
}

