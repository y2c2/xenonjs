/* XenonJS : Error
 * Copyright(c) 2017 y2c2 */

#include <stdarg.h>
#include <ec_libc.h>
#include <ec_alloc.h>
#include <ec_string.h>
#include <ec_encoding.h>
#include <ec_formatter.h>
#include "xjs_dt.h"
#include "xjs_types.h"
#include "xjs_aux.h"
#include "xjs_error.h"

#define _basename(_fullname) \
    (strrchr(_fullname, '/') ? strrchr(_fullname, '/') + 1 : \
     ((strrchr(_fullname, '\\') ? strrchr(_fullname, '\\') + 1 : _fullname)))

void xjs_error_init(xjs_error_ref e)
{
    e->occurred = xjs_false;
    e->loc.filename = NULL;
    e->loc.ln = 0;
    e->desc = NULL;
    e->error_no = 0;
}

void xjs_error_uninit(xjs_error_ref e)
{
    ec_delete(e->desc);
    e->desc = NULL;
}

void xjs_error_update(xjs_error_ref e, \
        const char *filename, const xjs_size_t ln, \
        int error_no)
{
    e->occurred = xjs_true;
    e->loc.filename = _basename(filename);
    e->loc.ln = ln;
    e->error_no = error_no;
}

int xjs_error_update_desc_puts(xjs_error_ref e, \
        const char *desc)
{
    ec_encoding_t enc;
    ec_delete(e->desc); e->desc = NULL;
    ec_encoding_utf8_init(&enc);
    if (ec_encoding_decode(&enc, &e->desc, (const ec_byte_t *)desc, ec_strlen(desc)) != 0)
    { return -1; }
    return 0;
}

int xjs_error_update_desc_printf_source(xjs_error_ref e, \
        ec_string *filepath, \
        const char *fmt, ...)
{
    ec_string *s = NULL;
    va_list ap;
    ec_formatter_t *formatter;

    /* Body */
    if ((formatter = ec_formatter_std_new()) == NULL) { return -1; }
    ec_delete(e->desc); e->desc = NULL;
    va_start(ap, fmt);
    {
        ec_string *basename = xjs_aux_basename(filepath);

        if (basename == NULL)
        { ec_delete(formatter); return -1; }
        if (ec_formatter_formatv(formatter, &s, fmt, ap) != 0)
        { ec_delete(basename); ec_delete(formatter); return -1; }
        if (ec_formatter_format(formatter, &e->desc, "{string}: {string}", basename, s) != 0)
        { ec_delete(basename); ec_delete(s); ec_delete(formatter); return -1; }

        ec_delete(basename);
    }
    va_end(ap);
    ec_delete(s);
    ec_delete(formatter);

    return 0;
}

int xjs_error_update_desc_printf(xjs_error_ref e, \
        const char *fmt, ...)
{
    va_list ap;
    ec_formatter_t *formatter;

    if ((formatter = ec_formatter_std_new()) == NULL) { return -1; }
    ec_delete(e->desc); e->desc = NULL;
    va_start(ap, fmt);
    {
        if (ec_formatter_formatv(formatter, &e->desc, fmt, ap) != 0)
        { ec_delete(formatter); return -1; }
    }
    va_end(ap);
    ec_delete(formatter);

    return 0;
}

