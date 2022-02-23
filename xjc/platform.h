/* XenonJS : Compiler Driver : Platform
 * Copyright(c) 2017-2018 y2c2 */

#ifndef PLATFORM_H
#define PLATFORM_H

#if (defined(__GNUC__) && \
        (defined(__MINGW32__) || \
         defined(__MINGW64__) || \
         defined(__MSYS__) || \
         defined(__CYGWIN__))) || \
         defined(_MSC_VER)
#  define PLATFORM_WINDOWS
#elif defined(__linux)
#  define PLATFORM_LINUX
#elif defined(__FreeBSD__)
#  define PLATFORM_FREEBSD
#elif defined(__APPLE__)
#  define PLATFORM_MACOS
#else
#  define PLATFORM_UNKNOWN
#endif

#endif
