/* XenonJS : C4 : Data Section
 * Copyright(c) 2017 y2c2 */

#include <ec_encoding_utf8.h>
#include <ec_encoding.h>
#include <ec_algorithm.h>
#include <ec_list.h>
#include "xjs_helper.h"
#include "xjs_xformat.h"
#include "xjs_c4_datasec.h"

typedef enum
{
    ANNOTATED_DATA_SECTION_ITEM_TYPE_STR,
    ANNOTATED_DATA_SECTION_ITEM_TYPE_SYM,
    ANNOTATED_DATA_SECTION_ITEM_TYPE_F32,
    ANNOTATED_DATA_SECTION_ITEM_TYPE_F64,
} annotated_data_section_item_type;

typedef struct
{
    ec_u32 dataid;
    annotated_data_section_item_type type;
    ec_size_t offset;
    ec_byte_t *data;
    ec_size_t length;
} annotated_data_section_item;
typedef annotated_data_section_item *annotated_data_section_item_ref;

static void annotated_data_section_item_dtor(void *data)
{
    annotated_data_section_item_ref item = data;
    if (item->data != NULL) ec_free(item->data);
}

static
annotated_data_section_item_ref
annotated_data_section_item_new( \
        ec_u32 dataid, \
        annotated_data_section_item_type type, \
        ec_size_t offset, ec_byte_t *data, ec_size_t length)
{
    ec_byte_t *new_data = NULL;
    annotated_data_section_item_ref new_item; 

    if ((new_data = (ec_byte_t *)ec_malloc( \
                    sizeof(ec_byte_t) * (length + 1))) == NULL)
    { return NULL; }
    ec_memcpy(new_data, data, length);
    new_data[length] = 0x0;

    if ((new_item = ec_newd(annotated_data_section_item, \
                    annotated_data_section_item_dtor)) == NULL)
    { ec_free(new_data); return NULL; }

    new_item->dataid = dataid;
    new_item->type = type;
    new_item->offset = offset;
    new_item->length = length;
    new_item->data = new_data;

    return new_item;
}

static void annotated_data_section_item_list_node_dtor(annotated_data_section_item *item)
{
    ec_delete(item);
}

ect_list_define_undeclared(annotated_data_section_item_list, \
        annotated_data_section_item_ref, annotated_data_section_item_list_node_dtor);
typedef annotated_data_section_item_list *annotated_data_section_item_list_ref;

static annotated_data_section_item_list_ref 
xjs_c4_data_section_annotate(xjs_c4_ctx *ctx)
{
    xjs_ir_dataid_offset_map_ref map_ir_dataid_to_offset = NULL;
    annotated_data_section_item_list_ref annotated_items = NULL;
    ec_size_t offset = 0;

    XJS_VNZ_ERROR_MEM(annotated_items = ect_list_new(annotated_data_section_item_list), ctx->err);

    if ((map_ir_dataid_to_offset = xjs_ir_dataid_offset_map_new()) == NULL) { goto fail; }
    ctx->map_ir_dataid_to_offset = map_ir_dataid_to_offset;

    /* 1st pass, items */
    ect_iterator(xjs_ir_data_item_list) it;
    ect_for(xjs_ir_data_item_list, ctx->ir->data->items, it)
    {
        xjs_ir_data_item_ref item = ect_deref(xjs_ir_data_item_ref, it);
        {
            ec_string *s;
            annotated_data_section_item_type atype;

            if (item->type == xjs_ir_data_item_type_string)
            {
                s = item->u.as_string.value;
                atype = ANNOTATED_DATA_SECTION_ITEM_TYPE_STR;
            }
            else if (item->type == xjs_ir_data_item_type_symbol)
            {
                s = item->u.as_symbol.value;
                atype = ANNOTATED_DATA_SECTION_ITEM_TYPE_SYM;
            }
            else
            {
                XJS_ERROR_INTERNAL(ctx->err);
            }

            /* Encode as UTF-8 String */
            ec_encoding_t enc;
            ec_byte_t *encoded_bytes = NULL;
            ec_size_t encoded_bytes_len;

            ec_encoding_utf8_init(&enc);
            XJS_VEZ_ERROR_INTERNAL( \
                    ec_encoding_encode( \
                        &enc, \
                        &encoded_bytes, &encoded_bytes_len, \
                        s), ctx->err);
            {
                annotated_data_section_item_ref new_item;
                XJS_VNZ_ERROR_MEM_OR( \
                        new_item = annotated_data_section_item_new( \
                            (ec_u32)item->dataid, \
                            atype, \
                            offset, \
                            encoded_bytes, \
                            encoded_bytes_len), ctx->err, ec_free(encoded_bytes));
                ect_list_push_back( \
                        annotated_data_section_item_list, \
                        annotated_items, \
                        new_item);
                ec_free(encoded_bytes);
            }
            xjs_ir_dataid_offset_map_insert(map_ir_dataid_to_offset, \
                    item->dataid, (xjs_ir_offset)offset);
            /* Type:u16 + Length:u32 + Body:[u8] + NUL:8 */
            offset += 2 + 4 + encoded_bytes_len + 1;
        }
    }

    goto done;
fail:
    if (annotated_items != NULL)
    {
        ec_delete(annotated_items);
        annotated_items = NULL;
    }
done:
    return annotated_items;
}

int xjs_c4_write_data(xjs_c4_ctx *ctx)
{
    int ret = 0;
    annotated_data_section_item_list_ref annotated_items = NULL;

    if ((annotated_items = xjs_c4_data_section_annotate(ctx)) == NULL)
    { goto fail; }

    /* Section Signature */
    xjs_bitwriter_append(ctx->bw_data, \
            XJS_BC_SECTION_SIGNATURE_DATA, XJS_BC_SECTION_SIGNATURE_LEN);

    /* Items */
    {
        ect_iterator(annotated_data_section_item_list) it;
        ect_for(annotated_data_section_item_list, annotated_items, it)
        {
            annotated_data_section_item_ref item = ect_deref(annotated_data_section_item_ref, it);

            /* Type */
            if (item->type == ANNOTATED_DATA_SECTION_ITEM_TYPE_STR)
            {
                xjs_bitwriter_write_u16(ctx->bw_data, (xjs_u16)XJS_BC_DATASEC_ITEM_TYPE_UTF8STR);
            }
            else if (item->type == ANNOTATED_DATA_SECTION_ITEM_TYPE_SYM)
            {
                xjs_bitwriter_write_u16(ctx->bw_data, (xjs_u16)XJS_BC_DATASEC_ITEM_TYPE_UTF8SYM);
            }
            else
            {
                XJS_ERROR_INTERNAL(ctx->err);
            }
            /* Length */
            xjs_bitwriter_write_u32(ctx->bw_data, (xjs_u32)item->length);
            /* Body */
            xjs_bitwriter_append(ctx->bw_data, (char *)item->data, item->length);
            /* NUL */
            xjs_bitwriter_write_u8(ctx->bw_data, 0);
        }
    }

    goto done;
fail:
    ret = -1;
done:
    ec_delete(annotated_items);
    return ret;
}

