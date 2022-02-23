/* XenonJS : Data Types
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_DT_H
#define XJS_DT_H

#ifndef __cplusplus 
typedef enum
{
    xjs_false = 0,
    xjs_true = 1,
} xjs_bool;
#else
typedef enum
{
    xjs_false = false,
    xjs_true = true,
} xjs_bool;
#endif

#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <stdint.h>
#include <stdio.h>
typedef uint64_t xjs_u64;
typedef uint32_t xjs_u32;
typedef int32_t xjs_s32;
typedef int64_t xjs_s64;
typedef uint16_t xjs_u16;
typedef int16_t xjs_s16;
typedef uint8_t xjs_u8;
typedef int8_t xjs_s8;
typedef size_t xjs_size_t;
#else
typedef unsigned long long xjs_u64;
typedef unsigned int xjs_u32;
typedef unsigned short int xjs_u16;
typedef unsigned char xjs_u8;
typedef signed long long xjs_s64;
typedef signed int xjs_s32;
typedef signed short int xjs_s16;
typedef char xjs_s8;
typedef unsigned int xjs_size_t;
#endif
typedef float xjs_f32;
typedef double xjs_f64;

typedef xjs_u32 xjs_char_t;
typedef xjs_size_t xjs_offset_t;
typedef xjs_u8 xjs_byte_t;

#endif


