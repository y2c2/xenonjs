/* Enhanced C : Data Types
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_DT_H
#define EC_DT_H

#ifndef __cplusplus
typedef enum
{
    ec_false = 0,
    ec_true = 1,
} ec_bool;
#else
typedef enum
{
    ec_false = false,
    ec_true = true,
} ec_bool;
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <stdint.h>
#include <stdio.h>
typedef uint64_t ec_u64;
typedef uint32_t ec_u32;
typedef int32_t ec_s32;
typedef int64_t ec_s64;
typedef uint16_t ec_u16;
typedef int16_t ec_s16;
typedef uint8_t ec_u8;
typedef int8_t ec_s8;
typedef size_t ec_size_t;
#else
typedef unsigned long long ec_u64;
typedef unsigned int ec_u32;
typedef unsigned short int ec_u16;
typedef unsigned char ec_u8;
typedef signed long long ec_s64;
typedef signed int ec_s32;
typedef signed short int ec_s16;
typedef char ec_s8;
typedef unsigned int ec_size_t;
#endif

typedef ec_u32 ec_char_t;
typedef ec_size_t ec_offset_t;
typedef ec_u8 ec_byte_t;

#endif
