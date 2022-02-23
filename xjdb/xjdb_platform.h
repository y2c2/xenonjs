/* XenonJS : Debugger : Connection
 * Copyright(c) 2018 y2c2 */

#ifndef XJDB_PLATFORM_H
#define XJDB_PLATFORM_H

#if defined(linux)
# define PLATFORM_LINUX
#elif defined(__FreeBSD__)
# define PLATFORM_FREEBSD
#elif defined(__APPLE__)
# define PLATFORM_MACOS
#elif (defined(__GNUC__) && \
        (defined(__MINGW32__) || \
         defined(__MINGW64__) || \
         defined(__MSYS__) || \
         defined(__CYGWIN__))) || \
         defined(_MSC_VER)
# define PLATFORM_WINDOWS
#else
# define PLATFORM_UNKNOWN
#endif

#endif
