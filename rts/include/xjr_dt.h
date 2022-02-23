/* XenonJS : Runtime Time System : Data Types
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_DT_H
#define XJR_DT_H

typedef unsigned int xjr_size_t;
typedef unsigned int xjr_offset_t;
typedef unsigned int xjr_u32;
typedef signed int xjr_s32;
typedef unsigned short xjr_u16;
typedef signed short xjr_s16;
typedef unsigned char xjr_u8;
typedef signed char xjr_s8;
typedef float xjr_f32;
typedef double xjr_f64;
typedef xjr_u8 xjr_reg;
typedef xjr_u32 xjr_val;
typedef unsigned int xjr_size_t;
typedef enum
{
    xjr_false = 0,
    xjr_true = 1,
} xjr_bool;
#define xjr_nullptr ((void*)(0))

typedef void *(*xjr_heap_malloc_callback)(void *heap, xjr_size_t size);
typedef void (*xjr_heap_free_callback)(void *heap, void *p);

#endif

