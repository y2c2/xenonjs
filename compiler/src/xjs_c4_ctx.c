/* XenonJS : C4 : Context
 * Copyright(c) 2017 y2c2 */

#include "xjs_c4_ctx.h"

static void annotated_function_ctor(void *data)
{
    annotated_function_ref f = data;
    f->items = annotated_text_section_item_list_new();
    f->map_ir_label_to_offset = NULL;
    f->map_ir_label_to_point = NULL;
}

static void annotated_function_dtor(void *data)
{
    annotated_function_ref f = data;
    ec_delete(f->items);
    ec_delete(f->map_ir_label_to_offset);
    ec_delete(f->map_ir_label_to_point);
}

annotated_function_ref annotated_function_new(void)
{
    annotated_function_ref r = ec_newcd( \
            annotated_function, \
            annotated_function_ctor, annotated_function_dtor);
    return r;
}

static void annotated_function_list_node_dtor(annotated_function_ref item)
{
    ec_delete(item);
}

ect_list_define_declared(annotated_function_list, \
        annotated_function_ref, annotated_function_list_node_dtor);

