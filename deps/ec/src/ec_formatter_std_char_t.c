/* Enhanced C : Formatter : Standard : char_t
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_formatter_std_char_t.h"
#include "ec_algorithm.h"
#include "ec_formatter.h"
#include "ec_string.h"

static int ec_formatter_plugin_cb_apply_anchor_char_t(
    ec_formatter_plugin_args_apply_anchor_t* args)
{
    ec_char_t ch =
        ec_formatter_plugin_args_apply_anchor_va_arg(args, ec_char_t);
    ec_formatter_plugin_args_apply_anchor_append_char(args, ch);
    return 0;
}

int ec_formatter_install_plugin_char_t(ec_formatter_t* formatter)
{
    ec_formatter_install_plugin(formatter, "char_t", NULL, NULL, NULL,
                                ec_formatter_plugin_cb_apply_anchor_char_t);
    return 0;
}
