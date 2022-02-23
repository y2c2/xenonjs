/* XenonJS : Error
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_ERROR_H
#define XJS_ERROR_H

#include <stdarg.h>
#include <string.h>
#include <ec_string.h>
#include "xjs_dt.h"
#include "xjs_types.h"

void xjs_error_init(xjs_error_ref e);

void xjs_error_uninit(xjs_error_ref e);

void xjs_error_update(xjs_error_ref e, \
        const char *filename, const xjs_size_t ln, \
        int error_no);

int xjs_error_update_desc_puts(xjs_error_ref e, \
        const char *desc);

int xjs_error_update_desc_printf_source(xjs_error_ref e, \
        ec_string *filepath, \
        const char *fmt, ...);

int xjs_error_update_desc_printf(xjs_error_ref e, \
        const char *fmt, ...);

#define XJS_ERROR(_e, _errno) \
    do { \
        xjs_error_update(_e, __FILE__, __LINE__, _errno); \
    } while (0)

#define XJS_ERROR_PUTS(_e, _desc) \
    do { \
        if (xjs_error_update_desc_puts(_e, _desc) != 0) { \
            xjs_error_update(_e, __FILE__, __LINE__, -XJS_ERRNO_INTERNAL); \
        } \
    } while (0)

#define XJS_ERROR_PRINTF(_e, ...) \
    do { \
        if (xjs_error_update_desc_printf(_e, __VA_ARGS__) != 0) { \
            xjs_error_update(_e, __FILE__, __LINE__, -XJS_ERRNO_INTERNAL); \
        } \
    } while (0)

#define XJS_ERROR_PRINTF_SOURCE(_e, _filepath, ...) \
    do { \
        if (xjs_error_update_desc_printf_source(_e, _filepath, __VA_ARGS__) != 0) { \
            xjs_error_update(_e, __FILE__, __LINE__, -XJS_ERRNO_INTERNAL); \
        } \
    } while (0)


#endif

