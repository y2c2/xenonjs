/* Enhanced C : Formatter : Template
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_FORMATTER_TEMPLATE_H
#define EC_FORMATTER_TEMPLATE_H

#include "ec_dt.h"
#include "ec_formatter_dt.h"
#include "ec_string.h"

int ec_formatter_template_build_from_fmt(ec_formatter_template_t** tplt_out,
                                         ec_formatter_t* formatter,
                                         ec_string* fmt);

int ec_formatter_template_build_from_fmt_cstr(
    ec_formatter_template_t** tplt_out, ec_formatter_t* formatter,
    const char* fmt);

ec_size_t ec_formatter_template_items_count(ec_formatter_template_t* tplt);

#endif
