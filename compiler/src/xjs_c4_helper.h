/* XenonJS : C4 : Helper
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_C4_HELPER_H
#define XJS_C4_HELPER_H

#include <ec_string.h>
#include <ec_list.h>
#include <ec_map.h>
#include "xjs_xformat.h"
#include "xjs_types.h"

typedef struct
{
    ec_u32 opcode;

    /* Offset to the beginning of the function */
    ec_size_t offset;

    struct
    {
        struct
        {
            ec_size_t cnt;
        } as_allocreg;
        struct
        {
            xjs_ir_reg reg;
        } as_ret;
        struct
        {
            xjs_ir_reg exports;
            xjs_ir_reg module_name;
        } as_dynlib;
        struct
        {
            xjs_ir_reg callee;
            xjs_ir_reg dst;
            ec_size_t argc;
            xjs_ir_reg argv[XJS_BC_ARGC_MAX];
        } as_call, as_new;
        struct
        {
            xjs_ir_reg callee;
            xjs_ir_reg _this;
            xjs_ir_reg dst;
            ec_size_t argc;
            xjs_ir_reg argv[XJS_BC_ARGC_MAX];
        } as_callt;
        struct
        {
            xjs_ir_reg dst;
        } as_this;
        struct
        {
            xjs_ir_reg reg;
        } as_inspect;
        struct
        {
            xjs_ir_offset offset_data;
        } as_declvar;
        struct
        {
            xjs_ir_offset offset_data;
            xjs_ir_reg idx;
        } as_arg;
        struct
        {
            xjs_ir_offset offset_data;
            xjs_ir_reg reg;
        } as_load, as_store;
        struct
        {
            xjs_ir_reg dst;
            xjs_ir_reg obj;
            xjs_ir_reg member;
            xjs_ir_reg src;
        } as_objset;
        struct
        {
            xjs_ir_reg dst;
            xjs_ir_reg obj;
            xjs_ir_reg member;
        } as_objget;
        struct
        {
            xjs_ir_reg arr;
            xjs_ir_reg elem;
        } as_arrpush;
        struct
        {
            xjs_ir_offset dest;
        } as_br;
        struct
        {
            xjs_ir_reg cond;
            xjs_ir_offset dest;
        } as_br_cond;
        struct
        {
            xjs_ir_reg test, consequent, alternate, dst;
        } as_merge;
        struct
        {
            xjs_ir_reg reg;
        } as_load_print;
        struct
        {
            xjs_ir_reg reg;
            union
            {
                ec_u8 as_u8;
                ec_s8 as_s8;
                ec_u16 as_u16;
                ec_s16 as_s16;
                ec_u32 as_u32;
                ec_s32 as_s32;
                float as_f32;
                double as_f64;
            } u;
        } as_load_literal;
        struct
        {
            xjs_ir_reg reg;
            xjs_ir_offset offset_data;
        } as_load_string;
        struct
        {
            xjs_ir_reg reg;
        } as_load_object;
        struct
        {
            xjs_ir_reg reg;
        } as_load_array;
        struct
        {
            xjs_ir_reg reg;
            xjs_ir_offset offset_text;
        } as_load_function;
        struct
        {
            xjs_ir_reg lhs, rhs, dst;
        } as_binary_op;
        struct
        {
            xjs_ir_reg src, dst;
        } as_unary_op;
    } operand;

    ec_string *filename;
    struct
    {
        struct
        {
            int ln, col;
        } start, end;
    } loc;
    struct
    {
        int start, end;
    } range;
} annotated_text_section_item;
typedef annotated_text_section_item *annotated_text_section_item_ref;

annotated_text_section_item_ref annotated_text_section_item_new(void);

ect_list_declare(annotated_text_section_item_list, annotated_text_section_item_ref);
typedef annotated_text_section_item_list *annotated_text_section_item_list_ref;

/* Map (ir_functionid -> offset) */

ect_map_declare(xjs_ir_functionid_offset_map, xjs_ir_functionid, ec_size_t);
typedef xjs_ir_functionid_offset_map *xjs_ir_functionid_offset_map_ref;

/* Map (ir_function -> offset) */

ect_map_declare(xjs_ir_function_offset_map, xjs_ir_function_ref, ec_size_t);
typedef xjs_ir_function_offset_map *xjs_ir_function_offset_map_ref;

/* Map (labelid -> offset) */

ect_map_declare(xjs_ir_label_offset_map, xjs_ir_label, xjs_ir_offset);
typedef xjs_ir_label_offset_map *xjs_ir_label_offset_map_ref;

/* Map (labelid -> point) */

ect_map_declare(xjs_ir_label_point_map, xjs_ir_label, ec_size_t);
typedef xjs_ir_label_point_map *xjs_ir_label_point_map_ref;

/* Map (dataid -> offset) */
ect_map_declare(xjs_ir_dataid_offset_map, xjs_ir_dataid, xjs_ir_offset);
typedef xjs_ir_dataid_offset_map *xjs_ir_dataid_offset_map_ref;


#endif

