/* Enhanced C : Formatter
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_FORMATTER_H
#define EC_FORMATTER_H

#include "ec_dt.h"
#include "ec_formatter_dt.h"
#include "ec_string.h"
#include <stdarg.h>

/*
 *
 * {int} : %d
 * {int(format:hex)} : %d
 * {char} : %c
 * {float(precision:2)} : %.2f
 * {string} : %s
 *
 *
 * Steps of using a formatter
 *
 * 1. Create a blank formatter with 'ec_formatter_new'
 * 2. Install plugins with 'ec_formatter_install_plugin'
 * 3. Set template with 'ec_formatter_set_template'
 */

ec_formatter_t* ec_formatter_new(void);

ec_formatter_t* ec_formatter_std_new(void);

int ec_formatter_install_plugin(
    ec_formatter_t* formatter, const char* name,
    ec_formatter_plugin_cb_init_t cb_init,
    ec_formatter_plugin_cb_uninit_t cb_uninit,
    ec_formatter_plugin_cb_make_anchor_t cb_make_anchor,
    ec_formatter_plugin_cb_apply_anchor_t cb_apply_anchor);

int ec_formatter_formatv(ec_formatter_t* formatter, ec_string** out,
                         const char* fmt, va_list ap);

int ec_formatter_format(ec_formatter_t* formatter, ec_string** out,
                        const char* fmt, ...);

#endif
