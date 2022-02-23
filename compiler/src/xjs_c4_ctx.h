/* XenonJS : C4 : Context
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_C4_CTX_H
#define XJS_C4_CTX_H

#include "xjs_types.h"
#include "xjs_bitwriter.h"
#include "xjs_c4_regalloc.h"
#include "xjs_c4_helper.h"

typedef struct opaque_annotated_function annotated_function;
typedef annotated_function *annotated_function_ref;

ect_list_declare(annotated_function_list, annotated_function_ref);
typedef annotated_function_list *annotated_function_list_ref;

typedef struct
{
    xjs_error_ref err;
    xjs_bitwriter *bw_header;
    xjs_bitwriter *bw_index;
    xjs_bitwriter *bw_attr;
    xjs_bitwriter *bw_data;
    xjs_bitwriter *bw_text;
    xjs_bitwriter *bw_export;
    xjs_bitwriter *bw_import;
    xjs_bitwriter *bw_dbg0;
    xjs_bitwriter *bw_dbi0;
    xjs_ir_ref ir;

    /* Annotate */
    annotated_function_list_ref annotated_functions;

    /* text */
    xjs_ir_function_offset_map_ref map_ir_function_to_offset;
    xjs_ir_functionid_offset_map_ref map_ir_functionid_to_offset;

    /* data */
    xjs_ir_dataid_offset_map_ref map_ir_dataid_to_offset;

    ec_size_t offset_toplevel;

    xjs_bool generate_debug_info;
} xjs_c4_ctx;

struct opaque_annotated_function
{
    annotated_text_section_item_list_ref items;

    /* The offset to the head of text section */
    ec_size_t offset;

    xjs_ir_label_offset_map_ref map_ir_label_to_offset;
    xjs_ir_label_point_map_ref map_ir_label_to_point;
};

annotated_function_ref annotated_function_new(void);

typedef struct
{
    /* Global context */
    xjs_c4_ctx *global_ctx;

    /* Function context */
    xjs_regsched_ref regsched;
    /* number of processed text_item */
    ec_size_t point;
    /* Annotated function */
    annotated_function_ref annotated_function;

    xjs_size_t offset;
} xjs_c4_func_ctx;

#endif

