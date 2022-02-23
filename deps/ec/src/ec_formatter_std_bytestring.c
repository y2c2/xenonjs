/* Enhanced C : Formatter : Standard : bytestring
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_formatter_std_bytestring.h"
#include "ec_algorithm.h"
#include "ec_bytestring.h"
#include "ec_formatter.h"

static int ec_formatter_plugin_cb_apply_anchor_bytestring(
    ec_formatter_plugin_args_apply_anchor_t* args)
{
    ec_bytestring* s =
        ec_formatter_plugin_args_apply_anchor_va_arg(args, ec_bytestring*);
    char* s_c_str = ec_bytestring_c_str(s);
    ec_formatter_plugin_args_apply_anchor_append_c_str(args, s_c_str);
    return 0;
}

int ec_formatter_install_plugin_bytestring(ec_formatter_t* formatter)
{
    ec_formatter_install_plugin(formatter, "bytestring", NULL, NULL, NULL,
                                ec_formatter_plugin_cb_apply_anchor_bytestring);
    return 0;
}
