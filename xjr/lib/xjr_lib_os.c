/* XenonJS : Runtime Time System : Library : OS
 * Copyright(c) 2017 y2c2 */

#include "xjr_lib_os.h"

static void os_arch(xjr_native_fn_args *args)
{
#if (defined(_MSV_VER) && (defined(_M_X64) || defined(_M_AMD64))) || \
    (defined(__GNUC__) && (defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)))
    args->ret = xjr_val_make_string_from_heap(args->mp, "x64", 3);
#elif (defined(_MSV_VER) && (defined(_M_IX86)) || \
        (defined(__GNUC__) && (defined(i386)) || (defined(__i386)) || (defined(__i386__))))
    args->ret = xjr_val_make_string_from_heap(args->mp, "x32", 3);
#elif (defined(__GNUC__) && defined(__aarch64__))
    args->ret = xjr_val_make_string_from_heap(args->mp, "arm64", 5);
#elif (defined(__GNUC__) && defined(__arm__))
    args->ret = xjr_val_make_string_from_heap(args->mp, "arm", 3);
#elif (defined(__GNUC__) && (defined(__mips__) || defined(mips)))
    args->ret = xjr_val_make_string_from_heap(args->mp, "mips", 4);
#else
    args->ret = xjr_val_make_string_from_heap(args->mp, "unknown", 7);
#endif
}

static void os_platform(xjr_native_fn_args *args)
{
#if defined(__FreeBSD__)
    args->ret = xjr_val_make_string_from_heap(args->mp, "freebsd", 7);
#elif defined(__NetBSD__)
    args->ret = xjr_val_make_string_from_heap(args->mp, "netbsd", 6);
#elif defined(__OpenBSD__)
    args->ret = xjr_val_make_string_from_heap(args->mp, "openbsd", 7);
#elif defined(__gnu_hurd__)
    args->ret = xjr_val_make_string_from_heap(args->mp, "hurd", 4);
#elif defined(__linux__)
    args->ret = xjr_val_make_string_from_heap(args->mp, "linux", 5);
#elif defined(__CYGWIN__)
# if defined(__x86_64__)
    args->ret = xjr_val_make_string_from_heap(args->mp, "win64", 5);
# else
    args->ret = xjr_val_make_string_from_heap(args->mp, "win32", 5);
# endif
#elif defined(_WIN32)
# if defined(_WIN64)
    args->ret = xjr_val_make_string_from_heap(args->mp, "win64", 5);
# else
    args->ret = xjr_val_make_string_from_heap(args->mp, "win32", 5);
# endif
#elif defined(__APPLE__)
    args->ret = xjr_val_make_string_from_heap(args->mp, "darwin", 6);
#else
    args->ret = xjr_val_make_string_from_heap(args->mp, "unknown", 7);
#endif
}

/*
 * export default {
 *   "arch": os_arch,
 *   "platform": os_platform
 * };
 */

int xjr_lib_os(xjr_lib_ctx *ctx)
{
    xjr_lib_export_default()
    {
        xjr_lib_export_default_item("arch", 4, xjr_val_make_native_function(ctx->vm->rts.rheap.mp, ctx->env, os_arch));
        xjr_lib_export_default_item("platform", 8, xjr_val_make_native_function(ctx->vm->rts.rheap.mp, ctx->env, os_platform));
    }

    return 0;
}

