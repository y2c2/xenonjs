/* Enhanced C : Formatter : Standard : string
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_formatter_std_string.h"
#include "ec_algorithm.h"
#include "ec_formatter.h"
#include "ec_string.h"

static int ec_formatter_plugin_cb_apply_anchor_string(
    ec_formatter_plugin_args_apply_anchor_t* args)
{
    ec_string* s =
        ec_formatter_plugin_args_apply_anchor_va_arg(args, ec_string*);
    ec_formatter_plugin_args_apply_anchor_append_string(args, s);
    return 0;
}

int ec_formatter_install_plugin_string(ec_formatter_t* formatter)
{
    ec_formatter_install_plugin(formatter, "string", NULL, NULL, NULL,
                                ec_formatter_plugin_cb_apply_anchor_string);
    return 0;
}
