/* Enhanced C : Formatter : Standard : char *
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_formatter_std_c_str.h"
#include "ec_formatter.h"
#include "ec_string.h"

static int ec_formatter_plugin_cb_apply_anchor_c_str(
    ec_formatter_plugin_args_apply_anchor_t* args)
{
    char* s = ec_formatter_plugin_args_apply_anchor_va_arg(args, char*);
    ec_formatter_plugin_args_apply_anchor_append_c_str(args, s);
    return 0;
}

int ec_formatter_install_plugin_c_str(ec_formatter_t* formatter)
{
    ec_formatter_install_plugin(formatter, "c_str", NULL, NULL, NULL,
                                ec_formatter_plugin_cb_apply_anchor_c_str);
    return 0;
}
