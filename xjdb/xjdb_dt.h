/* XenonJS : Debugger : Data Types
 * Copyright(c) 2018 y2c2 */

#ifndef XJDB_DT_H
#define XJDB_DT_H

typedef unsigned int xjdb_size_t;
typedef unsigned int xjdb_offset_t;
typedef unsigned int xjdb_u32;
typedef signed int xjdb_s32;
typedef unsigned short xjdb_u16;
typedef signed short xjdb_s16;
typedef unsigned char xjdb_u8;
typedef signed char xjdb_s8;
typedef float xjdb_f32;
typedef double xjdb_f64;
typedef xjdb_u8 xjdb_reg;
typedef xjdb_u32 xjdb_val;
typedef unsigned int xjdb_size_t;
typedef enum
{
    xjdb_false = 0,
    xjdb_true = 1,
} xjdb_bool;
#define xjdb_nullptr ((void*)(0))

typedef void *(*xjdb_heap_malloc_callback)(void *heap, xjdb_size_t size);
typedef void (*xjdb_heap_free_callback)(void *heap, void *p);

#endif

