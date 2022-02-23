/* Enhanced C : Formatter : Standard
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_formatter_std.h"
#include "ec_formatter.h"
#include "ec_formatter_dt.h"
#include "ec_formatter_std_bytestring.h"
#include "ec_formatter_std_c_str.h"
#include "ec_formatter_std_char_t.h"
#include "ec_formatter_std_int.h"
#include "ec_formatter_std_string.h"

#define V_NEZ(_x)                                                              \
    do                                                                         \
    {                                                                          \
        if ((_x) != 0)                                                         \
        {                                                                      \
            ret = -1;                                                          \
            goto fail;                                                         \
        }                                                                      \
    } while (0)

int ec_formatter_install_std_plugins(ec_formatter_t* formatter)
{
    int ret = 0;
    V_NEZ(ec_formatter_install_plugin_int(formatter));
    V_NEZ(ec_formatter_install_plugin_unsigned_int(formatter));
    V_NEZ(ec_formatter_install_plugin_s8(formatter));
    V_NEZ(ec_formatter_install_plugin_u8(formatter));
    V_NEZ(ec_formatter_install_plugin_s16(formatter));
    V_NEZ(ec_formatter_install_plugin_u16(formatter));
    V_NEZ(ec_formatter_install_plugin_s32(formatter));
    V_NEZ(ec_formatter_install_plugin_u32(formatter));
    V_NEZ(ec_formatter_install_plugin_size_t(formatter));
    V_NEZ(ec_formatter_install_plugin_char_t(formatter));
    V_NEZ(ec_formatter_install_plugin_c_str(formatter));
    V_NEZ(ec_formatter_install_plugin_string(formatter));
    V_NEZ(ec_formatter_install_plugin_bytestring(formatter));
fail:
    return ret;
}
