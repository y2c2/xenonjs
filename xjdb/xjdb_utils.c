/* XenonJS : Debugger : Utils
 * Copyright(c) 2018 y2c2 */

#include "xjdb_platform.h"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#elif defined(PLATFORM_LINUX)
#include <unistd.h>
#endif

#include "xjdb_utils.h"

void xjdb_sleep(void)
{
#if defined(PLATFORM_WINDOWS)
    Sleep(50);
#elif defined(PLATFORM_LINUX)
    usleep(50000);
#endif
}

