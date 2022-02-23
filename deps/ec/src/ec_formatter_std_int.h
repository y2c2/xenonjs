/* Enhanced C : Formatter : Standard : int
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_FORMATTER_STD_INT_H
#define EC_FORMATTER_STD_INT_H

#include "ec_formatter_dt.h"

int ec_formatter_install_plugin_int(ec_formatter_t* formatter);
int ec_formatter_install_plugin_unsigned_int(ec_formatter_t* formatter);
int ec_formatter_install_plugin_s8(ec_formatter_t* formatter);
int ec_formatter_install_plugin_u8(ec_formatter_t* formatter);
int ec_formatter_install_plugin_s16(ec_formatter_t* formatter);
int ec_formatter_install_plugin_u16(ec_formatter_t* formatter);
int ec_formatter_install_plugin_s32(ec_formatter_t* formatter);
int ec_formatter_install_plugin_u32(ec_formatter_t* formatter);
int ec_formatter_install_plugin_size_t(ec_formatter_t* formatter);

#endif
