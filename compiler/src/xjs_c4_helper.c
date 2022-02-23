/* XenonJS : C4 : Helper
 * Copyright(c) 2017 y2c2 */

#include <ec_list.h>
#include <ec_map.h>
#include "xjs_xformat.h"
#include "xjs_c4_helper.h"

static void annotated_text_section_item_ctor(void *text)
{
    annotated_text_section_item_ref item = text;
    item->filename = NULL;
    item->offset = 0;
}

static void annotated_text_section_item_dtor(void *text)
{
    annotated_text_section_item_ref item = text;
    ec_delete(item->filename);
}

annotated_text_section_item_ref
annotated_text_section_item_new(void)
{
    annotated_text_section_item_ref new_item; 

    if ((new_item = ec_newcd(annotated_text_section_item, \
                    annotated_text_section_item_ctor, \
                    annotated_text_section_item_dtor)) == NULL)
    { return NULL; }

    new_item->opcode = XJS_BC_OP_NOP;

    new_item->filename = NULL;
    new_item->loc.start.ln = -1;
    new_item->loc.start.col = -1;
    new_item->loc.end.ln = -1;
    new_item->loc.end.col = -1;
    new_item->range.start = -1;
    new_item->range.end = -1;

    return new_item;
}

static void annotated_text_section_item_list_node_dtor(annotated_text_section_item *item)
{
    ec_delete(item);
}

ect_list_define_declared(annotated_text_section_item_list, \
        annotated_text_section_item_ref, annotated_text_section_item_list_node_dtor);

static int xjs_ir_functionid_offset_map_key_ctor(xjs_ir_functionid *detta_key, xjs_ir_functionid *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_ir_functionid_offset_map_key_cmp(xjs_ir_functionid *a, xjs_ir_functionid *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

static int xjs_ir_functionid_offset_map_value_ctor(ec_size_t *detta_value, ec_size_t *value)
{
    *detta_value = *value;
    return 0;
}

ect_map_define_declared(xjs_ir_functionid_offset_map, \
        xjs_ir_functionid, xjs_ir_functionid_offset_map_key_ctor, NULL, \
        ec_size_t, xjs_ir_functionid_offset_map_value_ctor, NULL, \
        xjs_ir_functionid_offset_map_key_cmp);

static int xjs_ir_function_offset_map_key_ctor(xjs_ir_function_ref *detta_key, xjs_ir_function_ref *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_ir_function_offset_map_key_cmp(xjs_ir_function_ref *a, xjs_ir_function_ref *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

static int xjs_ir_function_offset_map_value_ctor(ec_size_t *detta_value, ec_size_t *value)
{
    *detta_value = *value;
    return 0;
}

ect_map_define_declared(xjs_ir_function_offset_map, \
        xjs_ir_function_ref, xjs_ir_function_offset_map_key_ctor, NULL, \
        ec_size_t, xjs_ir_function_offset_map_value_ctor, NULL, \
        xjs_ir_function_offset_map_key_cmp);

static int xjs_ir_label_offset_map_key_ctor(xjs_ir_label *detta_key, xjs_ir_label *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_ir_label_offset_map_key_cmp(xjs_ir_label *a, xjs_ir_label *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

static int xjs_ir_label_offset_map_value_ctor(xjs_ir_offset *detta_value, xjs_ir_offset *value)
{
    *detta_value = *value;
    return 0;
}

ect_map_define_declared(xjs_ir_label_offset_map, \
        xjs_ir_label, xjs_ir_label_offset_map_key_ctor, NULL, \
        xjs_ir_offset, xjs_ir_label_offset_map_value_ctor, NULL, \
        xjs_ir_label_offset_map_key_cmp);

static int xjs_ir_dataid_offset_map_key_ctor(xjs_ir_dataid *detta_key, xjs_ir_dataid *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_ir_dataid_offset_map_key_cmp(xjs_ir_dataid *a, xjs_ir_dataid *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

static int xjs_ir_dataid_offset_map_value_ctor(xjs_ir_offset *detta_value, xjs_ir_offset *value)
{
    *detta_value = *value;
    return 0;
}

ect_map_define_declared(xjs_ir_dataid_offset_map, \
        xjs_ir_dataid, xjs_ir_dataid_offset_map_key_ctor, NULL, \
        xjs_ir_offset, xjs_ir_dataid_offset_map_value_ctor, NULL, \
        xjs_ir_dataid_offset_map_key_cmp);

static int xjs_ir_label_point_map_key_ctor(xjs_ir_label *detta_key, xjs_ir_label *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_ir_label_point_map_key_cmp(xjs_ir_label *a, xjs_ir_label *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

static int xjs_ir_label_point_map_value_ctor(ec_size_t *detta_value, ec_size_t *value)
{
    *detta_value = *value;
    return 0;
}

ect_map_define_declared(xjs_ir_label_point_map, \
        xjs_ir_label, xjs_ir_label_point_map_key_ctor, NULL, \
        ec_size_t, xjs_ir_label_point_map_value_ctor, NULL, \
        xjs_ir_label_point_map_key_cmp);

